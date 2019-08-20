"""/*
|PothosDoc ${makoVars['name']}
|category ${makoVars['category']}
|keywords ${makoVars['keywords']}
|factory /numpy/${makoVars['func']}(${makoVars['factoryParams']})
%if "alias" in makoVars:
%for alias in makoVars["alias"]:
|alias /numpy/${alias}
%endfor
%endif
*/"""
def ${makoVars["name"]}(${makoVars["factoryParams"]}):
%if "args" in makoVars:
    args = ${makoVars["args"]}
%endif
%if "kwargs" in makoVars:
    kwargs = ${makoVars["kwargs"]}
%endif
    return ${makoVars["class"]}(${makoVars["prefix"]}.${makoVars["func"]}, ${makoVars["classParams"]})
