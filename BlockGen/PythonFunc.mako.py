%for block in yaml:
%endfor


def Sin(dtype):
    return OneToOneBlock(dtype, dtype, numpy.sin, useDType=False, supportFloat=True)

from .OneToOneBlock import *
from .TwoToOneBlock import *
from . import Utility

import Pothos

import numpy

%for block in yaml

///////////////////////////////////////////////////////////////////////////////
// ${yaml["name"]}
///////////////////////////////////////////////////////////////////////////////

%for include in yaml["include"]:
#include <${include}>
%endfor

// Wrap relevant KFR functions in simple wrappers to avoid dealing with
// ugly SFINAE.

%for op in yaml["operations"]:
    %if "func" in op:
KFR_POTHOS_FUNC(${op["func"]})
    %endif
    %if "cfunc" in op:
KFR_POTHOS_COMPLEX_FUNC(${op["cfunc"]})
    %endif
%endfor

// Factory
static Pothos::Block* ${yaml["name"]}Factory(
    const Pothos::DType& dtype,
    const std::string& operation)
{

%for op in yaml["operations"]:
    %if "types" in yaml:
        %for typestr in yaml["types"]:
            %if "func" in op:
    if (doesDTypeMatch<${typestr}>(dtype) && (operation == "${op["param"]}"))
        return new PothosKFRBlock<${typestr}>(dtype.dimension(), KFR_${op["func"]}<${typestr}>);
            %endif
        %endfor
    %endif
    %if "ctypes" in yaml:
        %for typestr in yaml["ctypes"]:
            %if "cfunc" in op:
    if (doesDTypeMatch<std::complex<${typestr}>>(dtype) && (operation == "${op["param"]}"))
        return new PothosKFRBlock<std::complex<${typestr}>>(dtype.dimension(), KFR_${op["cfunc"]}<std::complex<${typestr}>>);
            %endif
        %endfor
    %endif
%endfor

    throw Pothos::InvalidArgumentException(
              "${yaml["name"]}Factory("+dtype.toString()+","+operation+")",
              "unsupported args");
}

// Registration
static Pothos::BlockRegistry register${yaml["name"]}(
    "/kfr/${yaml["subpath"]}", &${yaml["name"]}Factory);
