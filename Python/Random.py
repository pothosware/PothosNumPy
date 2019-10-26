# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BaseBlock import *
from .OneToOneBlock import *
from .Source import *
from . import Utility

import Pothos

import numpy

# Thin compatibility layer over NumPy's change from a module to a class.
# Once this is set, the child functions will be the same.
if numpy.__version__ >= "1.17.0":
    NumPyRandom = numpy.random.default_rng()
    integers = NumPyRandom.integers
else:
    NumPyRandom = numpy.random
    integers = NumPyRandom.randint
