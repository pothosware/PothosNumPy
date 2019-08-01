# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

import Pothos

import numpy

class TwoToOneBlock(Pothos.Block):
    def __init__(self, dtype, func, *args, useDType=True, callPostBuffer=False):
        Utility.validateDType(dtype)

        Pothos.Block.__init__(self)
        self.setupInput(0, dtype)
        self.setupInput(1, dtype)
        self.setupOutput(0, dtype)

        self.func = func
        self.args = args
        self.useDType = useDType
        self.callPostBuffer = callPostBuffer

    def work(self):
        if self.callPostBuffer:
            self.workWithPostBuffer()
        else:
            self.workWithGivenOutputBuffer()

    def workWithPostBuffer(self):
        elems = self.workInfo().minAllInElements
        if 0 == elems:
            return

        in0 = self.input(0).buffer()
        in1 = self.input(1).buffer()
        out = None
        numpyDType = self.input(0).dtype()

        if self.useDType:
            out = self.func(in0[:N], in1[:N], *self.args, dtype=numpyDType)
        else:
            out = self.func(in0[:N], in1[:N], *self.args)

        self.input(0).consume(elems)
        self.input(1).consume(elems)
        self.output(0).postBuffer(out)

    def workWithGivenOutputBuffer(self):
        elems = self.workInfo().minAllElements
        if 0 == elems:
            return

        in0 = self.input(0).buffer()
        in1 = self.input(1).buffer()
        out0 = self.output(0).buffer()
        numpyDType = self.input(0).dtype()

        N = min(len(in0), len(in1), len(out0))

        if self.useDType:
            out0[:N] = self.func(in0[:N], in1[:N], *self.args, dtype=numpyDType)
        else:
            out0[:N] = self.func(in0[:N], in1[:N], *self.args)

        self.input(0).consume(elems)
        self.input(1).consume(elems)
        self.output(0).produce(elems)

#
# Factories exposed to C++ layer
#

def BitwiseAnd(dtype):
    return TwoToOneBlock(dtype, numpy.bitwise_and)

def BitwiseOr(dtype):
    return TwoToOneBlock(dtype, numpy.bitwise_or)

def BitwiseXor(dtype):
    return TwoToOneBlock(dtype, numpy.bitwise_xor)

def Heaviside(dtype):
    return TwoToOneBlock(dtype, numpy.heaviside)
