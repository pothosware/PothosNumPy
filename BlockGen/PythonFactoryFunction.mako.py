"""/*
|PothosDoc ${yaml["name"]
|category ${yaml["category"]}
|keywords ${yaml["keywords"]}
|factory /numpy/${yaml["func"]}(${yaml["factoryParams"]})
*/"""
def ${yaml["name"]}(${yaml["factoryParams"]}):
    return ${yaml["class"]}(${yaml["classParams"]})
