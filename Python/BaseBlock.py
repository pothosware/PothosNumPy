# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from . import Utility

import Pothos

import logging
import numpy

class BaseBlock(Pothos.Block):
    def __init__(self, blockPath, func, inputDType, outputDType, inputDTypeArgs, outputDTypeArgs, funcArgs, funcKWargs, *args, **kwargs):
        Pothos.Block.__init__(self)

        self.func = func
        self.funcArgs = funcArgs
        self.funcKWargs = funcKWargs

        # Common kwargs values
        self.useDType = kwargs.get("useDType", True)
        self.callPostBuffer = kwargs.get("callPostBuffer", False)
        self.sizeParam = kwargs.get("sizeParam", False)

        self.initDTypes(inputDType, outputDType, inputDTypeArgs, outputDTypeArgs)

        # Set up logging for this block
        self.logger = logging.getLogger(blockPath)
        self.logger.addHandler(Pothos.LogHandler(blockPath))

    def initDTypes(self, inputDType, outputDType, inputDTypeArgs, outputDTypeArgs):
        if inputDType is not None:
            if type(inputDType) is str:
                inputDType = Utility.DType(inputDType)

            Utility.validateDType(inputDType, inputDTypeArgs)
        if outputDType is not None:
            if type(outputDType) is str:
                outputDType = Utility.DType(outputDType)

            Utility.validateDType(outputDType, outputDTypeArgs)

        self.inputDType = inputDType
        self.outputDType = outputDType

        # Other values to assemble from user input
        self.numpyInputDType = None if inputDType is None else Pothos.Buffer.dtype_to_numpy(self.inputDType)
        self.numpyOutputDType = None if outputDType is None else Pothos.Buffer.dtype_to_numpy(self.outputDType)

        if self.useDType:
            self.funcKWargs["dtype"] = self.numpyInputDType if self.numpyInputDType is not None else self.numpyOutputDType

