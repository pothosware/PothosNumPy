# Copyright (c) 2020 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

import Pothos
from . import Random

import numpy

import os

#
# Checking inputs
#

def checkArrayContents(expectedValues, actualValues):
    if type(expectedValues) is not numpy.ndarray:
        raise RuntimeError("expectedValues must be a NumPy array.")
    if type(actualValues) is not numpy.ndarray:
        raise RuntimeError("actualValues must be a NumPy array.")

    if expectedValues.dtype != actualValues.dtype:
        raise RuntimeError("Expected dtype {0}. Actual dtype {1}".format(expectedValues.dtype, actualValues.dtype))

    if len(expectedValues) != len(actualValues):
        raise RuntimeError("Expected length {0}. Actual length {1}".format(len(expectedValues), len(actualValues)))

    if (expectedValues != actualValues).any():
        raise RuntimeError("Contents do not match.")

def checkNpyContents(filepath, expectedValues):
    if not os.path.exists(filepath):
        raise RuntimeError("Invalid filepath: {0}".format(filepath))

    npyContents = numpy.load(filepath)
    checkArrayContents(expectedValues, npyContents)

def checkNpzContents(filepath, expectedValues):
    if not os.path.exists(filepath):
        raise RuntimeError("Invalid filepath: {0}".format(filepath))

    npzContents = numpy.load(filepath)

    expectedKeys = list(expectedValues.keys())
    expectedKeys.sort() # In place

    actualKeys = npzContents.keys()
    actualKeys.sort() # In place

    if expectedKeys != actualKeys:
        raise RuntimeError("Keys don't match.\nExpected: {0}\nActual: {1}".format(expectedKeys, actualKeys))

    for key in expectedKeys:
        checkArrayContents(expectedValues[key], npzContents[key])

#
# Generating outputs
#

def generate1DRandomValues(dtype, arrLength):
    if type(dtype) is not numpy.dtype:
        dtype = Pothos.Buffer.dtype_to_numpy(dtype)

    def randFloats(size):
        return Random.NumPyRandom.exponential(1.0, size)

    # This calls into a compatibility layer that abstracts the differences in
    # numpy.random class type between versions.
    if "int" in str(dtype):
        vals = Random.integers(0, 100, arrLength).astype(dtype)
    elif str(dtype) == "complex64":
        vals = randFloats(arrLength*2).astype("float32").view(dtype)
    elif str(dtype) == "complex128":
        vals = randFloats(arrLength*2).astype("float64").view(dtype)
    else:
        vals = randFloats(arrLength).astype(dtype)

    return vals

def generate2DRandomValues(dtype, nchans, arrLength):
    if type(dtype) is not numpy.dtype:
        dtype = Pothos.Buffer.dtype_to_numpy(dtype)

    vals = numpy.ndarray(shape=(nchans, arrLength), dtype=dtype)
    for chan in range(nchans):
        vals[chan] = generate1DRandomValues(dtype, arrLength)

    return vals

def generate1DNpyFile(filepath, dtype):
    values = generate1DRandomValues(dtype, 256)
    numpy.save(filepath, values)

    # Return values for validation
    return values

def generate2DNpyFile(filepath, dtype):
    values = generate2DRandomValues(dtype, 4, 256)
    numpy.save(filepath, values)

    # Return values for validation
    return values

def generateNpzFile(filepath, compressed):
    values = dict()
    keys = [
        "int8", "int16", "int32", "int64",
        "uint8", "uint16", "uint32", "uint64",
        "float32", "float64", "complex64", "complex128"
    ]
    for key in keys:
        values[key] = generate1DRandomValues(numpy.dtype(key), 256)

    if compressed:
        numpy.savez_compressed(filepath, **values)
    else:
        numpy.savez(filepath, **values)

    # Return values for validation
    return values
