# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *
from . import Utility

import Pothos

import numpy
import numpy.fft

class FFTClass(BaseBlock):
    def __init__(self, func, inputDType, outputDType, inputDTypeArgs, outputDTypeArgs, numBins, warnIfSuboptimal=True):
        BaseBlock.__init__(self, func, inputDType, outputDType, inputDTypeArgs, outputDTypeArgs)

        # The FFT algorithm is fastest for powers of 2.
        if warnIfSuboptimal and not numpy.log2(numBins).is_integer():
            logger = Utility.PythonLogger(str(func))
            logger.log(
                str(func),
                "numBins was specified as {0}, which is not a power of 2. This will result in suboptimal performance.".format(numBins),
                "WARNING")

        self.numBins = numBins

        self.setupInput(0, inputDType)
        self.setupOutput(0, outputDType)
        self.input(0).setReserve(numBins)

    def work(self):
        elems = self.workInfo().minAllElements
        if 0 == elems:
            return

        in0 = self.input(0)
        out0 = self.output(0)

        output = self.func(in0.buffer()).astype(self.numpyOutputDType)

        in0.consume(self.numBins)
        out0.postBuffer(output)

#
# Factories exposed to C++ layer
#

# TODO: enforce scalar

def FFT(dtype, numBins):
    dtypeArgs = dict(supportComplex=True)

    return FFTClass(
               numpy.fft.fft,
               dtype,
               dtype,
               dtypeArgs,
               dtypeArgs,
               numBins)

def IFFT(dtype, numBins):
    dtypeArgs = dict(supportComplex=True)

    return FFTClass(
               numpy.fft.ifft,
               dtype,
               dtype,
               dtypeArgs,
               dtypeArgs,
               numBins)

def RFFT(dtype, numBins):
    # Validate here so we get a clearer error message than DType complaining about
    # complex_complex_X.
    Utility.validateDType(dtype, dict(supportFloat=True))
    complexDType = Utility.DType("complex_"+dtype.toString())

    # Since we've already validated, skip validation in the base block.
    return FFTClass(
               numpy.fft.rfft,
               dtype,
               complexDType,
               dict(supportAll=True),
               dict(supportAll=True),
               numBins)

def IRFFT(dtype, numBins):
    # Validate here so we get a clearer error message than DType complaining about
    # complex_complex_X.
    Utility.validateDType(dtype, dict(supportFloat=True))
    complexDType = Utility.DType("complex_"+dtype.toString())

    # Since we've already validated, skip validation in the base block.
    return FFTClass(
               numpy.fft.irfft,
               complexDType,
               dtype,
               dict(supportAll=True),
               dict(supportAll=True),
               numBins,
               warnIfSuboptimal=False)
