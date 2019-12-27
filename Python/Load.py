# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

from . import Utility

import Pothos

import numpy
import os

class LoadBaseBlock(BaseBlock):
    def __init__(self, blockPath, filepath, extension, repeat):
        if not os.path.exists(filepath):
            raise IOError("The given file does not exist: {0}".format(filepath))
        if os.path.splitext(filepath)[1] != extension:
            raise RuntimeError("This block only accepts {0} files.".format(extension))

        dtypeArgs = dict(supportAll=True)
        BaseBlock.__init__(self, blockPath, numpy.load, None, None, None, None, list(), dict())

        self.__filepath = filepath
        self.__pos = 0
        self.__repeat = repeat

        self.data = None

    def setupOutputFromDataDType(self):
        dtype = Utility.DType(self.data.dtype.name)
        dtypeArgs = dict(supportAll=True)
        self.initDTypes(None, dtype, None, dtypeArgs)

        self.setupOutput(0, dtype)

    def getFilepath(self):
        return self.__filepath

    def getRepeat(self):
        return self.__repeat

    def setRepeat(self, repeat):
        self.__repeat = repeat

    def work(self):
        out0 = self.output(0).buffer()
        n = min(len(out0), (len(self.data) - self.__pos))

        if 0 == n:
            if self.__repeat:
                self.__pos = 0
                n = min(len(out0), (len(self.data) - self.__pos))
            else:
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
 * |category /NumPy/File IO
 * |keywords load numpy binary file IO
 * |factory /numpy/load_npy(filepath,repeat)
 * |setter setRepeat(repeat)
 *
 * |param filepath(File path)
 * |widget FileEntry(mode=open)
 * |default ""
 * |preview enable
 *
 * |param repeat(Repeat?)
 * |widget ToggleSwitch(on="True",off="False")
 * |default false
 * |preview enable
 */
"""
class LoadNpy(LoadBaseBlock):
    def __init__(self, filepath, repeat):
        LoadBaseBlock.__init__(self, "/numpy/load_npy", filepath, ".npy", repeat)

        # Note: "with numpy.load... as" only works in NumPy 1.15 and up
        self.data = numpy.load(filepath, "r")
        self.setupOutputFromDataDType()

"""
/*
 * |PothosDoc Load .npz
 *
 * Corresponding NumPy function: numpy.load (with .npz extension)
 *
 * |category /NumPy/File IO
 * |keywords load numpy binary file IO
 * |factory /numpy/load_npz(filepath,key)
 * |setter setRepeat(repeat)
 *
 * |param filepath(File path)
 * |widget FileEntry(mode=open)
 * |default ""
 * |preview enable
 *
 * |param key(Key)
 * |widget StringEntry()
 * |default "key"
 * |preview enable
 *
 * |param repeat(Repeat?)
 * |widget ToggleSwitch(on="True",off="False")
 * |default false
 * |preview enable
 */
"""
# TODO: can I make an "overlay" to choose the key?
class LoadNpz(LoadBaseBlock):
    def __init__(self, filepath, key, repeat):
        LoadBaseBlock.__init__(self, "/numpy/load_npz", filepath, ".npz", repeat)

        # Note: "with numpy.load... as" only works in NumPy 1.15 and up
        npzContents = numpy.load(filepath, "r")
        self.__allKeys = list(dict(npzContents).keys())

        if key not in npzContents:
            raise KeyError('Could not find key "{0}".'.format(key))

        self.data = npzContents[key]
        self.setupOutputFromDataDType()

        self.__key = key

    def getKey(self):
        return self.__key

    def getAllKeys(self):
        return self.__allKeys
