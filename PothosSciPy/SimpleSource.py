# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

import Pothos

import numpy

class SimpleSource(Pothos.Block):
    def __init__(self, dtype, func, *args):
        Pothos.Block.__init__(self)
        self.setupOutput("0", dtype)

        self.func = func
        self.numpyDType = Pothos.Buffer.dtype_to_numpy(dtype)

        # Store as a list so setters can access the arguments. This
        # can still be passed in as parameters with *self.args as
        # with a tuple.
        self.args = list(args)

    def work(self):
        out0 = self.output(0).buffer()
        N = len(out0)

        if N == 0:
            return

        if self.args is None:
            out0[:] = self.func(N, dtype=self.numpyDType)
        else:
            out0[:] = self.func(N, *self.args, dtype=self.numpyDType)

        self.output(0).produce(N)

class Full(SimpleSource):
    def __init__(self, dtype, fillValue):
        SimpleSource.__init__(self, dtype, numpy.full, fillValue)
        self.setFillValue(fillValue)

    def getFillValue(self):
        return self.fillValue

    def setFillValue(self, fillValue):
        self.fillValue = fillValue
        self.args[0] = fillValue

#
# Factories exposed to C++ layer
#

def NumPyOnes(dtype):
    return SimpleSource(dtype, numpy.ones)

def NumPyZeros(dtype):
    return SimpleSource(dtype, numpy.zeros)

# TODO: enforce that fillValue is valid for the given dtype
def NumPyFull(dtype, fillValue):
    return Full(dtype, fillValue)
