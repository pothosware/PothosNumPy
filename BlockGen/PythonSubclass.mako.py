
class ${makoVars["name"]}(${makoVars["class"]}):
    def __init__(self, ${", ".join(makoVars["factoryParams"])}):
%for var in makoVars["factoryVars"][::-1]:
        ${var} = ${makoVars[var]}
%endfor
        ${makoVars["class"]}.__init__(self, ${", ".join(makoVars["classParams"])})

%for arg in makoVars.get("funcArgs",[]):
        self.registerProbe("${arg["name"]}", "${arg["name"]}Changed", "set${arg["title"]}")
%endfor

%for arg in makoVars.get("funcArgs",[]):
        self.set${arg["title"]}(${arg["name"]})
%endfor
%for arg in makoVars.get("funcArgs",[]):

    def get${arg["title"]}(self):
    %if (arg["dtype"] == "str") and arg.get("storeParamLowercase", False):
        return self.${arg["privateVar"]}.upper()
    %else:
        return self.${arg["privateVar"]}
    %endif

    def set${arg["title"]}(self, ${arg["name"]}):
        # Input validation
    %if arg["dtype"] == "blockType":
        %if "Source" in makoVars["class"]:
        Utility.validateParameter(${arg["name"]}, self.numpyOutputDType)
        %else:
        Utility.validateParameter(${arg["name"]}, self.numpyInputDType)
        %endif
    %elif arg["dtype"] == "str":
        if type(${arg["name"]}) is not str:
            raise TypeError("${arg["name"]} must be a string")
    %else:
        Utility.validateParameter(${arg["name"]}, numpy.dtype("${arg["dtype"]}"))
    %endif
    %if "validValues" in arg:
        if ${arg["name"]} not in ${arg["validValues"]}:
            raise ValueError("Invalid value for ${arg["name"]}. Valid values: {0}".format(${arg["validValues"]}))
    %endif
    %if ">" in arg:
        if ${arg["name"]} <= ${arg[">"]}:
            raise ValueError("${arg["name"]} must be > ${arg[">"]}")
    %elif ">=" in arg:
        if ${arg["name"]} < ${arg[">="]}:
            raise ValueError("${arg["name"]} must be >= ${arg[">="]}")
    %endif
    %if "<" in arg:
        if ${arg["name"]} >= ${arg["<"]}:
            raise ValueError("${arg["name"]} must be < ${arg["<"]}")
    %elif "<=" in arg:
        if ${arg["name"]} > ${arg["<="]}:
            raise ValueError("${arg["name"]} must be <= ${arg["<="]}")
    %endif
    %if "addedValidation" in arg:
        %for line in arg["addedValidation"]:
        ${line}
        %endfor
    %endif

    %if (arg["dtype"] == "str") and arg.get("storeParamLowercase", False):
        self.${arg["privateVar"]} = ${arg["name"]}.lower()
    %else:
        self.${arg["privateVar"]} = ${arg["name"]}
    %endif
        self.__refreshArgs()

        # C++ equivalent: emitSignal("${arg["name"]}Changed", ${arg["name"]})
        self.${arg["name"]}Changed(${arg["name"]})
%endfor

    def __refreshArgs(self):
        self.funcArgs = [${", ".join(makoVars.get("funcArgsList",[]))}]
