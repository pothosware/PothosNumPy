# Copyright (c) 2019-2020 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

from . import Utility

import Pothos

import numpy
import os

# TODO: implement "append" option
"""
/*
 * |PothosDoc .npy File Sink
 *
 * Corresponding NumPy function: <b>numpy.save</b>
 *
 * |category /NumPy/File IO
 * |keywords save numpy binary file IO
 * |factory /numpy/npy_sink(filepath,dtype,nchans,append)
 *
 * |param filepath[Filepath]
 * |widget FileEntry(mode=save)
 * |default ""
 * |preview enable
 *
 * |param dtype[Data Type] The block data type.
 * |widget DTypeChooser(int=1,uint=1,float=1,cfloat=1)
 * |default "float64"
 * |preview disable
 *
 * |param nchans[Num Channels] The number of inputs.
 * |widget SpinBox(minimum=1)
 * |default 1
 * |preview disable
 *
 * |param append[Append?]
 * |default false
 * |widget ToggleSwitch(on="True",off="False")
 * |preview enable
 */
"""
class NpyFileSink(BaseBlock):
    def __init__(self, filepath, dtype, nchans, append):
        if os.path.splitext(filepath)[1] != ".npy":
            raise RuntimeError("Only .npy files are supported.")

        dtype = Utility.toDType(dtype)

        dtypeArgs = dict(supportAll=True)
        BaseBlock.__init__(self, "/numpy/npy_sink", numpy.save, dtype, None, dtypeArgs, None, list(), dict())

        if 1 == nchans:
            self.logger.info("The \"nchans\" options is currently unimplemented.")

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

    def filepath(self):
        return self.__filepath

    def append(self):
        self.logger.info("The \"append\" option is currently unimplemented.")
        return False

    def setAppend(self, append):
        self.logger.info("The \"append\" option is currently unimplemented.")

    def work(self):
        if 0 == self.workInfo().minAllInElements:
            return

        in0 = self.input(0).buffer()
        N = len(in0)

        self.__buffer = numpy.concatenate([self.__buffer, in0])
        self.input(0).consume(N)

"""
/*
 * |PothosDoc .npz File Sink
 *
 * Corresponding NumPy functions: <b>numpy.savez</b>, <b>numpy.savez_compressed</b>
 *
 * |category /NumPy/File IO
 * |keywords save numpy binary file IO
 * |factory /numpy/npz_sink(filepath,key,dtype,nchans,compressed,append)
 *
 * |param filepath[Filepath]
 * |widget FileEntry(mode=save)
 * |default ""
 * |preview enable
 *
 * |param key[Key]
 * |widget StringEntry()
 * |default "key"
 * |preview enable
 *
 * |param dtype[Data Type] The block data type.
 * |widget DTypeChooser(int=1,uint=1,float=1,cfloat=1)
 * |default "float64"
 * |preview disable
 *
 * |param nchans[Num Channels] The number of inputs.
 * |widget SpinBox(minimum=1)
 * |default 1
 * |preview disable
 *
 * |param compressed[Compressed?]
 * |default false
 * |widget ToggleSwitch(on="True",off="False")
 * |preview enable
 *
 * |param append[Append?]
 * |default false
 * |widget ToggleSwitch(on="True",off="False")
 * |preview enable
 */
"""
class SaveZBlock(BaseBlock):
    def __init__(self, func, dtype, filepath, key, nchans, compressed, append):
        if os.path.splitext(filepath)[1] != ".npz":
            raise RuntimeError("Only .npz files are supported.")

        dtype = Utility.toDType(dtype)

        if type(key) is not str:
            raise ValueError("Key must be a string. Got {0}".format(type(key)))

        dtypeArgs = dict(supportAll=True)
        BaseBlock.__init__(self, "/numpy/npz_sink", func, dtype, None, dtypeArgs, None, list(), dict())

        if 1 == nchans:
            self.logger.info("The \"nchans\" options is currently unimplemented.")

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

    def filepath(self):
        return self.__filepath

    def key(self):
        return self.__key

    def compressed(self):
        return self.__compressed

    def append(self):
        return self.__append

    def setAppend(self, append):
        self.__append = append

    def allKeys(self):
        return self.__allKeys

    def work(self):
        if 0 == self.workInfo().minAllInElements:
            return

        in0 = self.input(0).buffer()
        N = len(in0)

        self.__buffer = numpy.concatenate([self.__buffer, in0])
        self.input(0).consume(N)

def NpzFileSink(filepath, key, dtype, nchans, compressed, append):
    func = numpy.savez_compressed if compressed else numpy.savez
    return SaveZBlock(func, dtype, filepath, key, nchans, compressed, append)
