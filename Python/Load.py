# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

from . import Utility

import Pothos

import numpy
import os

# TODO: file repeating

class LoadBaseBlock(BaseBlock):
    def __init__(self, blockPath, filepath, extension):
        if not os.path.exists(filepath):
            raise IOError("The given file does not exist: {0}".format(filepath))
        if os.path.splitext(filepath)[1] != extension:
            raise RuntimeError("This block only accepts {0} files.".format(extension))

        dtypeArgs = dict(supportAll=True)
        BaseBlock.__init__(self, blockPath, numpy.load, None, None, None, None, list(), dict())

        self.__filepath = filepath
        self.__pos = 0

        self.data = None

    def setupOutputFromDataDType(self):
        dtype = Utility.DType(self.data.dtype.name)
        dtypeArgs = dict(supportAll=True)
        self.initDTypes(None, dtype, None, dtypeArgs)

        self.setupOutput(0, dtype)

    def getFilepath(self):
        return self.__filepath

    def work(self):
        out0 = self.output(0).buffer()
        n = min(len(out0), (len(self.data) - self.__pos))

        if 0 == n:
            return

        newPos = self.__pos + n
        out0[:n] = self.data[self.__pos:newPos].astype(self.numpyOutputDType)

        self.__pos = newPos
        self.output(0).produce(n)

"""
/*
 * |PothosDoc Load .npy
 *
 * Corresponding NumPy function: numpy.load (with .npy extension)
 *
 * |category /NumPy/Sources
 * |keywords load numpy binary file IO
 * |factory /numpy/load_npy(filepath)
 *
 * |param filepath(File path)
 * |widget FileEntry(mode=open)
 * |default ""
 * |preview enable
 */
"""
class LoadNpy(LoadBaseBlock):
    def __init__(self, filepath):
        LoadBaseBlock.__init__(self, "/numpy/load_npy", filepath, ".npy")

        # Note: "with numpy.load... as" only works in NumPy 1.15 and up
        self.data = numpy.load(filepath, "r")
        self.setupOutputFromDataDType()

"""
/*
 * |PothosDoc Load .npz
 *
 * Corresponding NumPy function: numpy.load (with .npz extension)
 *
 * |category /NumPy/Sources
 * |keywords load numpy binary file IO
 * |factory /numpy/load_npz(filepath,key)
 *
 * |param filepath(File path)
 * |widget FileEntry(mode=open)
 * |default ""
 * |preview enable
 *
 * |param key(Key)
 * |widget StringEntry()
 * |preview enable
 */
"""
# TODO: can I make an "overlay" to choose the key?
class LoadNpz(LoadBaseBlock):
    def __init__(self, filepath, key):
        LoadBaseBlock.__init__(self, "/numpy/load_npz", filepath, ".npz")

        # Note: "with numpy.load... as" only works in NumPy 1.15 and up
        npzContents = numpy.load(filepath, "r")
        if key not in npzContents:
            raise KeyError('Could not find key "{0}".'.format(key))

        self.data = npzContents[key]
        self.setupOutputFromDataDType()

        self.__key = key

    def getKey(self):
        return self.__key
