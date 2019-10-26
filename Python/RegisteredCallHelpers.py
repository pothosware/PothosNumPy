# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from . import Utility

import Pothos

import json
import numpy
import os

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
    topLevel["NumPy Info"] = dict()
    topLevel["NumPy Info"]["Version"] = numpy.__version__
    topLevel["NumPy Info"]["Install Directory"] = os.path.dirname(os.path.dirname(numpy.__file__))

    for key in KEYS:
        info = numpy.__config__.get_info(key)
        if info:
            for k,v in info.items():
                if type(v) is list:
                    if type(v[0]) is tuple:
                        infoLists = [list(t) for t in v]
                        infoLists = [[str(u) for u in l if u is not None] for l in infoLists]
                        infoLists = ["=".join(u) for u in infoLists]
                        info[k] = ",".join(list(set(infoLists)))
                    elif type(v[0]) is str:
                        info[k] = ",".join(list(set(v)))
            topLevel["NumPy Info"][key] = info

    return json.dumps(topLevel)

def getNumPyIntInfoFromPothosDType(pothosDType):
    Utility.validateDType(pothosDType, dict(supportAll=True))

    return numpy.iinfo(Pothos.Buffer.dtype_to_numpy(pothosDType))

def getNumPyFloatInfoFromPothosDType(pothosDType):
    Utility.validateDType(pothosDType, dict(supportAll=True))

    return numpy.finfo(Pothos.Buffer.dtype_to_numpy(pothosDType))
