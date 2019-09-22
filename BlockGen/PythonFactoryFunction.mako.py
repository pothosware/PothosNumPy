"""/*
|PothosDoc ${makoVars.get('niceName', makoVars['name'])}

Corresponding NumPy function: ${makoVars["prefix"]}.${makoVars["func"]}

|category ${makoVars['category']}
|keywords ${makoVars['keywords']}
|factory /numpy/${makoVars['func']}(${", ".join(makoVars['factoryParams'])})
%if "alias" in makoVars:
%for alias in makoVars["alias"]:
|alias /numpy/${alias}
%endfor
%endif
*/"""
def ${makoVars["name"]}(${", ".join(makoVars["factoryParams"])}):
%for var in makoVars["factoryVars"][::-1]:
    ${var} = ${makoVars[var]}
%endfor
    return ${makoVars["class"]}(${", ".join(makoVars["classParams"])})
