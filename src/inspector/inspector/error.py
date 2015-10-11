class Error(Exception):
    def __init__(self, msg, error=None):
        super(Error, self).__init__(msg)
        self.error = error
