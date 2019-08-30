# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *

from . import Utility

import Pothos

import numpy
import os

class Load(BaseBlock):
    def __init__(self, dtype, filepath):
        if os.path.splitext(filepath)[1] != ".npy":
            raise RuntimeError("Only .npy files are supported.")

        if "int64" in dtype.toString():
            logger = Utility.PythonLogger(self.__class__.__name__)
            logger.log(
                str(self.__class__.__name__),
                "This block supports type {0}, but input values are not guaranteed " \
                "to be preserved due to limitations of type conversions between " \
                "C++ and Python.".format(dtype.toString()),
                "WARNING")

        dtypeArgs = dict(supportAll=True)
        BaseBlock.__init__(self, numpy.load, None, dtype, None, dtypeArgs)

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
        out0[:n] = self.__data[self.__pos:newPos]

        self.__pos = newPos
        self.output(0).produce(n)
