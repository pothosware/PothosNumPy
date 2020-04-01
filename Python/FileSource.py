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
        self.__is1D = True

    def validateDataAndSetupOutput(self):
        if len(self.data.shape) not in [1,2]:
            raise RuntimeError("This block only supports 1D or 2D arrays.")

        dtype = Utility.DType(self.data.dtype.name)
        dtypeArgs = dict(supportAll=True)
        self.initDTypes(None, dtype, None, dtypeArgs)

        for port in range(self.data.shape[0]):
            self.setupOutput(port, dtype)

        self.__is1D = (1 == len(self.data.shape))

    def filepath(self):
        return self.__filepath

    def repeat(self):
        return self.__repeat

    def setRepeat(self, repeat):
        self.__repeat = repeat

    def getOutputLenAndNewPos(self, data, output):
        n = min(len(output.buffer()), (len(data) - self.__pos))

        if 0 == n:
            if self.__repeat:
                self.__pos = 0
                n = min(len(output.buffer()), (len(data) - self.__pos))
            else:
                return (-1, -1)

        newPos = self.__pos + n
        return n, newPos

    def work1D(self):
        elems = self.workInfo().minElements
        if 0 == elems:
            return

        output = self.output(0)

        n, newPos = self.getOutputLenAndNewPos(self.data, output)
        if -1 == n:
            return

        output.buffer()[:n] = self.data[self.__pos:newPos].astype(self.numpyOutputDType)
        output.produce(n)

        self.__pos = newPos

    def work2D(self):
        elems = self.workInfo().minElements
        if 0 == elems:
            return

        outputs = self.outputs()

        # Since NumPy arrays cannot be jagged, it is safe to pass in the
        # first buffer.
        n, newPos = self.getOutputLenAndNewPos(self.data[0], outputs[0])
        if -1 == n:
            return

        for (data, output) in zip(self.data, outputs):
            output.buffer()[:n] = data[self.__pos:newPos].astype(self.numpyOutputDType)
            output.produce(n)

        self.__pos = newPos

    def work(self):
        if self.__is1D:
            self.work1D()
        else:
            self.work2D()

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
        self.validateDataAndSetupOutput()

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
        self.validateDataAndSetupOutput()

        self.__key = key

    def key(self):
        return self.__key

    def allKeys(self):
        return self.__allKeys
