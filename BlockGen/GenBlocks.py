#!/usr/bin/env python

import datetime
from mako.template import Template
import os
import sys
import yaml

Now = datetime.datetime.now()

PythonFactoryFunctionTemplate = None

def populateTemplates():
    global PythonFactoryFunctionTemplate

    pythonFactoryFunctionTemplatePath = os.path.join(os.path.dirname(__file__), "PythonFactoryFunction.mako.py")
    with open(pythonFactoryFunctionTemplatePath) as f:
        PythonFactoryFunctionTemplate = f.read()

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
            if "alias" in fullEntry:
                del fullEntry["alias"]
            fullEntry.update(v)
        else:
            fullEntry = v.copy()

        fullEntries[k] = fullEntry

    return fullEntries

def generatePythonFactoryFunction(func,yaml):
    # Generate variables for processing.
    makoVars = dict()
    makoVars["name"] = yaml["name"]
    makoVars["category"] = " ".join(yaml["categories"])
    makoVars["func"] = func
    makoVars["keywords"] = func
    makoVars["class"] = yaml["class"]
    makoVars["prefix"] = yaml["prefix"]

    if "alias" in yaml:
        makoVars["alias"] = yaml["alias"]

    def formatTypeText(typeText):
        return typeText.title().replace("Uint", "UInt")

    if ("blockType" in yaml) and ("Block" in yaml["class"]):
        dictText = "dict({0})".format(", ".join(["support{0}=True".format(formatTypeText(typeText)) for typeText in yaml["blockType"]]))
        makoVars["factoryParams"] = "dtype"
        makoVars["classParams"] = "dtype, dtype, {1}, {1}".format(dictText, dictText)
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

    print(Template(PythonFactoryFunctionTemplate).render(makoVars=makoVars))

if __name__ == "__main__":
    blocksDir = os.path.join(os.path.dirname(__file__), "Blocks")
    yamlFiles = [os.path.join(blocksDir, filepath) for filepath in os.listdir(blocksDir) if os.path.splitext(filepath)[1] == ".yaml"]

    expandedYAMLList = [processYAMLFile(yamlFile) for yamlFile in yamlFiles]

    # At this point, we have a list of dictionaries, so consolidate them.
    expandedYAML = dict()
    for yaml in expandedYAMLList:
        expandedYAML.update(yaml)

    populateTemplates()

    for k,v in expandedYAML.items():
        generatePythonFactoryFunction(k,v)

'''
NONSTANDARD = ["templates", "factoryOnly"]

def manualReplace(blockPath):
    REPLACES = dict(
        Fft="FFT",
        Ifft="IFFT",
        Rfft="RFFT",
        Irfft="IRFFT",
        Arange="ARange",
        Linspace="LinSpace",
        Logspace="LogSpace",
        Geomspace="GeomSpace",
        Amax="AMax",
        Amin="AMin",
        Nanmax="NaNMax",
        Nanmin="NaNMin",
        Ptp="PTP",
        Nanmedian="NaNMedian",
        Nanmean="NaNMean",
        Nanstd="NaNStd",
        Nanvar="NaNVar",
        CountNonzero="CountNonZero",
        Copysign="CopySign")

    return REPLACES.get(blockPath, blockPath)

def getEntryPointName(blockPath):
    return manualReplace(os.path.basename(blockPath).title().replace("_","").replace(".",""))

# TODO: account for sources and sinks, where one params are None
def generatePythonMakoParams(blockName, blockYAML):
    makoDict = dict()

    makoDict["func"] = blockName
    makoDict["cppEntryPoint"] = getEntryPointName(blockName)
    makoDict["blockClass"] = blockYAML["blockClass"]

    if "Source" in blockYAML["blockClass"]:
        makoDict["dtypeEntryPointParams"] = "dtype"
        makoDict["dtypeBlockParams"] = "dtype"
    else:
        makoDict["dtypeEntryPointParams"] = "dtype" if blockYAML["singleDType"] else "inputDType, outputDType"
        makoDict["dtypeBlockParams"] = "dtype, dtype" if blockYAML["singleDType"] else "inputDType, outputDType"

    if "inputArgs" in blockYAML:
        inputArgs = blockYAML["inputArgs"]
        makoDict["inputArgs"] = "dict({0})".format(", ".join(['{0}={1}'.format(x, inputArgs[x]) for x in inputArgs]))

    if "outputArgs" in blockYAML:
        outputArgs = blockYAML["outputArgs"]
        makoDict["outputArgs"] = "dict({0})".format(", ".join(['{0}={1}'.format(x, outputArgs[x]) for x in outputArgs]))

    makoDict["otherEntryPointParams"] = ", nchans" if (makoDict["blockClass"] == "NToOneBlock") else ""
    if "otherEntryPointParams" in blockYAML:
        makoDict["otherEntryPointParams"] += ", {0}".format(", ".join(blockYAML["otherEntryPointParams"]))

    otherBlockParams = "nchans, " if (makoDict["blockClass"] == "NToOneBlock") else ""
    otherBlockParams += "useDType={0}".format(blockYAML.get("useDType", False))
    if ("otherBlockParams" in blockYAML) and (len(blockYAML["otherBlockParams"]) > 0):
        otherBlockParams = "{0}, {1}".format(", ".join(blockYAML["otherBlockParams"]), otherBlockParams)
    makoDict["otherBlockParams"] = otherBlockParams

    return makoDict

def generateCppFactory(blockName):
    return 'Pothos::BlockRegistry("/numpy/{0}", Pothos::Callable(&FactoryFunc).bind<std::string>("{1}", 2))' \
           .format(blockName, getEntryPointName(blockName))

def generatePythonCppEntrypointFile(blockYAML, outputDir):
    blockMakoParams = [generatePythonMakoParams(blockName, blockYAML[blockName]) for blockName in blockYAML if (blockName not in NONSTANDARD)]
    for baseBlock in blockYAML["templates"]:
        blockMakoParams += [generatePythonMakoParams(blockName, blockYAML[baseBlock]) for blockName in blockYAML["templates"][baseBlock]]

    tmpl = None
    tmplPath = os.path.join(os.path.dirname(__file__), "CppEntryPoints.mako.py")
    with open(tmplPath) as f:
        tmpl = f.read()

    prefix = """# Copyright (c) 2019-{0} Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

#
# This file was auto-generated on {1}.
#
""".format(datetime.datetime.now().year, datetime.datetime.now())

    rendered = Template(tmpl).render(blockMakoParams=blockMakoParams)

    output = "{0}\n{1}".format(prefix, rendered)

    outputPath = os.path.join(outputDir, "CppEntryPoints.py")
    with open(outputPath, "w") as f:
        f.write(output)

def generateCppFactoryFile(blockYAML, outputDir):
    factories = [generateCppFactory(blockName) for blockName in blockYAML if (blockName not in NONSTANDARD)]
    for baseBlock in blockYAML["templates"]:
        factories += [generateCppFactory(blockName) for blockName in blockYAML["templates"][baseBlock]]

    factories += [generateCppFactory(blockName) for blockName in blockYAML["factoryOnly"]]

    tmpl = None
    tmplPath = os.path.join(os.path.dirname(__file__), "Factory.mako.cpp")
    with open(tmplPath) as f:
        tmpl = f.read()

    prefix = """// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

//
// This file was auto-generated on {0}.
//
""".format(datetime.datetime.now())

    rendered = Template(tmpl).render(factories=factories)

    output = "{0}\n{1}".format(prefix, rendered)

    outputPath = os.path.join(outputDir, "Factory.cpp")
    with open(outputPath, "w") as f:
        f.write(output)

if __name__ == "__main__":
    yamlPath = os.path.join(os.path.dirname(__file__), "Blocks.yaml")
    yml = None
    with open(yamlPath) as f:
        yml = yaml.load(f.read())

    outputDir = os.path.abspath(sys.argv[1])

    generatePythonCppEntrypointFile(yml, outputDir)
    generateCppFactoryFile(yml, outputDir)'''
