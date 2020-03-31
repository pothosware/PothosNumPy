# Copyright (c) 2019-2020 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

from . import Utility

import Pothos

import numpy
import os

class FileSourceBaseBlock(BaseBlock):
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

    def filepath(self):
        return self.__filepath

    def repeat(self):
        return self.__repeat

    def setRepeat(self, repeat):
        self.__repeat = repeat

    def work(self):
        out0 = self.output(0).buffer()
        if 0 == len(out0):
            return

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
 * |PothosDoc .npy File Source
 *
 * Corresponding NumPy function: <b>numpy.load</b> (with .npy extension)
 *
 * |category /NumPy/File IO
 * |keywords load numpy binary file IO
 * |factory /numpy/npy_source(filepath,repeat)
 * |setter setRepeat(repeat)
 *
 * |param filepath[Filepath]
 * |widget FileEntry(mode=open)
 * |default ""
 * |preview enable
 *
 * |param repeat[Repeat?]
 * |widget ToggleSwitch(on="True",off="False")
 * |default false
 * |preview enable
 */
"""
class NpyFileSource(FileSourceBaseBlock):
    def __init__(self, filepath, repeat):
        FileSourceBaseBlock.__init__(self, "/numpy/npy_source", filepath, ".npy", repeat)

        # Note: "with numpy.load... as" only works in NumPy 1.15 and up
        self.data = numpy.load(filepath, "r")
        self.setupOutputFromDataDType()

"""
/*
 * |PothosDoc .npz File Source
 *
 * Corresponding NumPy function: <b>numpy.load</b> (with .npz extension)
 *
 * |category /NumPy/File IO
 * |keywords load numpy binary file IO
 * |factory /numpy/npz_source(filepath,key,repeat)
 * |setter setRepeat(repeat)
 *
 * |param filepath[Filepath]
 * |widget FileEntry(mode=open)
 * |default ""
 * |preview enable
 *
 * |param key[Key]
 * |default ""
 *
 * |param repeat[Repeat?]
 * |widget ToggleSwitch(on="True",off="False")
 * |default false
 * |preview enable
 */
"""
class NpzFileSource(FileSourceBaseBlock):
    def __init__(self, filepath, key, repeat):
        FileSourceBaseBlock.__init__(self, "/numpy/npz_source", filepath, ".npz", repeat)

        # Note: "with numpy.load... as" only works in NumPy 1.15 and up
        npzContents = numpy.load(filepath, "r")
        self.__allKeys = list(dict(npzContents).keys())

        if key not in npzContents:
            raise KeyError('Could not find key "{0}".'.format(key))

        self.data = npzContents[key]
        self.setupOutputFromDataDType()

        self.__key = key

    def key(self):
        return self.__key

    def allKeys(self):
        return self.__allKeys
