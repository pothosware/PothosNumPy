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

"""
/*
 * |PothosDoc Window
 *
 * Corresponding NumPy functions:
 * <ol>
 * <li>numpy.bartlett</li>
 * <li>numpy.blackman</li>
 * <li>numpy.hamming</li>
 * <li>numpy.hanning</li>
 * <li>numpy.kaiser</li>
 * </ol>
 *
 * |category /NumPy/Filter
 * |keywords numpy filter window bartlett blackman hamming hanning kaiser
 * |factory /numpy/window(dtype,windowType)
 *
 * |param dtype(Data Type) The block data type.
 * |widget DTypeChooser(float=1,cfloat=1)
 * |default "float64"
 * |preview disable
 *
 * |param windowType(Window Type)
 * |widget ComboBox(editable=False)
 * |default "BARTLETT"
 * |option [Bartlett] "BARTLETT"
 * |option [Blackman] "BLACKMAN"
 * |option [Hamming] "HAMMING"
 * |option [Hanning] "HANNING"
 * |option [Kaiser] "KAISER"
 * |preview enable
 *
 * |param beta(Beta)
 * |widget DoubleSpinBox()
 * |default 0.0
 * |preview when(enum=windowType, "KAISER")
 *
 * |setter setWindowType(windowType)
 * |setter setKaiserBeta(beta)
 */
"""
def Window(dtype, windowType):
    return WindowBlock(dtype, windowType)
