# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from . import Utility

import Pothos

import numpy

class OneToOneBlock(Pothos.Block):
    def __init__(self, dtype, func, *args, useDType=True):
        Utility.validateDType(dtype)

        Pothos.Block.__init__(self)
        self.setupInput("0", dtype)
        self.setupOutput("0", dtype)

        self.func = func
        self.args = args
        self.useDType = useDType

    def work(self):
        elems = self.workInfo().minAllElements
        if 0 == elems:
            return

        in0 = self.input(0).buffer()
        out0 = self.output(0).buffer()
        numpyDType = self.input(0).dtype()

        N = min(len(in0), len(out0))

        if self.useDType:
            out0[:N] = self.func(in0[:N], *self.args, dtype=numpyDType)
        else:
            out0[:N] = self.func(in0[:N], *self.args)

        self.input(0).consume(elems)
        self.output(0).produce(elems)

#
# Factories exposed to C++ layer
#

def Sin(dtype):
    return OneToOneBlock(dtype, numpy.sin, useDType=False)

def Cos(dtype):
    return OneToOneBlock(dtype, numpy.cos, useDType=False)

def Tan(dtype):
    return OneToOneBlock(dtype, numpy.tan, useDType=False)

def ArcSin(dtype):
    return OneToOneBlock(dtype, numpy.arcsin, useDType=False)

def ArcCos(dtype):
    return OneToOneBlock(dtype, numpy.arccos, useDType=False)

def ArcTan(dtype):
    return OneToOneBlock(dtype, numpy.arctan, useDType=False)

def Hypot(dtype):
    return OneToOneBlock(dtype, numpy.hypot, useDType=False)

def ArcTan2(dtype):
    return OneToOneBlock(dtype, numpy.arctan2, useDType=False)

def Degrees(dtype):
    return OneToOneBlock(dtype, numpy.degrees, useDType=False)

def Radians(dtype):
    return OneToOneBlock(dtype, numpy.radians, useDType=False)

def Sinh(dtype):
    return OneToOneBlock(dtype, numpy.sinh, useDType=False)

def Cosh(dtype):
    return OneToOneBlock(dtype, numpy.cosh, useDType=False)

def Tanh(dtype):
    return OneToOneBlock(dtype, numpy.tanh, useDType=False)

def ArcSinh(dtype):
    return OneToOneBlock(dtype, numpy.arcsinh, useDType=False)

def ArcCosh(dtype):
    return OneToOneBlock(dtype, numpy.arccosh, useDType=False)

def Floor(dtype):
    return OneToOneBlock(dtype, numpy.floor, useDType=False)

def Ceil(dtype):
    return OneToOneBlock(dtype, numpy.ceil, useDType=False)

def Trunc(dtype):
    return OneToOneBlock(dtype, numpy.trunc, useDType=False)

def Exp(dtype):
    return OneToOneBlock(dtype, numpy.exp, useDType=False)

def ExpM1(dtype):
    return OneToOneBlock(dtype, numpy.expm1, useDType=False)

def Exp2(dtype):
    return OneToOneBlock(dtype, numpy.exp2, useDType=False)

def Log(dtype):
    return OneToOneBlock(dtype, numpy.log, useDType=False)

def Log2(dtype):
    return OneToOneBlock(dtype, numpy.log2, useDType=False)

def Log10(dtype):
    return OneToOneBlock(dtype, numpy.log10, useDType=False)

def Log1P(dtype):
    return OneToOneBlock(dtype, numpy.log1p, useDType=False)

def LogAddExp(dtype):
    return OneToOneBlock(dtype, numpy.logaddexp, useDType=False)

def LogAddExp2(dtype):
    return OneToOneBlock(dtype, numpy.logaddexp2, useDType=False)

def Sinc(dtype):
    return OneToOneBlock(dtype, numpy.sinc, useDType=False)

def Reciprocal(dtype):
    return OneToOneBlock(dtype, numpy.reciprocal, useDType=False)

def Positive(dtype):
    return OneToOneBlock(dtype, numpy.positive, useDType=False)

def Negative(dtype):
    return OneToOneBlock(dtype, numpy.negative, useDType=False)

def Conjugate(dtype):
    return OneToOneBlock(dtype, numpy.conjugate, useDType=False)

def Sqrt(dtype):
    return OneToOneBlock(dtype, numpy.sqrt, useDType=False)

def Cbrt(dtype):
    return OneToOneBlock(dtype, numpy.cbrt, useDType=False)

def Square(dtype):
    return OneToOneBlock(dtype, numpy.square, useDType=False)

def Absolute(dtype):
    return OneToOneBlock(dtype, numpy.absolute, useDType=False)

def Invert(dtype):
    return OneToOneBlock(dtype, numpy.invert, useDType=False)

def RInt(dtype):
    return OneToOneBlock(dtype. numpy.rint)

def Fix(dtype):
    return OneToOneBlock(dtype. numpy.fix)
