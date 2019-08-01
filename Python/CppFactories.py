# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .OneToOneBlock import *
from .TwoToOneBlock import *
from . import Utility

import Pothos

import numpy

def Sin(dtype):
    return OneToOneBlock(dtype, dtype, numpy.sin, useDType=False, supportFloat=True)

def BitwiseAnd(dtype):
    return TwoToOneBlock(dtype, dtype, numpy.bitwise_and, useDType=True, supportInt=True, supportUInt=True)
