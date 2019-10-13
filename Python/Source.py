# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *
from . import Utility

import Pothos

import numpy

class SingleOutputSource(BaseBlock):
    def __init__(self, func, dtype, dtypeArgs, funcArgs, funcKWargs, *args, **kwargs):
        if dtype is None:
            raise ValueError("Null dtype")

        BaseBlock.__init__(self, func, None, dtype, None, dtypeArgs, funcArgs, funcKWargs, *args, **kwargs)
        self.setupOutput(0, self.outputDType)

        self.useShape = kwargs.get("useShape", True)

    def work(self):
        assert(self.numpyOutputDType is not None)

        if self.callPostBuffer:
            self.workWithPostBuffer()
        else:
            self.workWithGivenOutputBuffer()

    def workWithPostBuffer(self):
        out = self.func(*self.funcArgs, **self.funcKWargs).astype(self.numpyOutputDType, copy=False)
        self.output(0).postBuffer(out)

    def workWithGivenOutputBuffer(self):
        elems = len(self.output(0).buffer())
        if 0 == elems:
            return

        funcArgs = ([elems] + self.funcArgs) if self.useShape else self.funcArgs
        funcArgs = funcArgs + [elems] if self.sizeParam else funcArgs

        out0 = self.output(0).buffer()
        out0[:elems] = self.func(*funcArgs, **self.funcKWargs).astype(self.numpyOutputDType, copy=False)
        self.output(0).produce(elems)
