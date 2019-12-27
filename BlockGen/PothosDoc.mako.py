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
 * |preview disable
    %elif param == "inputDType":
 * |param inputDType(Data Type) The input data type.
 * |widget DTypeChooser(${makoVars["inputDTypeChooser"]})
 * |default "${makoVars["inputDTypeDefault"]}"
 * |preview disable
    %elif param == "outputDType":
 * |param outputDType(Data Type) The output data type.
 * |widget DTypeChooser(${makoVars["outputDTypeChooser"]})
 * |default "${makoVars["outputDTypeDefault"]}"
 * |preview disable
    %elif param == "nchans":
 * |param nchans(Num Channels)
 * |default 2
 * |widget SpinBox(minimum=2)
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
        %for arg in [funcArg for funcArg in makoVars.get("funcArgs",[]) if funcArg["name"] == param]:
 * |param ${arg["name"]}(${arg["title"]})
            %if "widget" in arg:
 * |widget ${arg["widget"]}(${arg["widgetArgs"]})
            %endif
            %if arg["dtype"] == "str":
                %if "validValues" in arg:
 * |default "${arg["validValues"][0]}"
                    %for val in arg["validValues"]:
 * |option [${val.title()}] "${val}"
                    %endfor
                %else:
 * |default ${arg.get("default", "")}
                %endif
            %else:
 * |default ${arg.get("default", 0)}
            %endif
 * |preview enable
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
