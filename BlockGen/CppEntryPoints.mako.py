%for params in blockMakoParams:
def ${params["cppEntryPoint"]}(${params["dtypeEntryPointParams"]}${params["otherEntryPointParams"]}):
    inputArgs = ${params["inputArgs"]}
    outputArgs = ${params["outputArgs"]}

    # TODO: account for sources and sinks, where one params are None
    return ${params["blockClass"]}(numpy.${params["func"]}, ${params["dtypeBlockParams"]}, inputArgs, outputArgs, ${params["otherBlockParams"]})

%endfor
