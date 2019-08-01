# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from . import Utility

import Pothos

import numpy

class BaseBlock(Pothos.Block):
    def __init__(self, inputDType, outputDType, func, *funcArgs, **kwargs):
        Pothos.Block.__init__(self)
        Utility.validateDType(inputDType, **kwargs)
        Utility.validateDType(outputDType, **kwargs)

        self.inputDType = inputDType
        self.outputDType = outputDType
        self.func = func
        self.funcArgs = funcArgs
        self.funcKWargs = dict()

        # Common kwargs values
        self.useDType = kwargs.get("useDType", True)
        self.callPostBuffer = kwargs.get("callPostBuffer", False)

        # Other values to assemble from user input
        self.numpyInputDType = None if inputDType is None else Pothos.Buffer.dtype_to_numpy(self.inputDType)
        self.numpyOutputDType = None if outputDType is None else Pothos.Buffer.dtype_to_numpy(self.outputDType)

        if kwargs.get("useDType", True):
            self.funcKWargs["dtype"] = self.numpyInputDType
