# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from . import Utility

import Pothos

import numpy

class SimpleSource(Pothos.Block):
    def __init__(self, dtype, func, *args):
        Utility.validateDType(dtype)

        Pothos.Block.__init__(self)
        self.setupOutput("0", dtype)

        self.func = func
        self.numpyDType = Pothos.Buffer.dtype_to_numpy(dtype)

        self.args = args

    def work(self):
        out0 = self.output(0).buffer()
        N = len(out0)

        if N == 0:
            return

        if self.args is None:
            out0[:] = self.func(N, dtype=self.numpyDType)
        else:
            out0[:] = self.func(N, *self.args, dtype=self.numpyDType)

        self.output(0).produce(N)

class PostBufferSource(Pothos.Block):
    def __init__(self, dtype, func, *args):
        Utility.validateDType(dtype)

        Pothos.Block.__init__(self)
        self.setupOutput("0", dtype)

        self.func = func
        self.numpyDType = Pothos.Buffer.dtype_to_numpy(dtype)

        self.args = args

    def work(self):
        N = len(self.output(0).buffer())

        # Even though we're posting the buffer, N will be 0 in a situation
        # where we shouldn't have output, so go along with that.
        if N == 0:
            return

        out = None

        if self.args is None:
            out = self.func(N, dtype=self.numpyDType)
        else:
            out = self.func(N, *self.args, dtype=self.numpyDType)

        self.postBuffer(out)

class FullClass(SimpleSource):
    def __init__(self, dtype, fillValue):
        SimpleSource.__init__(self, dtype, numpy.full, fillValue)
        self.setFillValue(fillValue)

    def getFillValue(self):
        return self.fillValue

    def setFillValue(self, fillValue):
        Utility.validateParameter(fillValue, self.numpyDType)

        self.fillValue = fillValue
        self.args = [fillValue]

class Range(SimpleSource):
    def __init__(self, dtype, func, start, stop, step):
        SimpleSource.__init__(self, dtype, func, start, stop, step)

        self.setStart(start)
        self.setStop(stop)
        self.setStep(step)

    def __refreshArgs(self):
        self.args = (self.start, self.stop, self.step)

    def getStart(self):
        return self.start

    def setStart(self, start):
        Utility.validateParameter(start, self.numpyDType)

        self.start = start
        self.__refreshArgs()

    def getStop(self):
        return self.stop

    def setStop(self, stop):
        Utility.validateParameter(stop, self.numpyDType)

        self.stop = stop
        self.__refreshArgs()

    def getStep(self):
        return self.step

    def setStep(self, step):
        Utility.validateParameter(step, self.numpyDType)

        self.step = step
        self.__refreshArgs()

class Space(SimpleSource):
    def __init__(self, dtype, func, start, stop, numValues):
        SimpleSource.__init__(self, dtype, func, start, stop, numValues)

        self.setStart(start)
        self.setStop(stop)
        self.setNumValues(numValues)

    def __refreshArgs(self):
        self.args = (self.start, self.stop, self.numValues)

    def getStart(self):
        return self.start

    def setStart(self, start):
        Utility.validateParameter(start, self.numpyDType)

        self.start = start
        self.__refreshArgs()

    def getStop(self):
        return self.stop

    def setStop(self, stop):
        Utility.validateParameter(stop, self.numpyDType)

        self.stop = stop
        self.__refreshArgs()

    def getNumValues(self):
        return self.numValues

    def setNumValues(self, numValues):
        Utility.validateParameter(numValues, self.numpyDType)

        self.numValues = numValues
        self.__refreshArgs()

#
# Factories exposed to C++ layer
#

def Ones(dtype):
    return SimpleSource(dtype, numpy.ones)

def Zeros(dtype):
    return SimpleSource(dtype, numpy.zeros)

# TODO: enforce that fillValue is valid for the given dtype
def Full(dtype, fillValue):
    return FullClass(dtype, fillValue)

def ARange(dtype, start, stop, step):
    return Range(dtype, numpy.arange, start, stop, step)

def LinSpace(dtype, start, stop, numValues):
    return Space(dtype, numpy.linspace, start, stop, numValues)

def LogSpace(dtype, start, stop, numValues):
    return Space(dtype, numpy.logspace, start, stop, numValues)

def GeomSpace(dtype, start, stop, numValues):
    return Space(dtype, numpy.geomspace, start, stop, numValues)
