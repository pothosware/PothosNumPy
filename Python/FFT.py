# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from . import Utility

import Pothos

import numpy
import numpy.fft

class FFTClass(Pothos.Block):
    def __init__(self, inputDType, outputDType, func, validateFunc, numBins, *args, warnIfSuboptimal=True):
        validateFunc(inputDType)
        validateFunc(outputDType)

        # The FFT algorithm is fastest for powers of 2.
        if warnIfSuboptimal and not numpy.log2(numBins).is_integer():
            logger = Utility.PythonLogger(str(func))
            logger.log(
                str(func),
                "numBins was specified as {0}, which is not a power of 2. This will result in suboptimal performance.".format(numBins),
                "WARNING")

        Pothos.Block.__init__(self)

        self.numpyInputDType = Pothos.Buffer.dtype_to_numpy(inputDType)
        self.numpyOutputDType = Pothos.Buffer.dtype_to_numpy(outputDType)
        self.func = func
        self.args = args
        self.numBins = numBins

        self.setupInput(0, inputDType)
        self.setupOutput(0, outputDType)
        self.input(0).setReserve(numBins * inputDType.dimension())

    def work(self):
        elems = self.workInfo().minAllElements
        if 0 == elems:
            return

        in0 = self.input(0)
        out0 = self.output(0)

        output = self.func(in0.buffer(), *self.args).astype(self.numpyOutputDType)

        in0.consume(self.numBins)
        out0.postBuffer(output)

#
# Factories exposed to C++ layer
#

# TODO: enforce scalar

def FFT(dtype, numBins):
    complexDType = Utility.DType("complex_"+dtype.toString())
    return FFTClass(
               complexDType,
               complexDType,
               numpy.fft.fft,
               Utility.validateDType,
               numBins)

def IFFT(dtype, numBins):
    complexDType = Utility.DType("complex_"+dtype.toString())
    return FFTClass(
               complexDType,
               complexDType,
               numpy.fft.ifft,
               Utility.validateDType,
               numBins)

def RFFT(dtype, numBins):
    complexDType = Utility.DType("complex_"+dtype.toString())
    return FFTClass(
               dtype,
               complexDType,
               numpy.fft.rfft,
               Utility.validateDType,
               numBins)

def IRFFT(dtype, numBins):
    complexDType = Utility.DType("complex_"+dtype.toString())
    return FFTClass(
               complexDType,
               dtype,
               numpy.fft.irfft,
               Utility.validateDType,
               numBins,
               warnIfSuboptimal=False)
