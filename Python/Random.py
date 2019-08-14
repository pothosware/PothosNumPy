# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *
from . import Utility

import Pothos

import numpy

# Thin compatibility layer over NumPy's change from a module to a class.
# The actual type being returned will differ between NumPy versions, but
# the functions will be the same off of this.
def GetNumPyRandom():
    rand = None
    try:
        rand = numpy.random.default_rng()
    else:
        rand = numpy.random

    return rand

def GetNumPyRandomIntegersFunc():
    randints = None
    try:
        rand = numpy.random.default_rng().integers
    else:
        rand = numpy.random.random_integers

    return rand

#
# Blocks whose invocations ore implementations are different enough to not auto-generate
#

# TODO: postBuffer
class Permutation(OneToOneBlock):
    def __init__(self, dtype):
        dtypeArgs = dict(supportAll=True)
        OneToOneBlock.__init__(self, GetNumPyRandom().permutation, dtype, dtype, dtypeArgs, dtypeArgs)

def Integers(dtype):
    outputArgs = dict(supportInt=True)

    return SingleOutputSource(GetNumPyRandomIntegersFunc(), dtype, outputArgs, useDType=True)
