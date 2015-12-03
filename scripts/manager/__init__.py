from .constants import BM_ROOT, BM_TTHREAD

try:
    import tthread
except ImportError:
    # put BM_TTHREAD in sys.path, for development purpose
    import sys
    sys.path.append(BM_TTHREAD)
    import tthread

from .command import *

import os

