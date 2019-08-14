# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *
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
    else:
        rand = numpy.random

    return rand

def GetNumPyRandomIntegersFunc():
    randints = None
    try:
        rand = numpy.random.default_rng().integers
    else:
        rand = numpy.random.random_integers

    return rand

#
# Blocks whose invocations ore implementations are different enough to not auto-generate
#

# TODO: postBuffer
class Permutation(OneToOneBlock):
    def __init__(self, dtype):
        dtypeArgs = dict(supportAll=True)
        OneToOneBlock.__init__(self, GetNumPyRandom().permutation, dtype, dtype, dtypeArgs, dtypeArgs)

def Integers(dtype):
    outputArgs = dict(supportInt=True)

    return SingleOutputSource(GetNumPyRandomIntegersFunc(), dtype, outputArgs, useDType=True)

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
        self.validateParameter(alpha, self.numpyOutputDType)

        if alpha < 0.0:
            raise ValueError("Alpha must be > 0.0")

        self.__alpha = alpha
        self.__updateArgs()

    def getBeta(self):
        return self.__beta

    def setBeta(self, beta):
        self.validateParameter(beta, self.numpyOutputDType)

        if beta < 0.0:
            raise ValueError("Beta must be > 0.0")

        self.__beta = beta
        self.__updateArgs()

    def __updateArgs(self):
        self.funcArgs = [self.__alpha, self.__beta]
