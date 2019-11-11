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
 * |category ${makoVars["category"]}
 * |keywords ${makoVars["keywords"]}
 * |factory /numpy/${makoVars["func"]}(${",".join(makoVars["factoryParams"])})
%if "alias" in makoVars:
%for alias in makoVars["alias"]:
 * |alias /numpy/${alias}
%endfor
%endif
%for i,param in enumerate(makoVars["factoryParams"]):
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
def ${makoVars["name"]}(${", ".join(makoVars["factoryParams"])}):
%for var in makoVars["factoryVars"][::-1]:
    ${var} = ${makoVars[var]}
%endfor
    return ${makoVars["class"]}(${", ".join(makoVars["classParams"])})
