from .OneToOneBlock import *
from .TwoToOneBlock import *
from .NToOneBlock import *
from .Source import *

import numpy

%for params in blockMakoParams:

def ${params["cppEntryPoint"]}(${params["dtypeEntryPointParams"]}${params["otherEntryPointParams"]}):
    %if "Source" in params["blockClass"]:
    outputArgs = ${params["outputArgs"]}
    return ${params["blockClass"]}(numpy.${params["func"]}, ${params["dtypeBlockParams"]}, outputArgs, ${params["otherBlockParams"]})
%else:
    inputArgs = ${params["inputArgs"]}
    outputArgs = ${params["outputArgs"]}

    return ${params["blockClass"]}(numpy.${params["func"]}, ${params["dtypeBlockParams"]}, inputArgs, outputArgs, ${params["otherBlockParams"]})
    %endif
%endfor
