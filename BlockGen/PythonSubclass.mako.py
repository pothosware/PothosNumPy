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
        Utility.validateParameter(${arg["name"]}, numpy.dtype("${arg["dtype"]}"))
        self.${arg["privateVar"]} = ${arg["name"]}
        self.__refreshArgs()
%endfor

    def __refreshArgs(self):
        self.funcArgs = [${", ".join(makoVars["funcArgsList"])}]
