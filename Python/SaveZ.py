# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

from . import Utility

import Pothos

import numpy
import os

class SaveZBlock(BaseBlock):
    def __init__(self, func, dtype, filepath, key, compressed, append):
        if os.path.splitext(filepath)[1] != ".npz":
            raise RuntimeError("Only .npz files are supported.")

        if type(dtype) is str:
            dtype = Utility.DType(dtype)

        if type(key) is not str:
            raise ValueError("Key must be a string. Got {0}".format(type(key)))

        dtypeArgs = dict(supportAll=True)
        BaseBlock.__init__(self, "/numpy/savez", func, dtype, None, dtypeArgs, None, list(), dict())

        self.__buffers = dict()
        if os.path.exists(filepath):
            self.__buffers = dict(numpy.load(filepath))

        self.__filepath = filepath
        self.__key = key
        self.__compressed = compressed
        self.__append = append
        self.__buffer = numpy.array([], dtype=self.numpyInputDType)
        self.__allKeys = list(self.__buffers.keys())

        self.setupInput("0", dtype)

    def deactivate(self):
        # The .npz file format is intended to take a single array write,
        # so to do this, we'll just accumulate what the previous block has
        # provided and write at the end of the topology.
        if len(self.__buffer) > 0:
            if (self.__key in self.__buffers) and self.__append:
                self.__buffers[self.__key] = numpy.concatenate([self.__buffers[self.__key], self.__buffer])
            else:
                self.__buffers[self.__key] = self.__buffer

            self.func(self.__filepath, **self.__buffers)
            os.sync()

    def getFilepath(self):
        return self.__filepath

    def getKey(self):
        return self.__key

    def getCompressed(self):
        return self.__compressed

    def getAppend(self):
        return self.__append

    def getAllKeys(self):
        return self.__allKeys

    def work(self):
        in0 = self.input(0).buffer()
        n = len(in0)

        if 0 == n:
            return

        self.__buffer = numpy.concatenate([self.__buffer, in0])
        self.input(0).consume(n)

def SaveZ(filepath, key, dtype, compressed, append):
    func = numpy.savez_compressed if compressed else numpy.savez
    return SaveZBlock(func, dtype, filepath, key, compressed, append)
