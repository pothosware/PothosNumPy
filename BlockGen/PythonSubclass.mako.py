"""/*
|PothosDoc ${makoVars.get('niceName', makoVars['name'])}

Corresponding NumPy function: ${makoVars["prefix"]}.${makoVars["func"]}

|category ${makoVars['category']}
|keywords ${makoVars['keywords']}
|factory /numpy/${makoVars['func']}(${",".join(makoVars['factoryParams'])})
%if "alias" in makoVars:
%for alias in makoVars["alias"]:
|alias /numpy/${alias}
%endfor
%endif
*/"""
class ${makoVars["name"]}(${makoVars["class"]}):
    def __init__(self, ${", ".join(makoVars["factoryParams"])}):
%for var in makoVars["factoryVars"][::-1]:
        ${var} = ${makoVars[var]}
%endfor
        ${makoVars["class"]}.__init__(self, ${", ".join(makoVars["classParams"])})

%for arg in makoVars["funcArgs"]:
        self.set${arg["title"]}(${arg["name"]})
%endfor
%for arg in makoVars["funcArgs"]:

    def get${arg["title"]}(self):
        return self.${arg["privateVar"]}

    def set${arg["title"]}(self, ${arg["name"]}):
        # Input validation
    %if arg["dtype"] == "blockType":
        %if "Source" in makoVars["class"]:
        Utility.validateParameter(${arg["name"]}, self.numpyOutputDType)
        %else:
        Utility.validateParameter(${arg["name"]}, self.numpyInputDType)
        %endif
    %else:
        Utility.validateParameter(${arg["name"]}, numpy.dtype("${arg["dtype"]}"))
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

        self.${arg["privateVar"]} = ${arg["name"]}
        self.__refreshArgs()
%endfor

    def __refreshArgs(self):
        self.funcArgs = [${", ".join(makoVars["funcArgsList"])}]
