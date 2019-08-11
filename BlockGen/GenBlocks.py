#!/usr/bin/env python

import datetime
from mako.template import Template
import os
import sys
import yaml

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
        Nanvar="NaNVar")

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

    prefix = """# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

#
# This file was auto-generated on {0}.
#
""".format(datetime.datetime.now())

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
    generateCppFactoryFile(yml, outputDir)
