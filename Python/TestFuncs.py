# Copyright (c) 2019 Nicholas Corgan
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
        raise RuntimeError("Expected dtype {0}. Actual dtype {1}".format(expectedNumPyDType, actualValues.dtype))

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

def generateRandomValues(dtype):
    if type(dtype) is not numpy.dtype:
        dtype = Pothos.Buffer.dtype_to_numpy(dtype)

    arrLength = 256
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

def generateNpyFile(filepath, dtype):
    values = generateRandomValues(dtype)
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
        values[key] = generateRandomValues(numpy.dtype(key))

    if compressed:
        numpy.savez_compressed(filepath, **values)
    else:
        numpy.savez(filepath, **values)

    # Return values for validation
    return values
