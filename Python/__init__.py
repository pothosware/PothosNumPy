# Copyright (c) 2019 Nicholas Corgan
# SPDX-License-Identifier: BSD-3-Clause

from .BlockEntryPoints import *
from .FFT import *
from .Load import *
from .Random import *
from .RegisteredCallHelpers import *
from .Save import *
from .Utility import *

import Pothos
import logging

# logging.captureWarnings() redirects all outputs from the "warnings" module
# to a Python logger named "py.warnings". Adding our log handler to this logger
# results in all NumPy warnings being consumed by our infrastructure.
logging.basicConfig(level=logging.INFO)
logging.getLogger("py.warnings").addHandler(Pothos.LogHandler("py.warnings"))
logging.captureWarnings(True)
