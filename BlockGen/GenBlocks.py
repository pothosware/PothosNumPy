#!/usr/bin/env python

import datetime
from mako.template import Template
import os
import sys
import yaml

import json

def generatePythonMakoParams(blockName, blockYAML):
    makoDict = dict()

    makoDict["func"] = blockName
    makoDict["cppEntryPoint"] = blockName.title().replace("_","")
    makoDict["blockClass"] = blockYAML["blockClass"]
    makoDict["dtypeEntryPointParams"] = "dtype" if blockYAML["singleDType"] else "inputDType, outputDType"
    makoDict["dtypeBlockParams"] = "dtype, dtype" if blockYAML["singleDType"] else "inputDType, outputDType"

    inputArgs = blockYAML["inputArgs"]
    makoDict["inputArgs"] = "dict({0})".format(", ".join(['"{0}"={1}'.format(x, inputArgs[x]) for x in inputArgs]))

    outputArgs = blockYAML["outputArgs"]
    makoDict["outputArgs"] = "dict({0})".format(", ".join(['"{0}"={1}'.format(x, outputArgs[x]) for x in outputArgs]))

    if "otherEntryPointParams" in blockYAML:
        makoDict["otherEntryPointParams"] = ", ".join(blockYAML["otherEntryPointParams"])
    else:
        makoDict["otherEntryPointParams"] = ""

    otherBlockParams = "useDType={0}".format(blockYAML.get("useDType", False))
    if ("otherBlockParams" in blockYAML) and (len(blockYAML["otherBlockParams"] > 0)):
        otherBlockParams = "{0}, {1}".format(", ".join(blockYAML["otherBlockParams"]), otherBlockParams)
    makoDict["otherBlockParams"] = otherBlockParams

    return makoDict

def generateCppFactory(blockName):
    return 'Pothos::BlockRegistry("numpy/{0}", Pothos::Callable(&FactoryFunc).bind<std::string>("{1}", 2))' \
           .format(blockName, blockName.title().replace("_",""))

def generatePythonCppEntrypointFile(blockYAML):
    blockMakoParams = [generatePythonMakoParams(blockName, blockYAML[blockName]) for blockName in blockYAML if (blockName != "templates")]
    for baseBlock in blockYAML["templates"]:
        blockMakoParams += [generatePythonMakoParams(blockName, blockYAML[baseBlock]) for blockName in blockYAML["templates"][baseBlock]]

    tmpl = None
    tmplPath = os.path.join(os.path.dirname(__file__), "CppEntryPoints.mako.py")
    with open(tmplPath) as f:
        tmpl = f.read()

    prefix = """
# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

#
# This file was auto-generated on {0}.
#
""".format(datetime.datetime.now())

    rendered = Template(tmpl).render(blockMakoParams=blockMakoParams)

    output = "{0}\n{1}".format(prefix, rendered)
    print(output)

def generateCppFactoryFile(blockYAML):
    factories = [generateCppFactory(blockName) for blockName in blockYAML if (blockName != "templates")]
    for baseBlock in blockYAML["templates"]:
        factories += [generateCppFactory(blockName) for blockName in blockYAML["templates"][baseBlock]]

    tmpl = None
    tmplPath = os.path.join(os.path.dirname(__file__), "Factory.mako.cpp")
    with open(tmplPath) as f:
        tmpl = f.read()

    prefix = """
// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

//
// This file was auto-generated on {0}.
//
""".format(datetime.datetime.now())

    rendered = Template(tmpl).render(factories=factories)

    output = "{0}\n{1}".format(prefix, rendered)
    print(output)

if __name__ == "__main__":
    yamlPath = os.path.join(os.path.dirname(__file__), "Blocks.yaml")
    yml = None
    with open(yamlPath) as f:
        yml = yaml.load(f.read())

    generatePythonCppEntrypointFile(yml)
    generateCppFactoryFile(yml)
