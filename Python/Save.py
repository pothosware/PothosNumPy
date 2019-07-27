# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from . import Utility

import Pothos

import numpy
import os

class Save(Pothos.Block):
    def __init__(self, dtype, filepath):
        Utility.validateDType(dtype)

        Pothos.Block.__init__(self)
        self.setupInput("0", self.__dtype)

        self.__dtype = dtype
        self.__filepath = filepath
        self.__buffer = []

    def activate(self):
        if os.path.splitext(self.__filepath)[1] != ".npy":
            raise RuntimeError("Only .npy files are supported.")

    def deactivate(self):
        # The .npy file format is intended to take a single array write,
        # so to do this, we'll just accumulate what the previous block has
        # provided and write at the end of the topology.
        if len(self.__buffer) > 0:
            numpy.save(self.__filepath, self.__buffer)

    def getFilepath(self):
        return self.__filepath

    def work(self):
        in0 = self.input(0).buffer()
        n = len(in0)

        if 0 == n:
            return

        self.__buffer += in0
        in0.consume(n)
