# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from . import Utility

import Pothos

import numpy

class SimpleBlock(Pothos.Block):
    def __init__(self, dtype, func):
        Utility.validateDType(dtype)

        Pothos.Block.__init__(self)
        self.setupInput("0", dtype)
        self.setupOutput("0", dtype)

        self.func = func

    def work(self):
        in0 = self.input(0).buffer()
        out0 = self.output(0).buffer()

        n = min(len(in0), len(out0))
        self.func(in0[:n], out0[:n])

        self.input(0).consume(n)
        self.output(0).produce(n)

#
# Factories exposed to C++ layer
#

def Sin(dtype):
    return SimpleBlock(dtype, numpy.sin)

def Cos(dtype):
    return SimpleBlock(dtype, numpy.cos)

def Tan(dtype):
    return SimpleBlock(dtype, numpy.tan)

def ArcSin(dtype):
    return SimpleBlock(dtype, numpy.arcsin)

def ArcCos(dtype):
    return SimpleBlock(dtype, numpy.arccos)

def ArcTan(dtype):
    return SimpleBlock(dtype, numpy.arctan)

def Hypot(dtype):
    return SimpleBlock(dtype, numpy.hypot)

def ArcTan2(dtype):
    return SimpleBlock(dtype, numpy.arctan2)

def Degrees(dtype):
    return SimpleBlock(dtype, numpy.degrees)

def Radians(dtype):
    return SimpleBlock(dtype, numpy.radians)

def Sinh(dtype):
    return SimpleBlock(dtype, numpy.sinh)

def Cosh(dtype):
    return SimpleBlock(dtype, numpy.cosh)

def Tanh(dtype):
    return SimpleBlock(dtype, numpy.tanh)

def ArcSinh(dtype):
    return SimpleBlock(dtype, numpy.arcsinh)

def ArcCosh(dtype):
    return SimpleBlock(dtype, numpy.arccosh)

def Floor(dtype):
    return SimpleBlock(dtype, numpy.floor)

def Ceil(dtype):
    return SimpleBlock(dtype, numpy.ceil)

def Trunc(dtype):
    return SimpleBlock(dtype, numpy.trunc)

def Exp(dtype):
    return SimpleBlock(dtype, numpy.exp)

def ExpM1(dtype):
    return SimpleBlock(dtype, numpy.expm1)

def Exp2(dtype):
    return SimpleBlock(dtype, numpy.exp2)

def Log(dtype):
    return SimpleBlock(dtype, numpy.log)

def Log2(dtype):
    return SimpleBlock(dtype, numpy.log2)

def Log10(dtype):
    return SimpleBlock(dtype, numpy.log10)

def Log1P(dtype):
    return SimpleBlock(dtype, numpy.log1p)

def LogAddExp(dtype):
    return SimpleBlock(dtype, numpy.logaddexp)

def LogAddExp2(dtype):
    return SimpleBlock(dtype, numpy.logaddexp2)

def Sinc(dtype):
    return SimpleBlock(dtype, numpy.sinc)

def Reciprocal(dtype):
    return SimpleBlock(dtype, numpy.reciprocal)

def Positive(dtype):
    return SimpleBlock(dtype, numpy.positive)

def Negative(dtype):
    return SimpleBlock(dtype, numpy.negative)

def Conjugate(dtype):
    return SimpleBlock(dtype, numpy.conjugate)

def Sqrt(dtype):
    return SimpleBlock(dtype, numpy.sqrt)

def Cbrt(dtype):
    return SimpleBlock(dtype, numpy.cbrt)

def Square(dtype):
    return SimpleBlock(dtype, numpy.square)

def Absolute(dtype):
    return SimpleBlock(dtype, numpy.absolute)

def Invert(dtype):
    return SimpleBlock(dtype, numpy.invert)
