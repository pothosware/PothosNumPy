# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

import Pothos

import numpy
import scipy.constants
import scipy.signal

class SimpleBlock(Pothos.Block):
    def __init__(self, dtype, func):
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

def NumPySin(dtype):
    return SimpleBlock(dtype, numpy.sin)

def NumPyCos(dtype):
    return SimpleBlock(dtype, numpy.cos)

def NumPyTan(dtype):
    return SimpleBlock(dtype, numpy.tan)

def NumPyArcSin(dtype):
    return SimpleBlock(dtype, numpy.arcsin)

def NumPyArcCos(dtype):
    return SimpleBlock(dtype, numpy.arccos)

def NumPyArcTan(dtype):
    return SimpleBlock(dtype, numpy.arctan)

def NumPyHypot(dtype):
    return SimpleBlock(dtype, numpy.hypot)

def NumPyArcTan2(dtype):
    return SimpleBlock(dtype, numpy.arctan2)

def NumPyDegrees(dtype):
    return SimpleBlock(dtype, numpy.degrees)

def NumPyRadians(dtype):
    return SimpleBlock(dtype, numpy.radians)

def NumPySinh(dtype):
    return SimpleBlock(dtype, numpy.sinh)

def NumPyCosh(dtype):
    return SimpleBlock(dtype, numpy.cosh)

def NumPyTanh(dtype):
    return SimpleBlock(dtype, numpy.tanh)

def NumPyArcSinh(dtype):
    return SimpleBlock(dtype, numpy.arcsinh)

def NumPyArcCosh(dtype):
    return SimpleBlock(dtype, numpy.arccosh)

def NumPyFloor(dtype):
    return SimpleBlock(dtype, numpy.floor)

def NumPyCeil(dtype):
    return SimpleBlock(dtype, numpy.ceil)

def NumPyTrunc(dtype):
    return SimpleBlock(dtype, numpy.trunc)

def NumPyExp(dtype):
    return SimpleBlock(dtype, numpy.exp)

def NumPyExpM1(dtype):
    return SimpleBlock(dtype, numpy.expm1)

def NumPyExp2(dtype):
    return SimpleBlock(dtype, numpy.exp2)

def NumPyLog(dtype):
    return SimpleBlock(dtype, numpy.log)

def NumPyLog2(dtype):
    return SimpleBlock(dtype, numpy.log2)

def NumPyLog10(dtype):
    return SimpleBlock(dtype, numpy.log10)

def NumPyLog1P(dtype):
    return SimpleBlock(dtype, numpy.log1p)

def NumPyLogAddExp(dtype):
    return SimpleBlock(dtype, numpy.logaddexp)

def NumPyLogAddExp2(dtype):
    return SimpleBlock(dtype, numpy.logaddexp2)

def NumPySinc(dtype):
    return SimpleBlock(dtype, numpy.sinc)

def NumPyReciprocal(dtype):
    return SimpleBlock(dtype, numpy.reciprocal)

def NumPyPositive(dtype):
    return SimpleBlock(dtype, numpy.positive)

def NumPyNegative(dtype):
    return SimpleBlock(dtype, numpy.negative)

def NumPyConjugate(dtype):
    return SimpleBlock(dtype, numpy.conjugate)

def NumPySqrt(dtype):
    return SimpleBlock(dtype, numpy.sqrt)

def NumPyCbrt(dtype):
    return SimpleBlock(dtype, numpy.cbrt)

def NumPySquare(dtype):
    return SimpleBlock(dtype, numpy.square)

def NumPyAbsolute(dtype):
    return SimpleBlock(dtype, numpy.absolute)

def NumPyInvert(dtype):
    return SimpleBlock(dtype, numpy.invert)

def SciPyConstantsLambda2Nu(dtype):
    return SimpleBlock(dtype, scipy.constants.lambda2nu)

def SciPyConstantsNu2Lambda(dtype):
    return SimpleBlock(dtype, scipy.constants.nu2lambda)

def SciPySignalQuadratic(dtype):
    return SimpleBlock(dtype, scipy.signal.quadratic)

def SciPySignalCubic(dtype):
    return SimpleBlock(dtype, scipy.signal.cubic)

def SciPySignalQuadratic(dtype):
    return SimpleBlock(dtype, scipy.signal.quadratic)
