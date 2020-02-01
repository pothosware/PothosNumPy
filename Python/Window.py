# Copyright (c) 2019-2020 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .Source import *

import Pothos

import numpy

WindowFuncDict = dict(
    BARTLETT=numpy.bartlett,
    BLACKMAN=numpy.blackman,
    HAMMING=numpy.hamming,
    HANNING=numpy.hanning,
    KAISER=numpy.kaiser
)

class WindowBlock(SingleOutputSource):
    def __init__(self, dtype, windowType):
        dtypeArgs = dict(supportFloat=True, supportComplex=True)
        kwargs = dict(useDType=False)
        SingleOutputSource.__init__(self, "/numpy/window", None, dtype, dtypeArgs, list(), dict(), list(), **kwargs)

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

"""
/*
 * |PothosDoc Window
 *
 * Corresponding NumPy functions:
 * <ol>
 * <li><b>numpy.bartlett</b></li>
 * <li><b>numpy.blackman</b></li>
 * <li><b>numpy.hamming</b></li>
 * <li><b>numpy.hanning</b></li>
 * <li><b>numpy.kaiser</b></li>
 * </ol>
 *
 * |category /NumPy/Filter
 * |keywords numpy filter window bartlett blackman hamming hanning kaiser
 * |factory /numpy/window(dtype,windowType)
 *
 * |param dtype[Data Type] The block data type.
 * |widget DTypeChooser(float=1,cfloat=1)
 * |default "float64"
 * |preview disable
 *
 * |param windowType[Window Type]
 * |widget ComboBox(editable=False)
 * |default "BARTLETT"
 * |option [Bartlett] "BARTLETT"
 * |option [Blackman] "BLACKMAN"
 * |option [Hamming] "HAMMING"
 * |option [Hanning] "HANNING"
 * |option [Kaiser] "KAISER"
 * |preview enable
 *
 * |param beta[Beta]
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
