# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

import Pothos

import numpy

# TODO: support functions that take in two inputs instead of a single 2D
# input
class NToOneBlock(Pothos.Block):
    def __init__(self, dtype, nchans, func, *args):
        Utility.validateDType(dtype)

        Pothos.Block.__init__(self)
        self.setupOutput(0, dtype)

        self.dtype = dtype
        self.numpyDType = Pothos.Buffer.dtype_to_numpy(self.dtype)
        self.nchans = 0
        self.func = func
        self.args = args

        self.setNumChannels(nchans)

    def getNumChannels(self):
        return self.nchans

    def setNumChannels(self, nchans):
        if nchans <= 0:
            raise ValueError("Number of channels must be positive.")

        oldNChans = self.nchans
        for i in range(oldNChans, nchans):
            self.setupInput(i, self.dtype)

        self.nchans = nchans

    def work(self):
        elems = self.workInfo().minAllElements
        if 0 == elems:
            return

        N = elems * self.dtype.dimension()
        allArrs = numpy.array([arr[:N].view() for arr in self.inputs()], dtype=self.numpyDType)
        self.output(0).buffer()[:N] = self.func(allArrs, axis=0, dtype=self.numpyDType)

        for port in self.inputs():
            port.consume(elems)

#
# Factories exposed to C++ layer
#

def Prod(self, dtype, nchans):
    return NToOneBlock(dtype, nchans, numpy.prod)

def Sum(self, dtype, nchans):
    return NToOneBlock(dtype, nchans, numpy.sum)

def NanProd(self, dtype, nchans):
    return NToOneBlock(dtype, nchans, numpy.nanprod)

def NanSum(self, dtype, nchans):
    return NToOneBlock(dtype, nchans, numpy.nansum)

def Diff(self, dtype, nchans):
    return NToOneBlock(dtype, nchans, numpy.diff)
