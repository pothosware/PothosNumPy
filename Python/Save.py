# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

from . import Utility

import Pothos

import numpy
import os

# TODO: implement "append" option
"""
/*
 * |PothosDoc Save .npy
 *
 * Corresponding NumPy function: numpy.save
 *
 * |category /NumPy/File IO
 * |keywords save numpy binary file IO
 * |factory /numpy/save_npy(filepath,dtype,append)
 *
 * |param filepath(File path)
 * |widget FileEntry(mode=save)
 * |default ""
 * |preview enable
 *
 * |param dtype(Data Type) The block data type.
 * |widget DTypeChooser(int=1,uint=1,float=1,cfloat=1)
 * |default "float64"
 * |preview disable
 *
 * |param append(Append?)
 * |default false
 * |widget ToggleSwitch(on="True",off="False")
 * |preview enable
 */
"""
class SaveNpy(BaseBlock):
    def __init__(self, filepath, dtype, append):
        if os.path.splitext(filepath)[1] != ".npy":
            raise RuntimeError("Only .npy files are supported.")

        dtype = Utility.toDType(dtype)

        dtypeArgs = dict(supportAll=True)
        BaseBlock.__init__(self, "/numpy/save_npy", numpy.save, dtype, None, dtypeArgs, None, list(), dict())

        if "int64" in dtype.toString():
            self.logger.warning(
                "This block supports type {0}, but input values are not guaranteed " \
                "to be preserved due to limitations of type conversions between " \
                "C++ and Python.".format(dtype.toString()))

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

    def getFilepath(self):
        return self.__filepath

    def getAppend(self):
        self.logger.info("The \"append\" option is currently unimplemented.")
        return False

    def setAppend(self, append):
        self.logger.info("The \"append\" option is currently unimplemented.")

    def work(self):
        in0 = self.input(0).buffer()
        n = len(in0)

        if 0 == n:
            return

        self.__buffer = numpy.concatenate([self.__buffer, in0])
        self.input(0).consume(n)

# TODO: overlay
"""
/*
 * |PothosDoc Save .npz
 *
 * Corresponding NumPy functions: numpy.savez, numpy.savez_compressed
 *
 * |category /NumPy/File IO
 * |keywords save numpy binary file IO
 * |factory /numpy/save_npz(filepath,key,dtype,compressed,append)
 *
 * |param filepath(File path)
 * |widget FileEntry(mode=save)
 * |default ""
 * |preview enable
 *
 * |param key(Key)
 * |widget StringEntry()
 * |default "key"
 * |preview enable
 *
 * |param dtype(Data Type) The block data type.
 * |widget DTypeChooser(int=1,uint=1,float=1,cfloat=1)
 * |default "float64"
 * |preview disable
 *
 * |param compressed(Compressed?)
 * |default false
 * |widget ToggleSwitch(on="True",off="False")
 * |preview enable
 *
 * |param append(Append?)
 * |default false
 * |widget ToggleSwitch(on="True",off="False")
 * |preview enable
 */
"""
class SaveZBlock(BaseBlock):
    def __init__(self, func, dtype, filepath, key, compressed, append):
        if os.path.splitext(filepath)[1] != ".npz":
            raise RuntimeError("Only .npz files are supported.")

        dtype = Utility.toDType(dtype)

        if type(key) is not str:
            raise ValueError("Key must be a string. Got {0}".format(type(key)))

        dtypeArgs = dict(supportAll=True)
        BaseBlock.__init__(self, "/numpy/save_npz", func, dtype, None, dtypeArgs, None, list(), dict())

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

    def setAppend(self, append):
        self.__append = append

    def getAllKeys(self):
        return self.__allKeys

    def work(self):
        in0 = self.input(0).buffer()
        n = len(in0)

        if 0 == n:
            return

        self.__buffer = numpy.concatenate([self.__buffer, in0])
        self.input(0).consume(n)

def SaveNpz(filepath, key, dtype, compressed, append):
    func = numpy.savez_compressed if compressed else numpy.savez
    return SaveZBlock(func, dtype, filepath, key, compressed, append)
