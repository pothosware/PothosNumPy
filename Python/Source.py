# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *
from . import Utility

import Pothos

import numpy

class SingleOutputSource(BaseBlock):
    def __init__(self, func, dtype, dtypeArgs, *funcArgs, **kwargs):
        if dtype is None:
            raise ValueError("Null dtype")

        BaseBlock.__init__(self, func, None, dtype, None, dtypeArgs, *funcArgs, **kwargs)
        self.setupOutput(0, self.outputDType)

    def work(self):
        assert(self.numpyOutputDType is not None)

        if self.callPostBuffer:
            self.workWithPostBuffer()
        else:
            self.workWithGivenOutputBuffer()

    def workWithPostBuffer(self):
        elems = len(self.output(0).buffer())

        # Even if we're posting the buffer, we're in a situation where we
        # shouldn't be outputting anything, so go with it.
        if elems == 0:
            return

        if self.useDType:
            out = self.func(elems, *self.funcArgs, dtype=self.numpyInputDType)
        else:
            out = self.func(elems, *self.funcArgs).astype(self.numpyOutputDType)

        self.postBuffer(out)

    def workWithGivenOutputBuffer(self):
        elems = len(self.output(0).buffer())
        if 0 == elems:
            return

        out0 = self.output(0).buffer()

        if self.useDType:
            out0[:] = self.func(elems, *self.funcArgs, dtype=self.numpyInputDType)
        else:
            out0[:] = self.func(elems, *self.funcArgs)

        self.output(0).produce(elems)

class FullClass(SingleOutputSource):
    def __init__(self, dtype, fillValue):
        dtypeArgs = dict(supportAll=True)
        SingleOutputSource.__init__(self, numpy.full, dtype, dtypeArgs, fillValue)
        self.setFillValue(fillValue)

    def getFillValue(self):
        return self.fillValue

    def setFillValue(self, fillValue):
        Utility.validateParameter(fillValue, self.numpyOutputDType)

        self.fillValue = fillValue
        self.funcArgs = [fillValue]

#
# Factories exposed to C++ layer
#

def Full(dtype, fillValue):
    return FullClass(dtype, fillValue)

"""
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

def ARange(dtype, start, stop, step):
    return Range(dtype, numpy.arange, start, stop, step)

def LinSpace(dtype, start, stop, numValues):
    return Space(dtype, numpy.linspace, start, stop, numValues)

def LogSpace(dtype, start, stop, numValues):
    return Space(dtype, numpy.logspace, start, stop, numValues)

def GeomSpace(dtype, start, stop, numValues):
    return Space(dtype, numpy.geomspace, start, stop, numValues)
"""
