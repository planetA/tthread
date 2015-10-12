import os
from inspector import Error


class Process:
    def __init__(self, perf_process, traced_process, cgroup):
        self.cgroup = cgroup
        self.perf_process = perf_process
        self.traced_process = traced_process

    def wait(self):
        try:
            while True:
                pid, status = os.wait()
                if pid == self.traced_process.pid:
                    self.perf_process.terminate()
                    perf_status = self.perf_process.wait()
                    return (status, perf_status)
                elif pid == self.perf_process.pid:
                    self.traced_process.terminate()
                    raise Error("perf exited prematurally with %d" % status)
                # else ignore other childs
        except OSError as e:
            raise Error("Failed to wait for result of processes '%s'" % e)
        finally:
            self.cgroup.destroy()
        return status
