# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

import Pothos

import numpy

import os

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
