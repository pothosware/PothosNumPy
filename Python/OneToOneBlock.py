# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

import Pothos

import numpy

class OneToOneBlock(BaseBlock):
    def __init__(self, blockPath, func, inputDType, outputDType, inputDTypeArgs, outputDTypeArgs, funcArgs, funcKWargs, *args, **kwargs):
        if inputDType is None:
            raise ValueError("For non-source blocks, inputDType cannot be None.")
        if outputDType is None:
            raise ValueError("For non-sink blocks, outputDType cannot be None.")

        BaseBlock.__init__(self, blockPath, func, inputDType, outputDType, inputDTypeArgs, outputDTypeArgs, funcArgs, funcKWargs, *args, **kwargs)

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

        if (out is not None) and (len(out) > 0):
            self.input(0).consume(elems)
            self.output(0).postBuffer(out)

    def workWithGivenOutputBuffer(self):
        elems = self.workInfo().minAllInElements
        if 0 == elems:
            return

        in0 = self.input(0).buffer()
        out0 = self.output(0).buffer()
        N = min(len(in0), len(out0))
        out = None

        out = self.func(in0, *self.funcArgs, **self.funcKWargs).astype(self.numpyOutputDType, copy=False)

        if (out is not None) and (len(out) > 0):
            out0[:N] = out
            self.input(0).consume(elems)
            self.output(0).produce(elems)
