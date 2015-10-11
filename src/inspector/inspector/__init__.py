import os
import subprocess


class Error(EnvironmentError):
    pass


def default_library_path():
    script_dir = os.path.dirname(__file__)
    tthread_dir = os.path.join(script_dir, "..", "..", "libtthread.so")
    return os.path.realpath(tthread_dir)


class Process:
    def __init__(self, popen):
        self.popen = popen

    def wait(self):
        return self.popen.wait()


def preexec_fn():
    pass


def run(command,
        tthread_path=default_library_path(),
        stdin=None,
        stdout=None,
        stderr=None):
    env = os.environ.copy()
    env["LD_PRELOAD"] = tthread_path
    env["TTHREAD_NO_LOG"] = "1"
    env["LD_BIND_NOW"] = "1"
    popen = subprocess.Popen(command,
                             env=env,
                             stdin=stdin,
                             stdout=stdout,
                             stderr=stderr,
                             preexec_fn=preexec_fn)
    return Process(popen)
