# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *
from .OneToOneBlock import *
from .Source import *
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
    except:
        rand = numpy.random

    return rand

def GetNumPyRandomIntegersFunc():
    randints = None
    try:
        rand = numpy.random.default_rng().integers
    except:
        rand = numpy.random.random_integers

    return rand

#
# Blocks whose invocations or implementations are different enough to not auto-generate
#

def Integers(dtype):
    outputArgs = dict(supportInt=True, supportUInt=True)
    return SingleOutputSource(GetNumPyRandomIntegersFunc(), dtype, outputArgs, useDType=False)
