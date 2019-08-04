# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from . import Utility

import Pothos

import json
import numpy

def getNumPyConfigInfoJSONString():
    KEYS = [
        "blas_mkl_info",
        "blis_info",
        "openblas_info",
        "atlas_3_10_blas_threads_info",
        "atlas_3_10_blas_info",
        "blas_info",
        "blas_opt_info",
        "lapack_mkl_info",
        "openblas_lapack_info",
        "atlas_3_10_threads_info",
        "atlas_info",
        "lapack_info",
        "lapack_opt_info"
    ]

    topLevel = dict()
    for key in KEYS:
        topLevel[key] = numpy.__config__.get_info(key)

    return json.dumps(topLevel)

def getNumPyIntInfoFromPothosDType(pothosDType):
    Utility.validateDType(pothosDType, dict(supportAll=True))

    return numpy.iinfo(Pothos.Buffer.dtype_to_numpy(pothosDType))

def getNumPyFloatInfoFromPothosDType(pothosDType):
    Utility.validateDType(pothosDType, dict(supportAll=True))

    return numpy.finfo(Pothos.Buffer.dtype_to_numpy(pothosDType))
