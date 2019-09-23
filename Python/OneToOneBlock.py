# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

import Pothos

import numpy

class OneToOneBlock(BaseBlock):
    def __init__(self, func, inputDType, outputDType, inputDTypeArgs, outputDTypeArgs, funcArgs, funcKWargs, *args, **kwargs):
        if inputDType is None:
            raise ValueError("For non-source blocks, inputDType cannot be None.")
        if outputDType is None:
            raise ValueError("For non-sink blocks, outputDType cannot be None.")

        BaseBlock.__init__(self, func, inputDType, outputDType, inputDTypeArgs, outputDTypeArgs, funcArgs, funcKWargs, *args, **kwargs)

        self.setupInput(0, self.inputDType)
        self.setupOutput(0, self.outputDType)

    def work(self):
        assert(self.numpyInputDType is not None)
        assert(self.numpyOutputDType is not None)

        if self.callPostBuffer:
            self.workWithPostBuffer()
        else:
            self.workWithGivenOutputBuffer()

    def workWithPostBuffer(self):
        elems = self.workInfo().minAllInElements
        if 0 == elems:
            return

        in0 = self.input(0).buffer()
        out = self.func(in0, *self.funcArgs, **self.funcKWargs).astype(self.numpyOutputDType, copy=False)

        self.input(0).consume(elems)
        self.output(0).postBuffer(out)

    def workWithGivenOutputBuffer(self):
        elems = self.workInfo().minAllInElements
        if 0 == elems:
            return

        in0 = self.input(0).buffer()
        out0 = self.output(0).buffer()
        N = min(len(in0), len(out0))

        out0[:N] = self.func(in0, *self.funcArgs, **self.funcKWargs).astype(self.numpyOutputDType, copy=False)

        self.input(0).consume(elems)
        self.output(0).produce(elems)

#
# Blocks too different to be auto-generated
#

class Roll(OneToOneBlock):
    def __init__(self, dtype, roll):
        dtypeArgs = dict(supportAll=True)
        OneToOneBlock.__init__(self, numpy.roll, dtype, dtype, dtypeArgs, dtypeArgs)
        self.setRoll(roll)

    def getShift(self):
        return self.__shift

    def setShift(self, shift):
        if type(shift) != int:
            raise TypeError("The value of \"shift\" must be an int.")

        self.__shift = shift
        self.__refreshArgs()

    def __refreshArgs(self):
        self.funcArgs = tuple(self.__roll)

    # TODO: remove when funcKWargs is added
    def work(self):
        elems = self.workInfo().minAllInElements
        if 0 == elems:
            return

        in0 = self.input(0).buffer()
        out = self.func(in0, self.__shift)

        self.input(0).consume(elems)
        self.output(0).postBuffer(out)

class Partition(OneToOneBlock):
    def __init__(self, dtype, index):
        dtypeArgs = dict(supportAll=True)
        OneToOneBlock.__init__(self, numpy.partition, dtype, dtype, dtypeArgs, dtypeArgs)
        self.setIndex(index)

    def getIndex(self):
        return self.__index

    def setIndex(self, index):
        if type(index) != int:
            raise TypeError("The value of \"index\" must be an int.")

        if index < 0:
            raise ValueError("index must be >= 0.")

        self.__index = index
        self.__refreshArgs()

    def __refreshArgs(self):
        self.funcArgs = tuple(self.__index)

    def work(self):
        elems = self.workInfo().minAllInElements

        # Do nothing unless we have enough values for our specified
        # index.
        if elems <= self.__index:
            return

        in0 = self.input(0).buffer()
        out = self.func(in0, self.__index)

        self.input(0).consume(elems)
        self.output(0).postBuffer(out)
