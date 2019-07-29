# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

import Pothos

import numpy

class NToOneBlock(Pothos.Block):
    def __init__(self, dtype, func, *args):
        Pothos.Block.__init__(self)
        self.setupInput(0, dtype)
        self.setupInput(1, dtype)
        self.setupOutput(0, dtype)

        self.func = func
        self.args = args

    def work(self):
        elems = self.workInfo().minAllElements
        if 0 == elems:
            return

        in0 = self.input(0).buffer()
        in1 = self.input(1).buffer()
        out0 = self.output(0).buffer()
        numpyDType = self.input(0).dtype()

        N = min(len(in0), len(in1), len(out0))

        if (self.args is None) or (len(self.args) == 0):
            out0[:N] = self.func(in0[:N], in1[:N], dtype=numpyDType)
        else:
            out0[:N] = self.func(in0[:N], in1[:N], *self.args, dtype=numpyDType)

        self.input(0).consume(elems)
        self.input(1).consume(elems)
        self.output(0).produce(elems)

#
# Factories exposed to C++ layer
#
