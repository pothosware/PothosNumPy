# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

import Pothos

import numpy

class SimpleSource(Pothos.Block):
    def __init__(self, dtype, func, *args):
        Pothos.Block.__init__(self)
        self.setupOutput("0", dtype)

        self.__func = func
        self.__numpyDType = Pothos.Buffer.dtype_to_numpy(dtype)
        self.__args = args

    def work(self):
        out0 = self.output(0).buffer()
        N = len(out0)

        if N == 0:
            return

        if self.__args is None:
            out0[:] = self.__func(N, dtype=self.__numpyDType)
        else:
            out0[:] = self.__func(N, *self.__args, dtype=self.__numpyDType)

        self.output(0).produce(N)

#
# Factories exposed to C++ layer
#

def NumPyOnes(dtype):
    return SimpleSource(dtype, numpy.ones)

def NumPyZeros(dtype):
    return SimpleSource(dtype, numpy.zeros)
