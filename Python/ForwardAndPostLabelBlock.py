# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

import Pothos

import numpy

class ForwardAndPostLabelBlock(BaseBlock):
    def __init__(self, func, findIndexFunc, labelName, inputDType, outputDType, inputArgs, outputArgs, *funcArgs, **kwargs):
        if inputDType is None:
            raise ValueError("For non-source blocks, inputDType cannot be None.")
        if outputDType is None:
            raise ValueError("For non-sink blocks, outputDType cannot be None.")

        BaseBlock.__init__(self, func, inputDType, outputDType, inputArgs, outputArgs, *funcArgs, **kwargs)

        self.setupInput(0, self.inputDType)

        # Unique domain because of buffer forwarding
        self.setupOutput(0, self.outputDType, self.uid())

        self.findIndexFunc = findIndexFunc
        self.labelName = labelName

    def work(self):
        assert(self.numpyInputDType is not None)
        assert(self.numpyOutputDType is not None)

        elems = len(self.input(0).buffer)
        if 0 == elems:
            return

        buf = self.takeBuffer(self.input(0))
        numpyRet = None

        if self.useDType:
            numpyRet = self.func(buf, *self.funcArgs, dtype=self.numpyInputDType)
        else:
            numpyRet = self.func(buf, *self.funcArgs)

        self.processAndPostBuffer(numpyRet, buf)

        self.input(0).consume(elems)
        self.output(0).postBuffer(buf)

    def processAndPostBuffer(self, numpyRet, buf):
        self.output(0).postLabel(self.labelName, numpyRet, self.findIndexFunc(buf))

#
# Utility
#

# Use when there is no set index associated with a given operation
def __returnZero(buf):
    return 0

#
# Subclasses
#

# Needed because of "initial" function parameter
class AxisMinMaxBlock(ForwardAndPostLabelBlock):
    def __init__(self, dtype, func, findIndexFunc, labelName):
        dtypeArgs = dict(supportAll=True)
        ForwardAndPostLabelBlock.__init__(self, func, findIndexFunc, labelName, dtype, dtype, dtypeArgs, dtypeArgs)

        # TODO: make "initial" settable on the fly, validated
        self.funcArgs = None

# Needed because the array index associated with the label depends on the return
# value as well as the buffer
class MedianClass(ForwardAndPostLabelBlock):
    def __init__(self, dtype, medianFunc):
        dtypeArgs = dict(supportAll=True)
        ForwardAndPostLabelBlock.__init__(self, medianFunc, __returnZero, "MEDIAN", dtype, dtype, dtypeArgs, dtypeArgs)

    def processAndPostBuffer(self, numpyRet, buf):
        # numpy.where returns a tuple of ndarrays
        arrIndex = numpy.where(buf == numpyRet)[0][0]

        self.output(0).postLabel(self.labelName, numpyRet, arrIndex)

#
# Factories exposed to C++
#

def AMax(dtype):
    return AxisMinMaxBlock(dtype, numpy.amax, numpy.argmax, "MAX")

def AMin(dtype):
    return AxisMinMaxBlock(dtype, numpy.amin, numpy.argmin, "MIN")

def Median(dtype):
    dtypeArgs = dict(supportAll=True)
    return MedianClass(dtype, numpy.median)

def NaNMedian(dtype):
    dtypeArgs = dict(supportAll=True)
    return MedianClass(dtype, numpy.nanmedian)
