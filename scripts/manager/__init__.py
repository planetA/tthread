from .constants import BM_ROOT, BM_TTHREAD, BM_RACKSIM

try:
    import tthread
except ImportError:
    # put BM_TTHREAD in sys.path, for development purpose
    import sys
    sys.path.append(BM_TTHREAD)
    import tthread

try:
    import racksim
except ImportError:
    # put BM_TTHREAD in sys.path, for development purpose
    import sys
    sys.path.append(BM_RACKSIM)
    import racksim

from .command import *

import os

