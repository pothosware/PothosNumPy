#!/usr/bin/env python

import datetime
import json
from mako.template import Template
import mako.exceptions
import numpy
import os
import sys
import yaml

#
# Manual or version-specific helper stuff
#

if numpy.__version__ >= "1.17.0":
    NumPyRandomString = "numpy.random.default_rng()"
    NumPyRandomIntegersString = "integers"
else:
    NumPyRandomString = "numpy.random"
    NumPyRandomIntegersString = "randint"

def nonCentralChiSquareDFMin():
    return 1 if (numpy.__version__ >= "1.10.0") else 2

# Copied from NumPy source
def poissonLamMax():
    return (numpy.iinfo('int64').max - numpy.sqrt(numpy.iinfo('int64').max)*10)

HardcodedValues = dict(
    NonCentralChiSquareDFMin=nonCentralChiSquareDFMin(),
    PoissonLamMax=poissonLamMax()
)

def getMapEntryIfStr(val):
    return HardcodedValues[val] if (type(val) is str) else val

#
# Templates
#

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

#
# Test code
#

def processYAMLFile(yamlPath):
    yml = None
    with open(yamlPath) as f:
        yml = yaml.load(f.read())

    if not yml:
        raise RuntimeError("No YAML found in {0}".format(yamlPath))

    # These entries are fully defined, so when other entries have a "copy"
    # field, the script will look in this dict.
    templateEntries = {k:v for k,v in yml.items() if "copy" not in v}

    fullEntries = templateEntries.copy()
    for k,v in yml.items():
        if "copy" in v:
            if v["copy"] not in templateEntries:
                raise RuntimeError('Could not find template entry: "{0}".'.format(v["copy"]))

            fullEntry = templateEntries[v["copy"]].copy()
            for keyToErase in ["alias", "niceName", "description"]:
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

def blockTypeToDTypeChooser(blockTypeYAML):
    if "all" in blockTypeYAML:
        yamlToProcess = ["int", "uint", "float", "complex"]
    else:
        yamlToProcess = blockTypeYAML

    args = dict()
    for typeStr in yamlToProcess:
        args[typeStr.replace("complex","cfloat")] = 1

    return args

def blockTypeToDTypeDefault(blockTypeYAML):
    dtypeChooser = blockTypeToDTypeChooser(blockTypeYAML)

    if dtypeChooser.get("cfloat",0) == 1:
        defaultType = "complex_float64"
    elif "float" in dtypeChooser:
        defaultType = "float64"
    else:
        defaultType = list(dtypeChooser.keys())[0].replace("complex", "complex_float").split("=")[0] + "64"

    return defaultType

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
            makoVars["outputDTypeChooser"] = blockTypeToDTypeChooser(yaml[key])
            makoVars["outputDTypeDefault"] = blockTypeToDTypeDefault(yaml[key])
            makoVars["classParams"] = ["outputDTypeArgs"] + makoVars["classParams"]
            makoVars["factoryVars"] += ["outputDTypeArgs"]
            break
    for key in ["blockType", "inputType"]:
        if key in yaml:
            makoVars["inputDTypeArgs"] = blockTypeToDictString(yaml[key])
            makoVars["inputDTypeChooser"] = blockTypeToDTypeChooser(yaml[key])
            makoVars["inputDTypeDefault"] = blockTypeToDTypeDefault(yaml[key])
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
            makoVars["outputDTypeChooser"] = blockTypeToDTypeChooser(yaml[key])
            makoVars["outputDTypeDefault"] = blockTypeToDTypeDefault(yaml[key])
            makoVars["classParams"] = ["outputDTypeArgs"] + makoVars["classParams"]
            makoVars["factoryVars"] += ["outputDTypeArgs"]
            break

    makoVars["classParams"] = ["dtype"] + makoVars["classParams"]
    makoVars["factoryParams"] = ["dtype"] + makoVars["factoryParams"]

    if yaml["class"] in ["FixedSingleOutputSource"]:
        makoVars["classParams"] += ["repeat"]
        makoVars["factoryParams"] += ["repeat"]

# The added _ prevents Python from trying to use the Python type as the key.
ParamWidgets = dict(
    float_="DoubleSpinBox",
    int_="SpinBox",
    uint_="SpinBox",
    str_="ComboBox",
)

def generateMakoVars(func,yaml):
    # Generate variables for processing.
    makoVars = dict()
    makoVars["name"] = yaml["name"]
    makoVars["subclass"] = yaml.get("subclass", False)
    makoVars["class"] = yaml["class"]
    makoVars["category"] = " ".join(yaml["categories"])
    makoVars["func"] = func
    makoVars["pothosDocFunc"] = makoVars["func"].replace("randint", "integers").replace("integers", NumPyRandomIntegersString)
    makoVars["keywords"] = yaml.get("keywords", [func])
    makoVars["prefix"] = yaml.get("prefix", "numpy")
    makoVars["pothosDocPrefix"] = makoVars["prefix"].replace("Random.NumPyRandom", NumPyRandomString).replace("Random", NumPyRandomString)
    makoVars["factoryVars"] = []

    if "funcArgs" in yaml:
        for arg in yaml["funcArgs"]:
            arg["title"] = arg["name"][0].upper() + arg["name"][1:]
            arg["privateVar"] = "__{0}".format(arg["name"])
            if arg["dtype"]+"_" in ParamWidgets:
                arg["widget"] = ParamWidgets[arg["dtype"]+"_"]
                arg["widgetArgs"] = dict()
                if arg["widget"] == "ComboBox":
                   arg["widgetArgs"]["editable"] = False
                else:
                    if ">=" in arg:
                        arg[">="] = getMapEntryIfStr(arg[">="])
                        arg["widgetArgs"]["minimum"] = str(arg[">="])
                    elif ">" in arg:
                        arg[">"] = getMapEntryIfStr(arg[">"])
                        diff = 1 if (arg["widget"] == "SpinBox") else 0.01
                        arg["widgetArgs"]["minimum"] = str(arg[">"]+diff)
                    if "<=" in arg:
                        arg["<="] = getMapEntryIfStr(arg["<="])
                        arg["widgetArgs"]["maximum"] = str(arg["<="])
                    elif "<" in arg:
                        arg["<"] = getMapEntryIfStr(arg["<"])
                        diff = 1 if (arg["widget"] == "SpinBox") else 0.01
                        arg["widgetArgs"]["maximum"] = str(arg["<"]-diff)
        makoVars["funcArgsList"] = ["self.{0}".format(arg["privateVar"]) for arg in yaml["funcArgs"]]

    # Some keys are just straight copies.
    for key in ["alias", "niceName", "funcArgs", "factoryPrefix", "nanFunc"]:
        if key in yaml:
            makoVars[key] = yaml[key]

    if "description" in yaml:
        makoVars["description"] = yaml["description"]

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

    if "nanFunc" in makoVars:
        funcAsParam = "({0}.{1} if ignoreNaN else {0}.{2})".format(makoVars["prefix"], makoVars["nanFunc"], func)
        makoVars["factoryParams"] += ["ignoreNaN"]
    else:
        funcAsParam = "{0}.{1}".format(makoVars["prefix"], func)

    makoVars["classParams"] = ['"{0}/{1}"'.format(makoVars.get("factoryPrefix","/numpy"), func)] + [funcAsParam] + makoVars["classParams"]

    makoVars["classParams"] += ["list()"]
    makoVars["classParams"] += ["funcKWargs" if "funcKWargs" in makoVars else "dict()"]
    if "args" in makoVars:
        makoVars["classParams"] += ["*args"]
    if "kwargs" in makoVars:
        makoVars["classParams"] += ["**kwargs"]

    return makoVars

def addParameterBoundsToDesc(desc, funcArg):
    wasEmpty = (len(desc) == 0)
    if desc:
        desc += "\n"

    if ">" in funcArg:
        desc += "\nLower bound: {0} (exclusive)\n".format(funcArg[">"])
    elif ">=" in funcArg:
        desc += "\nLower bound: {0} (inclusive)\n".format(funcArg[">="])

    if "<" in funcArg:
        desc += "\nUpper bound: {0} (exclusive)\n".format(funcArg["<"])
    elif "<=" in funcArg:
        desc += "\nUpper bound: {0} (inclusive)\n".format(funcArg["<="])

    # If there was no incoming description, don't start it with a newline.
    if wasEmpty:
        desc = desc[1:]

    return desc.rstrip("\n")

def makoVarsToBlockDesc(makoVars):
    desc = dict()
    desc["name"] = makoVars.get("niceName", makoVars["name"])
    desc["path"] = "/numpy/"+makoVars["blockRegistryPath"]
    desc["args"] = makoVars["factoryParams"]
    desc["keywords"] = makoVars["keywords"]
    desc["categories"]  = makoVars["category"].split(" ")
    if "description" in makoVars:
        desc["docs"] = makoVars["description"].split("\n")
    else:
        desc["docs"] = []

    if makoVars["class"] in ["OneToOneBlock", "TwoToOneBlock", "NToOneBlock"]:
        desc["docs"] += ["<p>Corresponding NumPy function: <b>numpy.{0}</b></p>".format(makoVars["func"])]
    elif makoVars["class"] == "ForwardAndPostLabelBlock":
        if "nanFunc" in makoVars:
            desc["docs"] += ["<p>Corresponding NumPy functions: <b>numpy.{0}</b>, <b>numpy.{1}</b></p>".format(makoVars["func"], makoVars["nanFunc"])]
        else:
            desc["docs"] += ["<p>Corresponding NumPy function: <b>numpy.{0}</b></p>".format(makoVars["func"])]

    if "alias" in makoVars:
        desc["alias"] = ["/numpy/"+alias for alias in makoVars["alias"]]
        desc["keywords"] += makoVars["alias"]

    if "factoryParams" in makoVars:
        desc["params"] = list()
        desc["calls"] = list()

        for factoryParam in makoVars["factoryParams"]:
            param = dict(key=factoryParam, preview="enable")

            if "dtype" in factoryParam.lower():
                param["widgetType"] = "DTypeChooser"
                param["preview"] = "disable"

            if factoryParam == "dtype":
                param["name"] = "Data Type"
                param["desc"] = ["The block's data type."]
                param["default"] = "\"{0}\"".format(makoVars["outputDTypeDefault"])
                param["widgetKwargs"] = makoVars["outputDTypeChooser"]
            elif factoryParam == "inputDType":
                param["name"] = "Input Data Type"
                param["desc"] = ["The block's input data type."]
                param["default"] = "\"{0}\"".format(makoVars["inputDTypeDefault"])
                param["widgetKwargs"] = makoVars["inputDTypeChooser"]
            elif factoryParam == "outputDType":
                param["name"] = "Output Data Type"
                param["desc"] = ["The block's output data type."]
                param["default"] = "\"{0}\"".format(makoVars["outputDTypeDefault"])
                param["widgetKwargs"] = makoVars["outputDTypeChooser"]
            elif factoryParam == "nchans":
                param["name"] = "Num Channels"
                param["desc"] = ["The number of input channels for this block."]
                param["default"] = "2"
                param["preview"] = "disable"
                param["widgetType"] = "SpinBox"
                param["widgetKwargs"] = dict(minimum=2)
            elif factoryParam == "repeat":
                param["name"] = "Repeat?"
                param["desc"] = ["Whether or not a block with a fixed output repeatedly posts this output."]
                param["default"] = "false"
                param["widgetType"] = "ToggleSwitch"
                param["widgetKwargs"] = dict(on="True",off="False")
            elif factoryParam == "ignoreNaN":
                param["name"] = "Ignore NaN?"
                param["desc"] = ["If this is set to \"true\", the output buffer will have all occurrences of <b>NaN</b> removed."]
                param["default"] = "false"
                param["widgetType"] = "ToggleSwitch"
                param["widgetKwargs"] = dict(on="True",off="False")
            else:
                # Find this parameter in our funcArgs, which has all this info.
                funcArg = [funcArg for funcArg in makoVars.get("funcArgs",[]) if funcArg["name"] == factoryParam][0]
                param["name"] = funcArg.get("flowLabel", funcArg["title"])
                if funcArg["dtype"] == "str":
                    param["widgetType"] = "ComboBox"

                    if "validValues" in funcArg:
                        param["options"] = [dict(name=val.title(),value="\"{0}\"".format(val)) for val in funcArg["validValues"]]
                        param["default"] = "\"{0}\"".format(funcArg["validValues"][0])
                        param["widgetKwargs"] = dict(editable="false")
                    else:
                        param["default"] = "\"{0}\"".format(funcArg.get("default", ""))
                        param["widgetKwargs"] = dict(editable="true")
                else:
                    param["default"] = str(funcArg.get("default", "0"))

                if "description" in funcArg:
                    param["desc"] = funcArg["description"]
                if "widget" in funcArg:
                    param["widgetType"] = funcArg["widget"]
                if "widgetArgs" in funcArg:
                    param["widgetKwargs"] = funcArg["widgetArgs"]

                if any([op in funcArg for op in ["<","<=",">",">="]]):
                    param["desc"] = addParameterBoundsToDesc(param.get("desc",""), funcArg)

                if "desc" in param:
                    param["desc"] = param["desc"].split("\n")

                desc["calls"].append(dict(
                    type="setter",
                    name="set" + factoryParam[0].upper() + factoryParam[1:],
                    args=factoryParam))

            desc["params"].append(param)

    # Encode the block description into escaped JSON
    descEscaped = "".join([hex(ord(ch)).replace("0x", "\\x") for ch in json.dumps(desc)])
    return "Pothos::PluginRegistry::add(\"{0}\", std::string(\"{1}\"));".format(makoVars["docRegistryPath"], descEscaped)

def generatePythonEntryPoint(makoVars):
    try:
        if makoVars["subclass"]:
            entryPoint = Template(PythonSubclassTemplate).render(makoVars=makoVars)
        else:
            entryPoint = Template(PythonFactoryFunctionTemplate).render(makoVars=makoVars)
    except:
        print(mako.exceptions.text_error_template().render())

    return "{0}\n# /numpy/{1}\n{2}\n{3}".format("#"*79, makoVars["blockRegistryPath"], "#"*79, entryPoint)

def generateCppOutput(allMakoVars):
    prefix = """// Copyright (c) 2019-{0} Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

//
// This file was auto-generated on {1}.
//
""".format(Now.year, Now)

    factories = []
    docs = []
    for makoVars in allMakoVars:
        factories += [generateCppFactory(makoVars["blockRegistryPath"], makoVars["name"])]
        docs += [makoVarsToBlockDesc(makoVars)]
        if "alias" in makoVars:
            for alias in makoVars["alias"]:
                factories += [generateCppFactory(alias, makoVars["name"])]

    # Add C++-only blocks.
    factoryOnlyYAMLPath = os.path.join(BlocksDir, "FactoryOnly.yaml")
    with open(factoryOnlyYAMLPath) as f:
        factoryOnlyYAML = yaml.load(f.read())

    for k,v in factoryOnlyYAML.items():
        factories += [generateCppFactory(k,v["name"])]
    if "alias" in v:
        for alias in v["alias"]:
            factories += [generateCppFactory(alias,v["name"])]

    try:
        rendered = Template(CppFactoryTemplate).render(factories=factories, docs=docs)
    except:
        print(mako.exceptions.text_error_template().render())

    output = "{0}\n{1}".format(prefix, rendered)

    outputFilepath = os.path.join(OutputDir, "Factory.cpp")
    with open(outputFilepath, 'w') as f:
        f.write(output)

def generatePythonOutput(allMakoVars):
    prefix = """# Copyright (c) 2019-{0} Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

#
# This file was auto-generated on {1}.
#

from . import Random
from . import Utility

from .OneToOneBlock import *
from .TwoToOneBlock import *
from .NToOneBlock import *
from .ForwardAndPostLabelBlock import *
from .Source import *
""".format(Now.year, Now)

    try:
        pythonEntryPointsList = [generatePythonEntryPoint(makoVars) for makoVars in allMakoVars]
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
    badParamsMap = dict(
        Integer="badIntValues",
        UnsignedInt="badUIntValues",
        Float="badFloatValues",
        Complex="badComplexValues"
    )

    maxNumParams = max([len(expandedYAML[block].get("funcArgs", [])) for block in expandedYAML])
    if maxNumParams > 0:
        # Put quotes around test string values.
        for block in expandedYAML:
            for arg in expandedYAML[block].get("funcArgs", []):
                if arg["dtype"] == "str":
                    for key in ["testValue1", "testValue2"]:
                        if key in arg:
                            assert(type(arg[key]) is str)
                            # For some reason, this format adds two quotes
                            arg[key] = '"{0}"'.format(arg[key]).replace('""', '"')
                    for key in ["validValues", "badValues"]:
                        if key in arg:
                            assert(type(arg[key]) is list)
                            # For some reason, this format adds two quotes
                        arg[key] = ['"{0}"'.format(val).replace('""', '"') for val in arg[key]]

    try:
        output = Template(BlockExecutionTestTemplate).render(blockYAML=expandedYAML, Now=Now, sfinaeMap=sfinaeMap, maxNumParams=maxNumParams, badParamsMap=badParamsMap)
    except:
        print(mako.exceptions.text_error_template().render())

    outputFilepath = os.path.join(OutputDir, "BlockExecutionTestAuto.cpp")
    with open(outputFilepath, 'w') as f:
        f.write(output)

if __name__ == "__main__":
    yamlFiles = [os.path.join(BlocksDir, filepath) for filepath in os.listdir(BlocksDir) if os.path.splitext(filepath)[1] == ".yaml"]

    expandedYAMLList = [processYAMLFile(yamlFile) for yamlFile in yamlFiles if "FactoryOnly.yaml" not in yamlFile]

    # At this point, we have a list of dictionaries, so consolidate them.
    expandedYAML = dict()
    for yml in expandedYAMLList:
        expandedYAML.update(yml)

    allMakoVars = []
    for k,v in expandedYAML.items():
        if not v.get("factoryOnly", False):
            makoVars = generateMakoVars(k.split("/")[-1], v)
            makoVars["blockRegistryPath"] = k
            makoVars["docRegistryPath"] = "/blocks/docs/numpy/{0}".format(k)
            allMakoVars += [makoVars]

    populateTemplates()
    generateCppOutput(allMakoVars)
    generatePythonOutput(allMakoVars)
    generateBlockExecutionTest(expandedYAML)
