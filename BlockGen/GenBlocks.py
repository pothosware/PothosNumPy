#!/usr/bin/env python

import datetime
from mako.template import Template
import os
import sys
import yaml

ScriptDirName = os.path.dirname(__file__)
BlocksDir = os.path.join(ScriptDirName, "Blocks")
OutputDir = os.path.abspath(sys.argv[1])
Now = datetime.datetime.now()

CppFactoryTemplate = None
PythonFactoryFunctionTemplate = None
BlockExecutionTestTemplate = None

def populateTemplates():
    global CppFactoryTemplate
    global PythonFactoryFunctionTemplate
    global BlockExecutionTestTemplate

    cppFactoryFunctionTemplatePath = os.path.join(ScriptDirName, "Factory.mako.cpp")
    with open(cppFactoryFunctionTemplatePath) as f:
        CppFactoryTemplate = f.read()

    pythonFactoryFunctionTemplatePath = os.path.join(ScriptDirName, "PythonFactoryFunction.mako.py")
    with open(pythonFactoryFunctionTemplatePath) as f:
        PythonFactoryFunctionTemplate = f.read()

    blockExecutionTestTemplatePath = os.path.join(ScriptDirName, "BlockExecutionTest.mako.cpp")
    with open(blockExecutionTestTemplatePath) as f:
        BlockExecutionTestTemplate = f.read()

def processYAMLFile(yamlPath):
    yml = None
    with open(yamlPath) as f:
        yml = yaml.load(f.read())

    if not yml:
        raise RuntimeError("No YAML found in {0}".format(filepath))

    # These entries are fully defined, so when other entries have a "copy"
    # field, the script will look in this dict.
    templateEntries = {k:v for k,v in yml.items() if "copy" not in v}

    fullEntries = templateEntries.copy()
    for k,v in yml.items():
        if "copy" in v:
            if v["copy"] not in templateEntries:
                raise RuntimeError('Could not find template entry: "{0}".'.format(v["copy"]))

            fullEntry = templateEntries[v["copy"]].copy()
            for keyToErase in ["alias", "niceName"]:
                if keyToErase in fullEntry:
                    del fullEntry[keyToErase]
            fullEntry.update(v)
        else:
            fullEntry = v.copy()

        fullEntries[k] = fullEntry

    return fullEntries

def generateCppFactory(func,name):
    return 'Pothos::BlockRegistry("/numpy/{0}", Pothos::Callable(&FactoryFunc).bind<std::string>("{1}", 2))' \
           .format(func, name)

def formatTypeText(typeText):
    return typeText.title().replace("Uint", "UInt")

def blockTypeToDictString(blockTypeYAML):
    return "dict({0})".format(", ".join(["support{0}=True".format(formatTypeText(typeText)) for typeText in blockTypeYAML]))

def processBlock(yaml, makoVars):
    for key in ["blockType", "inputType"]:
        if key in yaml:
            makoVars["inputDTypeArgs"] = blockTypeToDictString(yaml[key])
            break
    for key in ["blockType", "outputType"]:
        if key in yaml:
            makoVars["outputDTypeArgs"] = blockTypeToDictString(yaml[key])
            break

    if yaml["class"] in ["NToOneBlock"]:
        makoVars["factoryParams"] = "nchans" if len(makoVars["factoryParams"]) == 0 else ("nchans, " + makoVars["factoryParams"])
        makoVars["classParams"] = "nchans" if len(makoVars["classParams"]) == 0 else ("nchans, " + makoVars["classParams"])

def processSource(yaml, makoVars):
    for key in ["blockType", "outputType"]:
        if key in yaml:
            makoVars["outputDTypeArgs"] = blockTypeToDictString(yaml[key])
            break

def generatePythonFactoryFunction(func,yaml):
    # Generate variables for processing.
    makoVars = dict()
    makoVars["name"] = yaml["name"]
    makoVars["category"] = " ".join(yaml["categories"])
    makoVars["func"] = func
    makoVars["keywords"] = func
    makoVars["class"] = yaml["class"]
    makoVars["prefix"] = yaml.get("prefix", "numpy")

    # Some keys are just straight copies.
    for key in ["alias", "niceName"]:
        if key in yaml:
            makoVars[key] = yaml[key]

    makoVars["classParams"] = ""
    makoVars["factoryParams"] = ""

    for key in ["args", "funcArgs"]:
        if key in yaml:
            makoVars[key] = "[{0}]".format(", ".join(yaml[key]))
    if "args" in yaml:
        if key in yaml:
            makoVars["classParams"] += ", *args"

    for key in ["kwargs", "funcKWargs"]:
        if key in yaml:
            makoVars[key] = "dict({0})".format(", ".join(yaml[key]))
    if "kwargs" in yaml:
        if key in yaml:
            makoVars["classParams"] += ", **kwargs"

    if "blockType" in yaml:
        if "Block" in yaml["class"]:
            processBlock(yaml, makoVars)
        elif "Source" in yaml["class"]:
            processSource(yaml, makoVars)
        else:
            raise RuntimeError("Invalid block type.")
    elif "blockPattern" in yaml:
        if yaml["blockPattern"] == "ComplexToScalar":
            makoVars["inputDTypeArgs"] = blockTypeToDictString(["complex"])
            makoVars["inputDTypeArgs"] = blockTypeToDictString(["float"])
        else:
            raise RuntimeError("Invalid block pattern.")

    return Template(PythonFactoryFunctionTemplate).render(makoVars=makoVars)

def generatePythonFactoryFunctionOld(func,yaml):
    # Generate variables for processing.
    makoVars = dict()
    makoVars["name"] = yaml["name"]
    makoVars["category"] = " ".join(yaml["categories"])
    makoVars["func"] = func
    makoVars["keywords"] = func
    makoVars["class"] = yaml["class"]
    makoVars["prefix"] = yaml.get("prefix", "numpy")

    if "alias" in yaml:
        makoVars["alias"] = yaml["alias"]
    if "niceName" in yaml:
        makoVars["niceName"] = yaml["niceName"]

    def formatTypeText(typeText):
        return typeText.title().replace("Uint", "UInt")

    if "blockType" in yaml:
        if "Block" in yaml["class"]:
            dictText = "dict({0})".format(", ".join(["support{0}=True".format(formatTypeText(typeText)) for typeText in yaml["blockType"]]))
            makoVars["factoryParams"] = "dtype"
            if makoVars["class"] == "ForwardAndPostLabelBlock":
                makoVars["classParams"] = "{0}, \"{1}\", dtype, dtype, {2}, {2}".format(yaml.get("findIndexFunc", "None"), yaml["label"], dictText)
            else:
                makoVars["classParams"] = "dtype, dtype, {0}, {0}".format(dictText)
            if yaml["class"] == "NToOneBlock":
                makoVars["factoryParams"] += ", nchans"
                makoVars["classParams"] += ", nchans"
        elif "Source" in yaml["class"]:
            dictText = "dict({0})".format(", ".join(["support{0}=True".format(formatTypeText(typeText)) for typeText in yaml["blockType"]]))
            makoVars["factoryParams"] = "dtype"
            makoVars["classParams"] = "dtype, {0}".format(dictText)
        else:
            raise RuntimeError("Invalid block type.")
    elif "blockPattern" in yaml:
        if yaml["blockPattern"] == "ComplexToScalar":
            makoVars["factoryParams"] = "inputDType, outputDType"
            makoVars["classParams"] = "inputDType, outputDType, dict(supportComplex=True), dict(supportFloat=True)"
        else:
            raise RuntimeError("Invalid block pattern.")

    if "args" in yaml:
        makoVars["args"] = "[{0}]".format(", ".join(yaml["args"]))
        makoVars["classParams"] += ", *args"

    if "kwargs" in yaml:
        makoVars["kwargs"] = "dict({0})".format(", ".join(yaml["kwargs"]))
        makoVars["classParams"] += ", **kwargs"

    return Template(PythonFactoryFunctionTemplate).render(makoVars=makoVars)

def generateCppOutput(expandedYAML):
    prefix = """// Copyright (c) 2019-{0} Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

//
// This file was auto-generated on {1}.
//
""".format(Now.year, Now)

    factories = []
    for k,v in expandedYAML.items():
        factories += [generateCppFactory(k,v["name"])]
        if "alias" in v:
            for alias in v["alias"]:
                factories += [generateCppFactory(alias,v["name"])]

    # Add C++-only blocks.
    cppOnlyYAMLPath = os.path.join(BlocksDir, "CppOnly.yaml")
    with open(cppOnlyYAMLPath) as f:
        cppOnlyYAML = yaml.load(f.read())

    for k,v in cppOnlyYAML.items():
        factories += [generateCppFactory(k,v["name"])]
    if "alias" in v:
        for alias in v["alias"]:
            factories += [generateCppFactory(alias,v["name"])]

    rendered = Template(CppFactoryTemplate).render(factories=factories)

    output = "{0}\n{1}".format(prefix, rendered)

    outputFilepath = os.path.join(OutputDir, "Factory.cpp")
    with open(outputFilepath, 'w') as f:
        f.write(output)

def generatePythonOutput(expandedYAML):
    prefix = """# Copyright (c) 2019-{0} Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

#
# This file was auto-generated on {1}.
#

from . import Utility

from .OneToOneBlock import *
from .TwoToOneBlock import *
from .NToOneBlock import *
from .ForwardAndPostLabelBlock import *
from .Source import *
""".format(Now.year, Now)

    pythonFactoryFunctionsList = [generatePythonFactoryFunction(k,v) for k,v in expandedYAML.items() if not v.get("cppOnly", False)]
    pythonFactoryFunctionsStr = "\n".join(pythonFactoryFunctionsList)

    output = "{0}\n{1}".format(prefix, pythonFactoryFunctionsStr)

    outputFilepath = os.path.join(OutputDir, "CppEntryPoints.py")
    with open(outputFilepath, 'w') as f:
        f.write(output)

def generateBlockExecutionTest(expandedYAML):
    sfinaeMap = dict(
        Integer="int",
        UnsignedInt="uint",
        Float="float",
        Complex="complex"
    )

    output = Template(BlockExecutionTestTemplate).render(blockYAML=expandedYAML, Now=Now, sfinaeMap=sfinaeMap)

    outputFilepath = os.path.join(OutputDir, "BlockExecutionTest.cpp")
    with open(outputFilepath, 'w') as f:
        f.write(output)

if __name__ == "__main__":
    yamlFiles = [os.path.join(BlocksDir, filepath) for filepath in os.listdir(BlocksDir) if os.path.splitext(filepath)[1] == ".yaml"]

    expandedYAMLList = [processYAMLFile(yamlFile) for yamlFile in yamlFiles if "CppOnly.yaml" not in yamlFile]

    # At this point, we have a list of dictionaries, so consolidate them.
    expandedYAML = dict()
    for yml in expandedYAMLList:
        expandedYAML.update(yml)

    populateTemplates()
    generateCppOutput(expandedYAML)
    generatePythonOutput(expandedYAML)
    generateBlockExecutionTest(expandedYAML)
