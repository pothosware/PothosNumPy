# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from . import Utility

import Pothos

import numpy
import os

# Different enough that our BaseBlock doesn't apply
class SaveZBlock(Pothos.Block):
    def __init__(self, filepath, dtypes, keys, func):
        if os.path.splitext(filepath)[1] != ".npz":
            raise RuntimeError("Only .npz files are supported.")

        Pothos.Block.__init__(self)

        if ((dtypes == None) != (keys == None)):
            raise ValueError("Input sizes for channel generation must match.")
        if (dtypes != None) and (keys != None) and (len(dtypes) != len(keys)):
            raise ValueError("Input sizes for channel generation must match.")

        self.__filepath = filepath
        self.__func = func
        self.__buffers = dict()

        if dtypes != None:
            for (dtype, key) in zip(dtypes, keys):
                self.addChannel(dtype, key)

    # Essentially setupInput with validation
    def addChannel(self, dtype, key):
        Utility.validateDType(dtype, dict(supportAll=True))
        if "int64" in dtype.toString():
            logger = Utility.PythonLogger(self.__class__.__name__)
            logger.log(
                str(self.__class__.__name__),
                "This block supports type {0}, but input values are not guaranteed " \
                "to be preserved due to limitations of type conversions between " \
                "C++ and Python.".format(dtype.toString()),
                "WARNING")

        self.__buffers[key] = numpy.array([], dtype=Pothos.Buffer.dtype_to_numpy(dtype))
        self.setupInput(key, dtype)

    def deactivate(self):
        # The .npz file format is intended to take buffers in a single write,
        # so to do this, we'll just accumulate what the previous blocks have
        # provided and write at the end of the topology.
        self.__func(self.__filepath, **self.__buffers)
        os.sync()

    def getFilepath(self):
        return self.__filepath

    def work(self):
        for key,buf in self.allInputs().items():
            elems = buf.elements()
            if elems > 0:
                self.__buffers[key] = numpy.concatenate([self.__buffers[key], buf.buffer()])
                buf.consume(elems)

def SaveZ(filepath, dtypes=None, keys=None):
    return SaveZBlock(filepath, dtypes, keys, numpy.savez)

def SaveZCompressed(filepath, dtypes=None, keys=None):
    return SaveZBlock(filepath, dtypes, keys, numpy.savez_compressed)
