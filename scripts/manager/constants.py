import os

BM_ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '../..'))
BM_APPS = os.path.join(BM_ROOT, 'eval/tests')
BM_DATA = os.path.join(BM_ROOT, 'eval/datasets')
BM_TRACE = os.path.join(BM_ROOT, 'out/traces')
BM_TTHREAD = os.path.join(BM_ROOT, 'src/tthread-python')
