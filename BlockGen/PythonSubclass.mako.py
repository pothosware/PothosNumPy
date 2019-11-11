"""
/*
 * |PothosDoc ${makoVars.get('niceName', makoVars['name'])}
 *
%if "description" in makoVars:
 * ${makoVars["description"]}
 *
%endif
 * Corresponding NumPy function: ${makoVars["prefix"]}.${makoVars["func"]}
 *
 * |category ${makoVars['category']}
 * |keywords ${makoVars['keywords']}
 * |factory /numpy/${makoVars['func']}(${",".join(makoVars['factoryParams'])})
%if "alias" in makoVars:
%for alias in makoVars["alias"]:
 * |alias /numpy/${alias}
%endfor
%endif
%for param in makoVars["factoryParams"]:
 *
    %if param == "dtype":
 * |param dtype(Data Type) The block data type.
 * |widget DTypeChooser(${makoVars["outputDTypeChooser"]})
 * |default "${makoVars["outputDTypeDefault"]}"
 * |preview enable
    %elif param == "inputDType":
 * |param outputDType(Data Type) The input data type.
 * |widget DTypeChooser(${makoVars["inputDTypeChooser"]})
 * |default "${makoVars["inputDTypeDefault"]}"
 * |preview enable
    %elif param == "outputDType":
 * |param outputDType(Data Type) The output data type.
 * |widget DTypeChooser(${makoVars["outputDTypeChooser"]})
 * |default "${makoVars["outputDTypeDefault"]}"
 * |preview enable
    %elif param == "nchans":
 * |param nchans(Num Channels)
 * |default 1
 * |widget SpinBox(minimum=1)
 * |preview disable
    %else:
        %for arg in [funcArg for funcArg in makoVars["funcArgs"] if funcArg["name"] == param]:
 * |param ${arg["name"]}(${arg["title"]})
            %if "widget" in arg:
 * |widget ${arg["widget"]}(minimum=${arg.get("default", 0)})
            %endif
 * |default ${arg.get("default", 0)}
 * |preview enable
        %endfor
    %endif
%endfor
*/
"""
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
    %if "addedValidation" in arg:
        %for line in arg["addedValidation"]:
        ${line}
        %endfor
    %endif

        self.${arg["privateVar"]} = ${arg["name"]}
        self.__refreshArgs()
%endfor

    def __refreshArgs(self):
        self.funcArgs = [${", ".join(makoVars["funcArgsList"])}]
