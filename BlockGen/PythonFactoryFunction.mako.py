
def ${makoVars["name"]}(${", ".join(makoVars["factoryParams"])}):
%for var in makoVars["factoryVars"][::-1]:
    ${var} = ${makoVars[var]}
%endfor
    return ${makoVars["class"]}(${", ".join(makoVars["classParams"])})
