import os
import time
from . import Error
from collections import namedtuple


Status = namedtuple("Status", ["exit_code", "perf_exit_code", "duration"])


class Process:
    def __init__(self, perf_process, traced_process, cgroup):
        self.cgroup = cgroup
        self.perf_process = perf_process
        self.traced_process = traced_process
        self.start_time = time.time()

    def _wait(self):
        while True:
            pid, exitcode = os.wait()
            if pid == self.traced_process.pid:
                duration = time.time() - self.start_time
                self.perf_process.terminate()
                perf_exitcode = self.perf_process.wait()
                return Status(exitcode, perf_exitcode, duration)
            elif pid == self.perf_process.pid:
                self.traced_process.terminate()
                raise Error("perf exited prematurally with %d" % exitcode)
            # else ignore other childs

    def wait(self):
        try:
            return self._wait()
        except OSError as e:
            raise Error("Failed to wait for result of processes '%s'" % e)
        finally:
            self.cgroup.destroy()
        raise Error("Program error! should not be reached")
