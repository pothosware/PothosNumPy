# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

import Pothos

import numpy

class OneToOneBlock(BaseBlock):
    def __init__(self, func, inputDType, outputDType, inputArgs, outputArgs, *funcArgs, **kwargs):
        BaseBlock.__init__(self, func, inputDType, outputDType, inputArgs, outputArgs, *funcArgs, **kwargs)

        self.setupInput(0, self.inputDType)
        self.setupOutput(0, self.outputDType)

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
        out = None

        if self.useDType:
            out = self.func(in0, *self.funcArgs, dtype=self.numpyInputDType)
        else:
            out = self.func(in0, *self.funcArgs).astype(self.numpyOutputDType)

        self.input(0).consume(elems)
        self.output(0).postBuffer(out)

    def workWithGivenOutputBuffer(self):
        elems = self.workInfo().minAllElements
        if 0 == elems:
            return

        in0 = self.input(0).buffer()
        out0 = self.output(0).buffer()
        N = min(len(in0), len(out0))

        if self.useDType:
            out0[:N] = self.func(in0[:N], *self.funcArgs, dtype=self.numpyInputDType)
        else:
            out0[:N] = self.func(in0[:N], *self.funcArgs)

        self.input(0).consume(elems)
        self.output(0).produce(elems)
