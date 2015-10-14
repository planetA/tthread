class Error(Exception):
    def __init__(self, msg, error=None):
        super(Error, self).__init__(msg)
        self.error = error

from .run import run, default_tthread_path  # flake8: noqa
