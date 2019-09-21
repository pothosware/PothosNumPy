# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *
from .OneToOneBlock import *
from .Source import *
from . import Utility

import Pothos

import numpy

# Thin compatibility layer over NumPy's change from a module to a class.
# The actual type being returned will differ between NumPy versions, but
# the functions will be the same off of this.
def GetNumPyRandom():
    rand = None
    try:
        rand = numpy.random.default_rng()
    except:
        rand = numpy.random

    return rand

def GetNumPyRandomIntegersFunc():
    randints = None
    try:
        rand = numpy.random.default_rng().integers
    except:
        rand = numpy.random.random_integers

    return rand

#
# Blocks whose invocations ore implementations are different enough to not auto-generate
#

class Permutation(OneToOneBlock):
    def __init__(self, dtype):
        dtypeArgs = dict(supportAll=True)
        OneToOneBlock.__init__(self, GetNumPyRandom().permutation, dtype, dtype, dtypeArgs, dtypeArgs, callPostBuffer=True)

def Integers(dtype):
    outputArgs = dict(supportInt=True, supportUInt=True)
    return SingleOutputSource(GetNumPyRandomIntegersFunc(), dtype, outputArgs, useDType=False)

class Beta(SingleOutputSource):
    def __init__(self, dtype, alpha, beta):
        outputArgs = dict(supportFloat=True)
        SingleOutputSource.__init__(self, GetNumPyRandom().beta, dtype, outputArgs, useDType=False)

        self.__alpha = None
        self.__beta = None

        self.setAlpha(alpha)
        self.setBeta(beta)

    def getAlpha(self):
        return self.__alpha

    def setAlpha(self, alpha):
        Utility.validateParameter(alpha, self.numpyOutputDType)

        if alpha <= 0.0:
            raise ValueError("Alpha must be > 0.0")

        self.__alpha = alpha
        self.__updateArgs()

    def getBeta(self):
        return self.__beta

    def setBeta(self, beta):
        Utility.validateParameter(beta, self.numpyOutputDType)

        if beta <= 0.0:
            raise ValueError("Beta must be > 0.0")

        self.__beta = beta
        self.__updateArgs()

    def __updateArgs(self):
        self.funcArgs = [self.__alpha, self.__beta]

    def work(self):
        N = self.workInfo().minAllOutElements
        if 0 == N:
            return

        out = self.output(0).buffer()
        out[:N] = self.func(*self.funcArgs, size=(N)).astype(self.numpyOutputDType)
        self.output(0).produce(N)

class Binomial(SingleOutputSource):
    def __init__(self, dtype, N, P):
        outputArgs = dict(supportInt=True, supportUInt=True)
        SingleOutputSource.__init__(self, GetNumPyRandom().binomial, dtype, outputArgs, useDType=False)

        self.__N = None
        self.__P = None

        self.setN(N)
        self.setP(P)

    def getN(self):
        return self.__N

    def setN(self, N):
        Utility.validateParameter(N, self.numpyOutputDType)

        self.__N = N
        self.__updateArgs()

    def getP(self):
        return self.__P

    def setP(self, P):
        Utility.validateParameter(P, numpy.dtype("float32"))

        if (P < 0.0) or (P > 1.0):
            raise ValueError("P must be in the range [0.0, 1.0]")

        self.__P = P
        self.__updateArgs()

    def __updateArgs(self):
        self.funcArgs = [self.__N, self.__P]

    def work(self):
        N = self.workInfo().minAllOutElements
        if 0 == N:
            return

        out = self.output(0).buffer()
        out[:N] = self.func(*self.funcArgs, size=(N)).astype(self.numpyOutputDType)
        self.output(0).produce(N)

class ChiSquare(SingleOutputSource):
    def __init__(self, dtype, degreesOfFreedom):
        outputArgs = dict(supportFloat=True)
        SingleOutputSource.__init__(self, GetNumPyRandom().chisquare, dtype, outputArgs, useDType=False)

        self.__degreesOfFreedom = None

        self.setDegreesOfFreedom(degreesOfFreedom)

    def getDegreesOfFreedom(self):
        return self.__degreesOfFreedom

    def setDegreesOfFreedom(self, degreesOfFreedom):
        Utility.validateParameter(degreesOfFreedom, self.numpyOutputDType)

        if degreesOfFreedom <= 0.0:
            raise ValueError("degreesOfFreedom must be > 0.0")

        self.__degreesOfFreedom = degreesOfFreedom
        self.__updateArgs()

    def __updateArgs(self):
        self.funcArgs = [self.__degreesOfFreedom]

    def work(self):
        N = self.workInfo().minAllOutElements
        if 0 == N:
            return

        out = self.output(0).buffer()
        out[:N] = self.func(*self.funcArgs, size=(N)).astype(self.numpyOutputDType)
        self.output(0).produce(N)

class Exponential(SingleOutputSource):
    def __init__(self, dtype, scale):
        outputArgs = dict(supportFloat=True)
        SingleOutputSource.__init__(self, GetNumPyRandom().exponential, dtype, outputArgs, useDType=False)

        self.__scale = None

        self.setScale(scale)

    def getScale(self):
        return self.__scale

    def setScale(self, scale):
        Utility.validateParameter(scale, self.numpyOutputDType)

        if scale < 0.0:
            raise ValueError("scale must be >= 0.0")

        self.__scale = scale
        self.__updateArgs()

    def __updateArgs(self):
        self.funcArgs = [self.__scale]

    def work(self):
        N = self.workInfo().minAllOutElements
        if 0 == N:
            return

        out = self.output(0).buffer()
        out[:N] = self.func(*self.funcArgs, size=(N)).astype(self.numpyOutputDType)
        self.output(0).produce(N)

class F(SingleOutputSource):
    def __init__(self, dtype, numerator, denominator):
        outputArgs = dict(supportFloat=True)
        SingleOutputSource.__init__(self, GetNumPyRandom().f, dtype, outputArgs, useDType=False)

        self.__numerator = None
        self.__denominator = None

        self.setNumerator(numerator)
        self.setDenominator(denominator)

    def getNumerator(self):
        return self.__numerator

    def setNumerator(self, numerator):
        Utility.validateParameter(numerator, self.numpyOutputDType)

        if numerator < 0.0:
            raise ValueError("numerator must be >= 0.0")

        self.__numerator = numerator
        self.__updateArgs()

    def getDenominator(self):
        return self.__denominator

    def setDenominator(self, denominator):
        Utility.validateParameter(denominator, self.numpyOutputDType)

        if denominator <= 0.0:
            raise ValueError("denominator must be > 0.0")

        self.__denominator = denominator
        self.__updateArgs()

    def __updateArgs(self):
        self.funcArgs = [self.__numerator, self.__denominator]

    def work(self):
        N = self.workInfo().minAllOutElements
        if 0 == N:
            return

        out = self.output(0).buffer()
        out[:N] = self.func(*self.funcArgs, size=(N)).astype(self.numpyOutputDType)
        self.output(0).produce(N)

class RandomGamma(SingleOutputSource):
    def __init__(self, dtype, shape, scale):
        outputArgs = dict(supportFloat=True)
        SingleOutputSource.__init__(self, GetNumPyRandom().gamma, dtype, outputArgs, useDType=False)

        self.__shape = None
        self.__scale = None

        self.setShape(shape)
        self.setScale(scale)

    def getShape(self):
        return self.__shape

    def setShape(self, shape):
        Utility.validateParameter(shape, self.numpyOutputDType)

        if shape < 0.0:
            raise ValueError("shape must be >= 0.0")

        self.__shape = shape
        self.__updateArgs()

    def getScale(self):
        return self.__scale

    def setScale(self, scale):
        Utility.validateParameter(scale, self.numpyOutputDType)

        if scale < 0.0:
            raise ValueError("scale must be >= 0.0")

        self.__scale = scale
        self.__updateArgs()

    def __updateArgs(self):
        self.funcArgs = [self.__shape, self.__scale]

    def work(self):
        N = self.workInfo().minAllOutElements
        if 0 == N:
            return

        out = self.output(0).buffer()
        out[:N] = self.func(*self.funcArgs, size=(N)).astype(self.numpyOutputDType)
        self.output(0).produce(N)

class Geometric(SingleOutputSource):
    def __init__(self, dtype, P):
        outputArgs = dict(supportUInt=True, supportInt=True)
        SingleOutputSource.__init__(self, GetNumPyRandom().geometric, dtype, outputArgs, useDType=False)

        self.__P = None

        self.setP(P)

    def getP(self):
        return self.__P

    def setP(self, P):
        Utility.validateParameter(P, numpy.dtype("float32"))

        if (P < 0.0) or (P > 1.0):
            raise ValueError("P must be in range [0.0, 1.0]")

        self.__P = P
        self.__updateArgs()

    def __updateArgs(self):
        self.funcArgs = [self.__P]

    def work(self):
        N = self.workInfo().minAllOutElements
        if 0 == N:
            return

        out = self.output(0).buffer()
        out[:N] = self.func(*self.funcArgs, size=(N)).astype(self.numpyOutputDType)
        self.output(0).produce(N)

class Gumbel(SingleOutputSource):
    def __init__(self, dtype, location, scale):
        outputArgs = dict(supportFloat=True)
        SingleOutputSource.__init__(self, GetNumPyRandom().gumbel, dtype, outputArgs, useDType=False)

        self.__location = None
        self.__scale = None

        self.setLocation(location)
        self.setScale(scale)

    def getLocation(self):
        return self.__location

    def setLocation(self, location):
        Utility.validateParameter(location, self.numpyOutputDType)

        self.__location = location
        self.__updateArgs()

    def getScale(self):
        return self.__scale

    def setScale(self, scale):
        Utility.validateParameter(scale, self.numpyOutputDType)

        if scale < 0.0:
            raise ValueError("scale must be >= 0.0")

        self.__scale = scale
        self.__updateArgs()

    def __updateArgs(self):
        self.funcArgs = [self.__location, self.__scale]

    def work(self):
        N = self.workInfo().minAllOutElements
        if 0 == N:
            return

        out = self.output(0).buffer()
        out[:N] = self.func(*self.funcArgs, size=(N)).astype(self.numpyOutputDType)
        self.output(0).produce(N)

class Hypergeometric(SingleOutputSource):
    def __init__(self, dtype, numGood, numBad, numSampled):
        outputArgs = dict(supportUInt=True, supportInt=True)
        SingleOutputSource.__init__(self, GetNumPyRandom().hypergeometric, dtype, outputArgs, useDType=False)

        self.__numGood = None
        self.__numBad = None
        self.__numSampled = None

        self.setNumGood(numGood)
        self.setNumBad(numBad)
        self.setNumSampled(numSampled)

    def getNumGood(self):
        return self.__numGood

    def setNumGood(self, numGood):
        Utility.validateParameter(numGood, self.numpyOutputDType)

        if (numGood < 0) or (numGood > 10e9):
            raise ValueError("numGood must be in range [0, 10e9].")

        self.__numGood = numGood
        self.__updateArgs()

    def getNumBad(self):
        return self.__numBad

    def setNumBad(self, numBad):
        Utility.validateParameter(numBad, self.numpyOutputDType)

        if (numBad < 0) or (numBad > 10e9):
            raise ValueError("numBad must be in range [0, 10e9].")

        self.__numBad = numBad
        self.__updateArgs()

    def getNumSampled(self):
        return self.__numSampled

    def setNumSampled(self, numSampled):
        Utility.validateParameter(numSampled, self.numpyOutputDType)

        if (numSampled < 0) or (numSampled > (self.__numGood + self.__numBad)):
            raise ValueError("numSampled must be in range [0, numGood+numBad].")

        self.__numSampled = numSampled
        self.__updateArgs()

    def __updateArgs(self):
        # If numGood/numBad pushes numSampled out of range, pull it back.
        if None not in [self.__numGood, self.__numBad, self.__numSampled]:
            self.__numSampled = min(self.__numSampled, (self.__numGood+self.__numBad))
            self.funcArgs = [self.__numGood, self.__numBad, self.__numSampled]

    def work(self):
        N = self.workInfo().minAllOutElements
        if 0 == N:
            return

        out = self.output(0).buffer()
        out[:N] = self.func(*self.funcArgs, size=(N)).astype(self.numpyOutputDType)
        self.output(0).produce(N)
