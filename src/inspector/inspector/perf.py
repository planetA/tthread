import os
import time
import subprocess
from . import Error
from collections import namedtuple
from threading import BrokenBarrierError

Status = namedtuple("Status", ["exit_code", "perf_exit_code", "duration"])


def run(perf_command,
        perf_log,
        barrier,
        process,
        cgroup,
        processor_trace=True):
    command = [perf_command,
               "record",
               "--all-cpus",
               "--output", perf_log,
               "--event", "major-faults",
               "--cgroup", cgroup.name]
    if processor_trace:
        command += ["--event", "intel_pt/tsc=1/u",
                    "--cgroup", cgroup.name]
    # print("$ " + " ".join(command))
    perf_process = subprocess.Popen(command)
    for i in range(5):
        if perf_process.poll() is not None:
            raise Error("Failed to start perf")
        # ugly hack, but there is no mechanims to ensure perf is ready
        time.sleep(0.1)
    try:
        barrier.wait(timeout=3)
    except BrokenBarrierError:
        raise Error("Child process timed out")
    return Process(perf_process, process, cgroup)


class Process:
    def __init__(self, perf_process, traced_process, cgroup):
        self.perf_process = perf_process
        self.traced_process = traced_process
        self.cgroup = cgroup
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
