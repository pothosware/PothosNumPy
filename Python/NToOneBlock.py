# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

import Pothos

import numpy

class NToOneBlock(BaseBlock):
    def __init__(self, inputDType, outputDType, func, *funcArgs, **kwargs):
        BaseBlock.__init__(self, inputDType, outputDType, func, *funcArgs, **kwargs)

        self.setNumChannels(nchans)
        self.setupOutput(0, self.outputDType)

    def getNumChannels(self):
        return self.nchans

    def setNumChannels(self, nchans):
        if nchans <= 0:
            raise ValueError("Number of channels must be positive.")

        oldNChans = self.nchans
        for i in range(oldNChans, nchans):
            self.setupInput(i, self.inputDType)

        self.nchans = nchans

    def work(self):
        if self.callPostBuffer:
            self.workWithPostBuffer()
        else:
            self.workWithGivenOutputBuffer()

    def workWithPostBuffer(self):
        elems = self.workInfo().minAllInElements
        if 0 == elems:
            return

        # This creates a 2D ndarray containing the array subsets we're interested
        # in. This points to the input buffers themselves without copying memory.
        allArrs = numpy.array([arr[:elems].view() for arr in self.inputs()], dtype=self.numpyInputDType)

        # TODO: what happens if a function doesn't take in *args of *kwargs?
        out = numpy.apply_along_axis(self.func, 0, allArrs, *self.funcArgs, **self.funcKWargs)

        for port in self.inputs():
            port.consume(elems)
        self.output(0).postBuffer(out)

    def workWithGivenOutputBuffer(self):
        elems = self.workInfo().minAllElements
        if 0 == elems:
            return

        # This creates a 2D ndarray containing the array subsets we're interested
        # in. This points to the input buffers themselves without copying memory.
        allArrs = numpy.array([arr[:elems].view() for arr in self.inputs()], dtype=self.numpyInputDType)
        out0 = self.output(0).buffer()

        # TODO: what happens if a function doesn't take in *args of *kwargs?
        out0[:elems] = numpy.apply_along_axis(self.func, 0, allArrs, *self.funcArgs, **self.funcKWargs)

        for port in self.inputs():
            port.consume(elems)
        self.output(0).produce(elems)
