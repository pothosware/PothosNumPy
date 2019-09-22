# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *
from . import Utility

import Pothos

import numpy

class SingleOutputSource(BaseBlock):
    def __init__(self, func, dtype, dtypeArgs, funcArgs, funcKWargs, *args, **kwargs):
        if dtype is None:
            raise ValueError("Null dtype")

        BaseBlock.__init__(self, func, None, dtype, None, dtypeArgs, funcArgs, funcKWargs, *args, **kwargs)
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

        self.output(0).postBuffer(out)

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

class RangeSource(SingleOutputSource):
    def __init__(self, func, dtype, start, stop, step):
        dtypeArgs = dict(supportInt=True, supportUInt=True, supportFloat=True)
        SingleOutputSource.__init__(self, func, dtype, dtypeArgs, callPostBuffer=True)

        self.setStart(start)
        self.setStop(stop)
        self.setStep(step)

    # These functions do not take in an element count, as the output size is
    # determined by the parameters.
    def workWithPostBuffer(self):
        assert(self.numpyOutputDType is not None)
        elems = len(self.output(0).buffer())

        # Even if we're posting the buffer, we're in a situation where we
        # shouldn't be outputting anything, so go with it.
        if elems == 0:
            return

        if self.useDType:
            out = self.func(*self.funcArgs, dtype=self.numpyInputDType).astype(self.numpyOutputDType)
        else:
            out = self.func(*self.funcArgs).astype(self.numpyOutputDType)

        self.output(0).postBuffer(out)

    def __refreshArgs(self):
        self.funcArgs = (self.start, self.stop, self.step)

    def getStart(self):
        return self.start

    def setStart(self, start):
        Utility.validateParameter(start, self.numpyOutputDType)

        self.start = start
        self.__refreshArgs()

    def getStop(self):
        return self.stop

    def setStop(self, stop):
        Utility.validateParameter(stop, self.numpyOutputDType)

        self.stop = stop
        self.__refreshArgs()

    def getStep(self):
        return self.step

    def setStep(self, step):
        Utility.validateParameter(step, self.numpyOutputDType)

        self.step = step
        self.__refreshArgs()

class SpaceSource(SingleOutputSource):
    def __init__(self, func, dtype, start, stop, numValues):
        dtypeArgs = dict(supportInt=True, supportUInt=True, supportFloat=True)
        SingleOutputSource.__init__(self, func, dtype, dtypeArgs, callPostBuffer=True)

        self.setStart(start)
        self.setStop(stop)
        self.setNumValues(numValues)

    # These functions do not take in an element count, as the output size is
    # determined by the parameters.
    def workWithPostBuffer(self):
        assert(self.numpyOutputDType is not None)
        elems = len(self.output(0).buffer())

        # Even if we're posting the buffer, we're in a situation where we
        # shouldn't be outputting anything, so go with it.
        if elems == 0:
            return

        if self.useDType:
            out = self.func(*self.funcArgs, dtype=self.numpyInputDType).astype(self.numpyOutputDType)
        else:
            out = self.func(*self.funcArgs).astype(self.numpyOutputDType)

        self.output(0).postBuffer(out)

    def __refreshArgs(self):
        self.funcArgs = (self.start, self.stop, self.numValues)

    def getStart(self):
        return self.start

    def setStart(self, start):
        Utility.validateParameter(start, self.numpyOutputDType)

        self.start = start
        self.__refreshArgs()

    def getStop(self):
        return self.stop

    def setStop(self, stop):
        Utility.validateParameter(stop, self.numpyOutputDType)

        self.stop = stop
        self.__refreshArgs()

    def getNumValues(self):
        return self.numValues

    def setNumValues(self, numValues):
        Utility.validateParameter(numValues, self.numpyOutputDType)

        self.numValues = numValues
        self.__refreshArgs()

#
# Factories exposed to C++ layer
#

def Full(dtype, fillValue):
    return FullClass(dtype, fillValue)

def ARange(dtype, start, stop, step):
    return RangeSource(numpy.arange, dtype, start, stop, step)

def LinSpace(dtype, start, stop, numValues):
    return SpaceSource(numpy.linspace, dtype, start, stop, numValues)

def LogSpace(dtype, start, stop, numValues):
    return SpaceSource(numpy.logspace, dtype, start, stop, numValues)

def GeomSpace(dtype, start, stop, numValues):
    return SpaceSource(numpy.geomspace, dtype, start, stop, numValues)
