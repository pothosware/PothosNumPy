# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

from . import Utility

import Pothos

import numpy
import os

# TODO: file repeating

"""
/*
 * |PothosDoc Load .npy
 *
 * Corresponding NumPy function: numpy.load (with .npy extension)
 *
 * |category /NumPy/Sources
 * |keywords load numpy binary file IO
 * |factory /numpy/load_npy(filepath,dtype)
 *
 * |param filepath(File path)
 * |widget FileEntry(mode=open)
 * |default ""
 * |preview disable
 *
 * |param dtype(Data Type)
 * |widget DTypeChooser(int=1,uint=1,float=1,cfloat=1)
 * |default "complex_float64"
 * |preview enable
 */
"""
class LoadNpy(BaseBlock):
    def __init__(self, filepath, dtype):
        if not os.path.exists(filepath):
            raise RuntimeError("The given file does not exist: {0}".format(filepath))

        if type(dtype) is str:
            dtype = Utility.DType(dtype)

        dtypeArgs = dict(supportAll=True)
        BaseBlock.__init__(self, "/numpy/load_npy", numpy.load, None, dtype, None, dtypeArgs, list(), dict())

        if "int64" in dtype.toString():
            self.logger.warning(
                "This block supports type {0}, but input values are not guaranteed " \
                "to be preserved due to limitations of type conversions between " \
                "C++ and Python.".format(dtype.toString()))

        self.__filepath = filepath
        self.__data = None
        self.__pos = 0

        self.setupOutput("0", dtype)

    def activate(self):
        # Note: "with numpy.load... as" only works in NumPy 1.15 and up
        self.__data = numpy.load(self.__filepath, "r")

    def getFilepath(self):
        return self.__filepath

    def work(self):
        out0 = self.output(0).buffer()
        n = min(len(out0), (len(self.__data) - self.__pos))

        if 0 == n:
            return

        newPos = self.__pos + n
        out0[:n] = self.__data[self.__pos:newPos].astype(self.numpyOutputDType)

        self.__pos = newPos
        self.output(0).produce(n)

"""
/*
 * |PothosDoc Load .npz
 *
 * Corresponding NumPy function: numpy.load (with .npz extension)
 *
 * |category /NumPy/Sources
 * |keywords load numpy binary file IO
 * |factory /numpy/load_npz(filepath)
 *
 * |param filepath(File path)
 * |widget FileEntry(mode=open)
 * |default ""
 * |preview disable
 */
"""
# Different enough that BaseBlock doesn't apply
class LoadNpz(Pothos.Block):
    def __init__(self, filepath):
        Pothos.Block.__init__(self)

        if not os.path.exists(filepath):
            raise RuntimeError("The given file does not exist: {0}".format(filepath))

        self.__filepath = filepath
        self.__data = dict()
        self.__pos = dict()

        numpyInput = numpy.load(filepath, "r")

        for key,buf in numpyInput.items():
            pothosDType = Utility.DType(buf.dtype.name)
            Utility.validateDType(pothosDType, dict(supportAll=True))

            self.__data[key] = buf
            self.__pos[key] = 0
            self.setupOutput(key, pothosDType)

        numpyInput.close()

    def getFilepath(self):
        return self.__filepath

    def work(self):
        for key,port in self.allOutputs().items():
            out = port.buffer()
            n = min(len(out), (len(self.__data[key]) - self.__pos[key]))

            if 0 == n:
                continue

            newPos = self.__pos[key] + n
            out[:n] = self.__data[key][self.__pos[key]:newPos]

            self.__pos[key] = newPos
            port.produce(n)
