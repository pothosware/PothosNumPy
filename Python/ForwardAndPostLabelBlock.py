# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

import Pothos

import numpy

class ForwardAndPostLabelBlock(BaseBlock):
    def __init__(self, func, inputDType, outputDType, inputDTypeArgs, outputDTypeArgs, findIndexFunc, labelName, funcArgs, funcKWargs, *args, **kwargs):
        if inputDType is None:
            raise ValueError("For non-source blocks, inputDType cannot be None.")
        if outputDType is None:
            raise ValueError("For non-sink blocks, outputDType cannot be None.")

        BaseBlock.__init__(self, func, inputDType, outputDType, inputDTypeArgs, outputDTypeArgs, funcArgs, funcKWargs, *args, **kwargs)

        self.setupInput(0, self.inputDType)

        # Unique domain because of buffer forwarding
        self.setupOutput(0, self.outputDType, self.uid())

        self.findIndexFunc = findIndexFunc
        self.labelName = labelName

    def work(self):
        assert(self.numpyInputDType is not None)
        assert(self.numpyOutputDType is not None)

        elems = self.input(0).elements()
        if 0 == elems:
            return

        buf = self.input(0).takeBuffer()
        numpyRet = None

        if self.useDType:
            numpyRet = self.func(buf, *self.funcArgs, dtype=self.numpyInputDType)
        else:
            numpyRet = self.func(buf, *self.funcArgs)

        self.processAndPostBuffer(numpyRet, buf)

    def processAndPostBuffer(self, numpyRet, buf):
        if self.findIndexFunc:
            index = self.findIndexFunc(buf)
        else:
            index = 0

        self.input(0).consume(len(buf))

        self.output(0).postLabel(Pothos.Label(self.labelName, numpyRet, index))
        self.output(0).postBuffer(buf)

#
# Subclasses
#

# Needed because the array index associated with the label depends on the return
# value as well as the buffer
class Median(ForwardAndPostLabelBlock):
    def __init__(self, dtype, ignoreNaN):
        medianFunc = numpy.nanmedian if ignoreNaN else numpy.median
        dtypeArgs = dict(supportAll=True)
        kwargs = dict(useDType=False)
        ForwardAndPostLabelBlock.__init__(self, medianFunc, dtype, dtype, dtypeArgs, dtypeArgs, None, "MEDIAN", list(), dict(), **kwargs)

    def processAndPostBuffer(self, numpyRet, buf):
        # numpy.where returns a tuple of ndarrays
        arrIndex = numpy.where(buf == numpyRet)[0][0]

        self.input(0).consume(len(buf))

        self.output(0).postLabel(Pothos.Label(self.labelName, numpyRet, arrIndex))
        self.output(0).postBuffer(buf)
