# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

import Pothos

import numpy

# TODO: simple classes like this can likely be auto-generated
class NumPySqrt(Pothos.Block):
    def __init__(self, dtype):
        Pothos.Block.__init__(self)
        self.setupInput("0", dtype)
        self.setupOutput("0", dtype)

    def work(self):
        in0 = self.input(0).buffer()
        out0 = self.output(0).buffer()

        n = min(len(in0), len(out0))
        numpy.sqrt(in0[:n], out0[:n])

        self.input(0).consume(n)
        self.output(0).produce(n)
