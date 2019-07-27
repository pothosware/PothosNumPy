# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

import Pothos

import numpy
import os

# TODO: dimension enforcement?
class SaveZ(Pothos.Block):
    def __init__(self, filepath, dtypes, keys, func):
        Pothos.Block.__init__(self)

        if (len(dtypes) != len(keys)) or (len(dtypes) != len(keys)):
            raise ValueError("Input sizes for channel generation must match.")

        self.__filepath = filepath
        self.__func = func
        self.__buffers = dict()

        for (dtype, key) in zip(dtypes, keys):
            self.__buffers[key] = []
            self.setupInput(key, dtype)

    def activate(self):
        if os.path.splitext(self.__filepath)[1] != ".npy":
            raise RuntimeError("Only .npy files are supported.")

    def deactivate(self):
        # The .npz file format is intended to take buffers in a single write,
        # so to do this, we'll just accumulate what the previous blocks have
        # provided and write at the end of the topology.
        self.__func(self.__filepath, **self.__buffers)

    def getFilepath(self):
        return self.__filepath

    def work(self):
        elems = self.workInfo().minAllInElements

        if 0 == elems:
            return

        allInputs = self.allInputs()
        for inputKey in allInputs:
            self.__buffers[key] += allInputs[key].buffer()
            allInputs[key].consume(elems)

def SaveZ(filepath, dtypes, keys):
    return SaveZ(filepath, dtypes, keys, numpy.savez)

def SaveZCompressed(filepath, dtypes, keys):
    return SaveZ(filepath, dtypes, keys, numpy.savez_compressed)
