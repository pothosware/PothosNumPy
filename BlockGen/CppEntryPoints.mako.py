from .OneToOneBlock import *
from .TwoToOneBlock import *
from .NToOneBlock import *
from .Random import *
from .Source import *

import numpy

%for params in blockMakoParams:

def ${params["cppEntryPoint"]}(${params["dtypeEntryPointParams"]}${params["otherEntryPointParams"]}):
    %if "Source" in params["blockClass"]:
    outputArgs = ${params["outputArgs"]}

        %if "random" in params["func"]:
    return ${params["blockClass"]}(GetNumPyRandom().${params["func"].replace("random/","")}, ${params["dtypeBlockParams"]}, outputArgs, ${params["otherBlockParams"]})
        %else:
    return ${params["blockClass"]}(numpy.${params["func"]}, ${params["dtypeBlockParams"]}, outputArgs, ${params["otherBlockParams"]})
        %endif
    %else:
    inputArgs = ${params["inputArgs"]}
    outputArgs = ${params["outputArgs"]}

        %if "random" in params["func"]:
    return ${params["blockClass"]}(GetNumPyRandom().${params["func"].replace("random/","")}, ${params["dtypeBlockParams"]}, inputArgs, outputArgs, ${params["otherBlockParams"]})
        %else:
    return ${params["blockClass"]}(numpy.${params["func"]}, ${params["dtypeBlockParams"]}, inputArgs, outputArgs, ${params["otherBlockParams"]})
        %endif
    %endif
%endfor
