# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

from . import Utility

import Pothos

import numpy
import os

class Save(BaseBlock):
    def __init__(self, dtype, filepath):
        if os.path.splitext(filepath)[1] != ".npy":
            raise RuntimeError("Only .npy files are supported.")

        if type(dtype) is str:
            dtype = Utility.DType(dtype)

        dtypeArgs = dict(supportAll=True)
        BaseBlock.__init__(self, numpy.save, dtype, None, dtypeArgs, None, list(), dict())

        if "int64" in dtype.toString():
            self.logger.warning(
                "This block supports type {0}, but input values are not guaranteed " \
                "to be preserved due to limitations of type conversions between " \
                "C++ and Python.".format(dtype.toString()))

        self.__filepath = filepath
        self.__buffer = numpy.array([], dtype=self.numpyInputDType)

        self.setupInput("0", dtype)

    def deactivate(self):
        # The .npy file format is intended to take a single array write,
        # so to do this, we'll just accumulate what the previous block has
        # provided and write at the end of the topology.
        if len(self.__buffer) > 0:
            numpy.save(self.__filepath, self.__buffer)
            os.sync()

    def getFilepath(self):
        return self.__filepath

    def work(self):
        in0 = self.input(0).buffer()
        n = len(in0)

        if 0 == n:
            return

        self.__buffer = numpy.concatenate([self.__buffer, in0])
        self.input(0).consume(n)
