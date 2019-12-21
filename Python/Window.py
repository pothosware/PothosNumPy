# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .OneToOneBlock import *

import Pothos

import numpy

WindowFuncDict = dict(
    BARTLETT=numpy.bartlett,
    BLACKMAN=numpy.blackman,
    HAMMING=numpy.hamming,
    HANNING=numpy.hanning,
    KAISER=numpy.kaiser
)

class WindowBlock(OneToOneBlock):
    def __init__(self, dtype, windowType):
        dtypeArgs = dict(supportFloat=True, supportComplex=True)
        OneToOneBlock.__init__(self, "/numpy/window", None, dtype, dtype, dtypeArgs, dtypeArgs, list(), dict(), dict())

        self.registerProbe("windowType", "windowTypeChanged", "setWindowType")
        self.registerProbe("kaiserBeta", "kaiserBetaChanged", "setKaiserBeta")

        self.setKaiserBeta(0.0)
        self.setWindowType(windowType)

    def getWindowType(self):
        return self.__windowType

    def setWindowType(self, windowType):
        if windowType not in WindowFuncDict:
            raise ValueError("Invalid window type: {0}".format(windowType))

        self.__windowType = windowType
        self.func = WindowFuncDict[windowType]

        if windowType == "KAISER":
            self.funcArgs = [self.__kaiserBeta]
        else:
            self.funcArgs = []

        # C++ equivalent: emitSignal("windowTypeChanged", windowType)
        self.windowTypeChanged(windowType)

    def getKaiserBeta(self):
        return self.__kaiserBeta

    def setKaiserBeta(self, kaiserBeta):
        self.__kaiserBeta = kaiserBeta
        if self.__windowType == "KAISER":
            self.funcArgs = [self.__kaiserBeta]

        # C++ equivalent: emitSignal("kaiserBetaChanged", beta)
        self.kaiserBetaChanged(kaiserBeta)

    def work(self):
        if len(self.input(0).buffer()) == 0:
            return

        in0 = self.input(0).takeBuffer()
        self.input(0).consume(len(in0))
        window = self.func(len(in0), *self.funcArgs).astype(self.numpyOutputDType)

        in0 = in0 * window
        self.output(0).postBuffer(in0)

# Factory
def Window(dtype, windowType):
    return WindowBlock(dtype, windowType)
