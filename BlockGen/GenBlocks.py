#!/usr/bin/env python

import datetime
from mako.template import Template
import mako.exceptions
import os
import sys
import yaml

ScriptDirName = os.path.dirname(__file__)
BlocksDir = os.path.join(ScriptDirName, "Blocks")
OutputDir = os.path.abspath(sys.argv[1])
Now = datetime.datetime.now()

CppFactoryTemplate = None
PythonFactoryFunctionTemplate = None
PythonSubclassTemplate = None
BlockExecutionTestTemplate = None

def populateTemplates():
    global CppFactoryTemplate
    global PythonFactoryFunctionTemplate
    global PythonSubclassTemplate
    global BlockExecutionTestTemplate

    cppFactoryFunctionTemplatePath = os.path.join(ScriptDirName, "Factory.mako.cpp")
    with open(cppFactoryFunctionTemplatePath) as f:
        CppFactoryTemplate = f.read()

    pythonFactoryFunctionTemplatePath = os.path.join(ScriptDirName, "PythonFactoryFunction.mako.py")
    with open(pythonFactoryFunctionTemplatePath) as f:
        PythonFactoryFunctionTemplate = f.read()

    pythonSubclassTemplatePath = os.path.join(ScriptDirName, "PythonSubclass.mako.py")
    with open(pythonSubclassTemplatePath) as f:
        PythonSubclassTemplate = f.read()

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
    if makoVars["class"] in ["NToOneBlock"]:
        makoVars["factoryParams"] = ["nchans"] + makoVars["factoryParams"]
        makoVars["classParams"] = ["nchans"] + makoVars["classParams"]
    if makoVars["class"] in ["ForwardAndPostLabelBlock"]:
        label = "\"{0}\"".format(yaml["label"]) if "label" in yaml else "None"
        makoVars["classParams"] = [label] + makoVars["classParams"]
        makoVars["classParams"] = [yaml.get("findIndexFunc", "None")] + makoVars["classParams"]

    for key in ["blockType", "outputType"]:
        if key in yaml:
            makoVars["outputDTypeArgs"] = blockTypeToDictString(yaml[key])
            makoVars["classParams"] = ["outputDTypeArgs"] + makoVars["classParams"]
            makoVars["factoryVars"] += ["outputDTypeArgs"]
            break
    for key in ["blockType", "inputType"]:
        if key in yaml:
            makoVars["inputDTypeArgs"] = blockTypeToDictString(yaml[key])
            makoVars["classParams"] = ["inputDTypeArgs"] + makoVars["classParams"]
            makoVars["factoryVars"] += ["inputDTypeArgs"]
            break

    if "blockType" in yaml:
        makoVars["classParams"] = ["dtype, dtype"] + makoVars["classParams"]
        makoVars["factoryParams"] = ["dtype"] + makoVars["factoryParams"]
    else:
        makoVars["classParams"] = ["inputDType", "outputDType"] + makoVars["classParams"]
        makoVars["factoryParams"] = ["inputDType", "outputDType"] + makoVars["factoryParams"]

def processSource(yaml, makoVars):
    for key in ["blockType", "outputType"]:
        if key in yaml:
            makoVars["outputDTypeArgs"] = blockTypeToDictString(yaml[key])
            makoVars["classParams"] = ["outputDTypeArgs"] + makoVars["classParams"]
            makoVars["factoryVars"] += ["outputDTypeArgs"]
            break

    makoVars["classParams"] = ["dtype"] + makoVars["classParams"]
    makoVars["factoryParams"] = ["dtype"] + makoVars["factoryParams"]

def generatePythonEntryPoint(func,yaml):
    # Generate variables for processing.
    makoVars = dict()
    makoVars["name"] = yaml["name"]
    makoVars["class"] = yaml["class"]
    makoVars["category"] = " ".join(yaml["categories"])
    makoVars["func"] = func
    makoVars["keywords"] = func
    makoVars["prefix"] = yaml.get("prefix", "numpy")
    makoVars["factoryVars"] = []

    isEntryPointSubclass = yaml.get("subclass", False)

    if "funcArgs" in yaml:
        for arg in yaml["funcArgs"]:
            arg["title"] = arg["name"][0].upper() + arg["name"][1:]
            arg["privateVar"] = "__{0}".format(arg["name"])
        makoVars["funcArgsList"] = ["self.{0}".format(arg["privateVar"]) for arg in yaml["funcArgs"]]

    # Some keys are just straight copies.
    for key in ["alias", "niceName", "funcArgs"]:
        if key in yaml:
            makoVars[key] = yaml[key]

    makoVars["classParams"] = []
    makoVars["factoryParams"] = []

    if "args" in yaml:
        makoVars["factoryVars"] += ["args"]
        makoVars["args"] = "[{0}]".format(", ".join(yaml["args"]))

    if "kwargs" in yaml:
        makoVars["factoryVars"] += ["kwargs"]
        makoVars["kwargs"] = "dict({0})".format(", ".join(yaml["kwargs"]))

    if "funcArgs" in yaml:
        assert(type(yaml["funcArgs"]) is list)

        for arg in yaml["funcArgs"]:
            if arg.get("isPublic", True):
                makoVars["factoryParams"] += [arg["name"]]

    if "funcKWargs" in yaml:
        assert(type(yaml["funcKWargs"]) is list)
        funcKWargs = []

        for arg in yaml["funcKWargs"]:
            if arg.get("isPublic", True):
                makoVars["factoryParams"] = [arg["name"]] + makoVars["factoryParams"]
                funcKWargs += ["{0}={0}".format(arg["name"])]
            else:
                funcKWargs += ["{0}={1}".format(arg["name"], arg.get("value", "None"))]

        makoVars["factoryVars"] += ["funcKWargs"]
        makoVars["funcKWargs"] = "dict({0})".format(", ".join(funcKWargs))

    if "blockType" in yaml:
        if "Block" in makoVars["class"]:
            processBlock(yaml, makoVars)
        elif "Source" in makoVars["class"]:
            processSource(yaml, makoVars)
        else:
            raise RuntimeError("Invalid block type.")
    elif "blockPattern" in yaml:
        if yaml["blockPattern"] == "ComplexToScalar":
            yaml["inputType"] = ["complex"]
            yaml["outputType"] = ["float"]
            processBlock(yaml, makoVars)
        else:
            raise RuntimeError("Invalid block pattern.")

    makoVars["classParams"] = ["{0}.{1}".format(makoVars["prefix"], func)] + makoVars["classParams"]

    makoVars["classParams"] += ["list()"] # TODO: don't pass funcArgs as param
    makoVars["classParams"] += ["funcKWargs" if "funcKWargs" in makoVars else "dict()"]
    if "args" in makoVars:
        makoVars["classParams"] += ["*args"]
    if "kwargs" in makoVars:
        makoVars["classParams"] += ["**kwargs"]

    try:
        if isEntryPointSubclass:
            return Template(PythonSubclassTemplate).render(makoVars=makoVars)
        else:
            return Template(PythonFactoryFunctionTemplate).render(makoVars=makoVars)
    except:
        print(mako.exceptions.text_error_template().render())

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

    try:
        rendered = Template(CppFactoryTemplate).render(factories=factories)
    except:
        print(mako.exceptions.text_error_template().render())

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

    try:
        pythonEntryPointsList = [generatePythonEntryPoint(k,v) for k,v in expandedYAML.items() if not v.get("cppOnly", False)]
    except:
        print(mako.exceptions.text_error_template().render())

    pythonEntryPointsStr = "\n".join(pythonEntryPointsList)

    output = "{0}\n{1}".format(prefix, pythonEntryPointsStr)

    outputFilepath = os.path.join(OutputDir, "BlockEntryPoints.py")
    with open(outputFilepath, 'w') as f:
        f.write(output)

def generateBlockExecutionTest(expandedYAML):
    sfinaeMap = dict(
        Integer="int",
        UnsignedInt="uint",
        Float="float",
        Complex="complex"
    )

    maxNumParams = max([len(expandedYAML[block].get("funcArgs", [])) for block in expandedYAML])
    assert(maxNumParams > 0)

    try:
        output = Template(BlockExecutionTestTemplate).render(blockYAML=expandedYAML, Now=Now, sfinaeMap=sfinaeMap, maxNumParams=maxNumParams)
    except:
        print(mako.exceptions.text_error_template().render())

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
