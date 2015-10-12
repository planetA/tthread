import os
import sys
import time
import subprocess
import multiprocessing as mp
from threading import BrokenBarrierError

from inspector import cgroups
from inspector import perf
from inspector.error import Error


def default_library_path():
    script_dir = os.path.dirname(__file__)
    tthread_dir = os.path.join(script_dir, "..", "..", "libtthread.so")
    return os.path.realpath(tthread_dir)


def _exec_program(barrier, command, tthread_path):
    env = os.environ.copy()
    env["LD_PRELOAD"] = tthread_path
    env["TTHREAD_NO_LOG"] = "1"
    env["LD_BIND_NOW"] = "1"
    try:
        barrier.wait(timeout=3)
    except BrokenBarrierError:
        print("Parent process timed out", file=sys.stderr)
    os.execlpe(*command, env)


def _run_perf(barrier, perf_cmd, process, cgroup):
    cgroup.addPids(process.pid)
    command = [perf_cmd,
               "record",
               "-e", "major-faults,intel_pt/tsc=1/u",
               "-G", cgroup.name,
               "-a", ]
    perf_process = subprocess.Popen(command)

    for i in range(30):
        if perf_process.poll() is None \
           or os.path.exists("perf.data"):
            break
        time.sleep(0.1)  # 30 * 0.1 = 3 sec
    try:
        barrier.wait(timeout=3)
    except BrokenBarrierError:
        raise Error("Child process timed out")
    return perf.Process(perf_process, process, cgroup)


def run(command,
        tthread_path=default_library_path(),
        perf_cmd="perf"):
    cgroup_name = "inspector-%d" % os.getpid()
    cgroup = cgroups.PerfEvent(cgroup_name)
    cgroup.create()

    barrier = mp.Barrier(2)
    process = mp.Process(target=_exec_program,
                         args=(barrier, command, tthread_path))
    process.start()
    return _run_perf(barrier, perf_cmd, process, cgroup)
