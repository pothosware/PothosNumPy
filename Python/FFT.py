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
                "numBins was specified as {0}, which is not a power of 2. " \
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

"""
/*
 * |PothosDoc FFT
 *
 * Compute the one-dimensional discrete Fourier Transform.
 *
 * This function computes the one-dimensional n-point discrete Fourier
 * Transform (DFT) with the efficient Fast Fourier Transform (FFT) algorithm [CT].
 *
 * FFT (Fast Fourier Transform) refers to a way the discrete Fourier Transfor
 * (DFT) can be calculated efficiently, by using symmetries in the calculated
 * terms. The symmetry is highest when n is a power of 2, and the transform is
 * therefore most efficient for these sizes.
 *
 * Corresponding NumPy function: numpy.fft.fft
 *
 * |category /NumPy/FFT
 * |keywords fft
 * |factory /numpy/fft/fft(dtype,numBins)
 *
 * |param dtype(Input Data Type) The block data type.
 * |widget DTypeChooser(float=1,cfloat=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param numBins(Num FFT Bins)
 * |default 1024
 * |option 512
 * |option 1024
 * |option 2048
 * |option 4096
 * |widget ComboBox(editable=true)
 */
"""
def FFT(dtype, numBins):
    dtype = Utility.toDType(dtype)

    return FFTClass(
               "/numpy/fft/fft",
               numpy.fft.fft,
               dtype,
               Utility.dtypeToComplex(dtype),
               dict(supportFloat=True, supportComplex=True),
               dict(supportComplex=True),
               numBins,
               warnIfSuboptimal=True)

"""
/*
 * |PothosDoc Inverse FFT
 *
 * Compute the one-dimensional inverse discrete Fourier Transform.
 *
 * This function computes the inverse of the one-dimensional n-point discrete
 * Fourier transform computed by <b>/numpy/fft/fft</b>. In other words,
 * <b>ifft(fft(a)) == a</b> to within numerical accuracy.
 *
 * The input should be ordered in the same way as is returned by fft, i.e.,
 * <ul>
 * <li><b>a[0]</b> should contain the zero frequency term,</li>
 * <li><b>a[1:n//2]</b> should contain the positive-frequency terms,</li>
 * <li><b>a[n//2 + 1:]</b> should contain the negative-frequency terms, in
 *     increasing order starting from the most negative frequency.</li>
 * </ul>
 *
 * For an even number of input points, <b>A[n//2]</b> represents the sum of the
 * values at the positive and negative Nyquist frequencies, as the two are
 * aliased together.
 *
 * Corresponding NumPy function: numpy.fft.ifft
 *
 * |category /NumPy/FFT
 * |keywords fft ifft inverse
 * |factory /numpy/fft/ifft(dtype,numBins)
 *
 * |param dtype(Input Data Type) The block data type.
 * |widget DTypeChooser(float=1,cfloat=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param numBins(Num FFT Bins)
 * |default 1024
 * |option 512
 * |option 1024
 * |option 2048
 * |option 4096
 * |widget ComboBox(editable=true)
 */
"""
def IFFT(dtype, numBins):
    dtype = Utility.toDType(dtype)

    return FFTClass(
               "/numpy/fft/ifft",
               numpy.fft.ifft,
               dtype,
               Utility.dtypeToComplex(dtype),
               dict(supportFloat=True, supportComplex=True),
               dict(supportComplex=True),
               numBins)

"""
/*
 * |PothosDoc Real FFT
 *
 * Compute the one-dimensional discrete Fourier Transform for real input.
 *
 * This function computes the one-dimensional n-point discrete Fourier
 * Transform (DFT) of a real-valued array by means of an efficient algorithm
 * called the Fast Fourier Transform (FFT).
 *
 * When the DFT is computed for purely real input, the output is
 * Hermitian-symmetric, i.e. the negative frequency terms are just the complex
 * conjugates of the corresponding positive-frequency terms, and the
 * negative-frequency terms are therefore redundant. This function does not
 * compute the negative frequency terms, and the length of the transformed
 * axis of the output is therefore n//2 + 1.
 *
 * When <b>A = rfft(a)</b> and <b>fs</b> is the sampling frequency, <b>A[0]</b>
 * contains the zero-frequency term <b>0*fs</b>, which is real due to Hermitian
 * symmetry.
 *
 * If <b>n</b> is even, <b>A[-1]</b> contains the term representing both
 * positive and negative Nyquist frequency <b>(+fs/2 and -fs/2)</b>, and must
 * also be purely real. If <b>n</b> is odd, there is no term at <b>fs/2</b>;
 * <b>A[-1]</b> contains the largest positive frequency <b>(fs/2*(n-1)/n)</b>,
 * and is complex in the general case.
 *
 * If the input a contains an imaginary part, it is silently discarded.
 *
 * Corresponding NumPy function: numpy.fft.rfft
 *
 * |category /NumPy/FFT
 * |keywords fft rfft real
 * |factory /numpy/fft/rfft(dtype,numBins)
 *
 * |param dtype(Input Data Type) The block data type.
 * |widget DTypeChooser(float=1)
 * |default "float64"
 * |preview disable
 *
 * |param numBins(Num FFT Bins)
 * |default 1024
 * |option 512
 * |option 1024
 * |option 2048
 * |option 4096
 * |widget ComboBox(editable=true)
 */
"""
def RFFT(dtype, numBins):
    dtype = Utility.toDType(dtype)

    return FFTClass(
               "/numpy/fft/rfft",
               numpy.fft.rfft,
               dtype,
               Utility.dtypeToComplex(dtype),
               dict(supportFloat=True),
               dict(supportComplex=True),
               numBins)

"""
/*
 * |PothosDoc Inverse Real FFT
 *
 * Compute the inverse of the n-point DFT for real input.
 *
 * This function computes the inverse of the one-dimensional n-point discrete
 * Fourier Transform of real input computed by <b>/numpy/fft/rfft</b>. In other
 * words, <b>irfft(rfft(a), len(a)) == a</b> to within numerical accuracy.
 *
 * The input is expected to be in the form returned by rfft, i.e. the real
 * zero-frequency term followed by the complex positive frequency terms in
 * order of increasing frequency. Since the discrete Fourier Transform of
 * real input is Hermitian-symmetric, the negative frequency terms are taken
 * to be the complex conjugates of the corresponding positive frequency terms.
 *
 * Corresponding NumPy function: numpy.fft.rifft
 *
 * |category /NumPy/FFT
 * |keywords fft rfft rifft real inverse
 * |factory /numpy/fft/irfft(dtype,numBins)
 *
 * |param dtype(Input Data Type) The block data type.
 * |widget DTypeChooser(float=1,cfloat=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param numBins(Num FFT Bins)
 * |default 1024
 * |option 512
 * |option 1024
 * |option 2048
 * |option 4096
 * |widget ComboBox(editable=true)
 */
"""
def IRFFT(dtype, numBins):
    dtype = Utility.toDType(dtype)

    return FFTClass(
               "/numpy/fft/irfft",
               numpy.fft.irfft,
               dtype,
               Utility.dtypeToScalar(dtype),
               dict(supportFloat=True, supportComplex=True),
               dict(supportFloat=True),
               numBins)

"""
/*
 * |PothosDoc Hermetian FFT
 *
 * Compute the FFT of a signal that has Hermitian symmetry, i.e., a real
 * spectrum. Here the signal has Hermitian symmetry in the time domain and
 * is real in the frequency domain.
 *
 * Corresponding NumPy function: numpy.fft.fft
 *
 * |category /NumPy/FFT
 * |keywords fft hfft hermetian
 * |factory /numpy/fft/hfft(dtype,numBins)
 *
 * |param dtype(Input Data Type) The block data type.
 * |widget DTypeChooser(float=1,cfloat=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param numBins(Num FFT Bins)
 * |default 1024
 * |option 512
 * |option 1024
 * |option 2048
 * |option 4096
 * |widget ComboBox(editable=true)
 */
"""
def HFFT(dtype, numBins):
    dtype = Utility.toDType(dtype)

    return FFTClass(
               "/numpy/fft/hfft",
               numpy.fft.hfft,
               dtype,
               Utility.dtypeToScalar(dtype),
               dict(supportFloat=True, supportComplex=True),
               dict(supportFloat=True),
               numBins)

"""
/*
 * |PothosDoc Inverse Hermetian FFT
 *
 * Compute the inverse FFT of a signal that has Hermitian symmetry, i.e., a
 * real spectrum. Here the signal has Hermitian symmetry in the time domain and
 * is real in the frequency domain.
 *
 * Corresponding NumPy function: numpy.fft.fft
 *
 * |category /NumPy/FFT
 * |keywords fft hfft ihfft inverse hermetian
 * |factory /numpy/fft/ihfft(dtype,numBins)
 *
 * |param dtype(Input Data Type) The block data type.
 * |widget DTypeChooser(float=1)
 * |default "float64"
 * |preview disable
 *
 * |param numBins(Num FFT Bins)
 * |default 1024
 * |option 512
 * |option 1024
 * |option 2048
 * |option 4096
 * |widget ComboBox(editable=true)
 */
"""
def IHFFT(dtype, numBins):
    dtype = Utility.toDType(dtype)

    return FFTClass(
               "/numpy/fft/ihfft",
               numpy.fft.ihfft,
               dtype,
               Utility.dtypeToComplex(dtype),
               dict(supportFloat=True),
               dict(supportComplex=True),
               numBins)
