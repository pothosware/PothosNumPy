# Copyright (c) 2020 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *
from . import Utility

import Pothos

import numpy

"""
/*
 * |PothosDoc As Type
 *
 * Converts all values of the incoming buffer to the specified DType and passes
 * them to the output buffer.
 *
 * Corresponding NumPy function: <b>numpy.ndarray.astype</b>
 *
 * |category /NumPy/Stream
 * |keywords cast convert
 * |factory /numpy/astype(dtype)
 *
 * |param dtype[Output Data Type] The block data type.
 * |widget DTypeChooser(int=1,uint=1,float=1,cfloat=1)
 * |default "float64"
 * |preview disable
 */
"""
class AsType(BaseBlock):
    def __init__(self, outputDType):
        outputDTypeArgs = dict(supportAll=True)
        BaseBlock.__init__(self, "/numpy/astype", None, None, outputDType, None, outputDTypeArgs, list(), dict())

        self.setupInput(0, "")
        self.setupOutput(0, self.outputDType)

    def work(self):
        elems = self.workInfo().minAllElements
        if 0 == elems:
            return

        in0 = self.input(0)
        out0 = self.output(0)
        N = min(len(in0.buffer()), len(out0.buffer()))

        out = in0.buffer()[:N].astype(self.numpyOutputDType)
        if (out is not None) and (len(out) > 0):
            out0.buffer()[:N] = out
            in0.consume(N)
            out0.produce(N)
