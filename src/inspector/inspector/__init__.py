class Error(Exception):
    def __init__(self, msg, error=None):
        super(Error, self).__init__(msg)
        self.error = error

from .run import run  # flake8: noqa
run = run
