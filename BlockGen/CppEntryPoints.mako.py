from .OneToOneBlock import *
from .TwoToOneBlock import *
from .NToOneBlock import *

import numpy

%for params in blockMakoParams:
def ${params["cppEntryPoint"]}(${params["dtypeEntryPointParams"]}${params["otherEntryPointParams"]}):
    inputArgs = ${params["inputArgs"]}
    outputArgs = ${params["outputArgs"]}

    return ${params["blockClass"]}(numpy.${params["func"]}, ${params["dtypeBlockParams"]}, inputArgs, outputArgs, ${params["otherBlockParams"]})

%endfor
