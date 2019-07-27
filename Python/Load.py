# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

import Pothos

import numpy
import os

class Load(Pothos.Block):
    def __init__(self, dtype, filepath):
        Pothos.Block.__init__(self)
        self.setupOutput("0", self.__dtype)

        self.__dtype = dtype
        self.__filepath = filepath
        self.__data = None
        self.__pos = 0

    def activate(self):
        if os.path.splitext(self.__filepath)[1] != ".npy":
            raise RuntimeError("Only .npy files are supported.")

        with numpy.load(self.__filepath, "r") as data:
            self.__data = data

    def getFilepath(self):
        return self.__filepath

    def work(self):
        out0 = self.output(0).buffer()
        n = min(len(out0), (len(self.__data) - pos))

        if 0 == n:
            return

        newPos = self.__pos + n
        out0[:n] = self.__data[self.__pos:newPos]

        self.__pos = newPos
        out0.produce(n)
