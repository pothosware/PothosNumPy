# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

import Pothos

import scipy.signal

class SplineBlock(Pothos.Block):
    def __init__(self, dtype, order, func):
        Pothos.Block.__init__(self)
        self.setupInput("0", dtype)
        self.setupOutput("0", dtype)

        self.__func = func
        self.setOrder(order)

    def getOrder(self):
        return self.__order

    def setOrder(self, order):
        if order <= 0:
            raise IndexError("Spline orders must be positive.")

        self.__order = order

    def work(self):
        in0 = self.input(0).buffer()
        out0 = self.output(0).buffer()
        n = min(len(in0), len(out0))

        out0[:n] = self.__func(in0[:n], self.__order)

        self.input(0).consume(n)
        self.output(0).produce(n)

#
# Factories exposed to C++ layer
#

def SciPySignalBSpline(dtype, order):
    return SplineBlock(dtype, order, scipy.signal.bspline)

def SciPySignalGaussSpline(dtype, order):
    return SplineBlock(dtype, order, scipy.signal.gauss_spline)
