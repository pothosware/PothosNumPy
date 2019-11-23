"""
/*
 * |PothosDoc ${makoVars.get('niceName', makoVars['name'])}
 *
%if "description" in makoVars:
 * ${makoVars["description"]}
 *
%endif
%if "nanFunc" in makoVars:
 * Corresponding NumPy functions: ${makoVars["pothosDocPrefix"]}.${makoVars["pothosDocFunc"]}, ${makoVars["pothosDocPrefix"]}.${makoVars["nanFunc"]}
%else:
 * Corresponding NumPy function: ${makoVars["pothosDocPrefix"]}.${makoVars["pothosDocFunc"]}
%endif
 *
 * |category ${makoVars["category"]}
 * |keywords ${makoVars["keywords"]}
 * |factory ${makoVars.get("factoryPrefix", "/numpy")}/${makoVars["func"]}(${",".join(makoVars["factoryParams"])})
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
    %elif param == "repeat":
 * |param repeat(Repeat?)
 * |default false
 * |widget ToggleSwitch(on="True",off="False")
 * |preview enable
    %elif param == "ignoreNaN":
 * |param ignoreNaN(Ignore NaN?)
 * |default false
 * |widget ToggleSwitch(on="True",off="False")
 * |preview enable
    %else:
        %for arg in [funcArg for funcArg in makoVars["funcArgs"] if funcArg["name"] == param]:
 * |param ${arg["name"]}(${arg["title"]})
            %if "widget" in arg:
 * |widget ${arg["widget"]}(minimum=${arg.get("default", 0)})
            %endif
 * |default ${arg.get("default", 0)}
 * |preview valid
        %endfor
    %endif
%endfor
%if "funcArgs" in makoVars:
 *
    %for arg in makoVars["funcArgs"]:
 * |setter set${arg["title"]}(${arg["name"]})
    %endfor
%endif
%if "repeat" in makoVars["factoryParams"]:
 * |setter setRepeat(repeat)
%endif
*/
"""
