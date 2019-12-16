# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *
from . import Utility

import Pothos

import numpy
import numpy.fft

class FFTClass(BaseBlock):
    def __init__(self, blockPath, func, inputDType, outputDType, inputDTypeArgs, outputDTypeArgs, numBins, warnIfSuboptimal=False):
        BaseBlock.__init__(self, blockPath, func, inputDType, outputDType, inputDTypeArgs, outputDTypeArgs, list(), dict())

        # The FFT algorithm is fastest for powers of 2.
        if warnIfSuboptimal and not numpy.log2(numBins).is_integer():
            self.logger.warning(
                "numBins was specified as {0}, which is not a power of 2." \
                "This will result in suboptimal performance.".format(numBins))

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

"""/*
|PothosDoc FFT

Corresponding NumPy function: numpy.fft.fft

|category /NumPy/FFT
|keywords fft
|factory /numpy/fft/fft(dtype,numBins)
*/"""
def FFT(dtype, numBins):
    dtypeArgs = dict(supportComplex=True)

    return FFTClass(
               "/numpy/fft/fft",
               numpy.fft.fft,
               dtype,
               dtype,
               dtypeArgs,
               dtypeArgs,
               numBins,
               warnIfSuboptimal=True)

"""/*
|PothosDoc Inverse FFT

Corresponding NumPy function: numpy.fft.ifft

|category /NumPy/FFT
|keywords fft ifft
|factory /numpy/fft/ifft(dtype,numBins)
*/"""
def IFFT(dtype, numBins):
    dtypeArgs = dict(supportComplex=True)

    return FFTClass(
               "/numpy/fft/ifft",
               numpy.fft.ifft,
               dtype,
               dtype,
               dtypeArgs,
               dtypeArgs,
               numBins)

"""/*
|PothosDoc Real FFT

Corresponding NumPy function: numpy.fft.rfft

|category /NumPy/FFT
|keywords fft rfft
|factory /numpy/fft/rfft(dtype,numBins)
*/"""
def RFFT(dtype, numBins):
    # Validate here so we get a clearer error message than DType complaining about
    # complex_complex_X.
    Utility.validateDType(dtype, dict(supportFloat=True))
    complexDType = Utility.DType("complex_"+dtype.toString())

    # Since we've already validated, skip validation in the base block.
    return FFTClass(
               "/numpy/fft/rfft",
               numpy.fft.rfft,
               dtype,
               complexDType,
               dict(supportAll=True),
               dict(supportAll=True),
               numBins)

"""/*
|PothosDoc Inverse Real FFT

Corresponding NumPy function: numpy.fft.irfft

|category /NumPy/FFT
|keywords fft irfft
|factory /numpy/fft/irfft(dtype,numBins)
*/"""
def IRFFT(dtype, numBins):
    # Validate here so we get a clearer error message than DType complaining about
    # complex_complex_X.
    Utility.validateDType(dtype, dict(supportFloat=True))
    complexDType = Utility.DType("complex_"+dtype.toString())

    # Since we've already validated, skip validation in the base block.
    return FFTClass(
               "/numpy/fft/irfft",
               numpy.fft.irfft,
               complexDType,
               dtype,
               dict(supportAll=True),
               dict(supportAll=True),
               numBins)

"""/*
|PothosDoc Hermetian FFT

Corresponding NumPy function: numpy.fft.hfft

|category /NumPy/FFT
|keywords fft hfft
|factory /numpy/ffthfft(dtype,numBins)
*/"""
def HFFT(dtype, numBins):
    # Validate here so we get a clearer error message than DType complaining about
    # complex_complex_X.
    Utility.validateDType(dtype, dict(supportFloat=True))

    # Since we've already validated, skip validation in the base block.
    return FFTClass(
               "/numpy/fft/hfft",
               numpy.fft.hfft,
               dtype,
               dtype,
               dict(supportAll=True),
               dict(supportAll=True),
               numBins)

"""/*
|PothosDoc Inverse Hermetian FFT

Corresponding NumPy function: numpy.fft.ihfft

|category /NumPy/FFT
|keywords fft ihfft
|factory /numpy/fft/ihfft(dtype,numBins)
*/"""
def IHFFT(dtype, numBins):
    # Validate here so we get a clearer error message than DType complaining about
    # complex_complex_X.
    Utility.validateDType(dtype, dict(supportFloat=True, supportComplex=True))
    complexDType = dtype if "complex" in dtype.toString() else Utility.DType("complex_"+dtype.toString())

    # Since we've already validated, skip validation in the base block.
    return FFTClass(
               "/numpy/fft/ihfft",
               numpy.fft.ihfft,
               dtype,
               complexDType,
               dict(supportAll=True),
               dict(supportAll=True),
               numBins)
