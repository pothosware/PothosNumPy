# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from . import Utility

import Pothos

import numpy

# These blocks ignore what was allocated for the output since
# the entire input buffer must be processed and returned at once.
class PostBufferBlock(Pothos.Block):
    def __init__(self, dtype, func, *args, useDType=True):
        Utility.validateDType(dtype)

        Pothos.Block.__init__(self)
        self.setupInput("0", dtype)
        self.setupOutput("0", dtype)

        self.func = func
        self.numpyDType = Pothos.Buffer.dtype_to_numpy(dtype)
        self.useDType = useDType

        self.args = args

    def work(self):
        elems = self.workInfo().minInElements
        if 0 == elems:
            return

        in0 = self.input(0).buffer()
        out = None

        if self.useDType:
            if (self.args is None) or (0 == len(self.args)):
                out = self.func(in0, dtype=self.numpyDType)
            else:
                out = self.func(in0, *self.args, dtype=self.numpyDType)
        else:
            if (self.args is None) or (0 == len(self.args)):
                out = self.func(in0).astype(self.numpyDType)
            else:
                out = self.func(in0, *self.args).astype(self.numpyDType)

        self.input(0).consume(elems)
        self.output(0).postBuffer(out)

class RollClass(PostBufferBlock):
    def __init__(self, dtype, shift):
        PostBufferBlock.__init__(self, dtype, numpy.roll, shift, useDType=False)
        self.setShift(shift)

    def getShift(self):
        return self.shift

    def setShift(self, shift):
        Utility.validateParameter(shift, self.numpyDType)

        self.shift = shift
        self.args = [self.shift]

# TODO: figure out with >1 dimensions

def Flip(dtype):
    return PostBufferBlock(dtype, numpy.flip, 0, useDType=False)

# TODO: test
def Roll(dtype, shift):
    return RollClass(dtype, shift)
