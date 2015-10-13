import os
import pwd
import grp
import time
import subprocess
import sys
import multiprocessing as mp
from threading import BrokenBarrierError

from . import cgroups, Error, perf


def drop_privileges(user, group):
    if os.getuid() != 0:
        raise Error("Must run as root to drop priviliges")

    if type(group) is int:
        os.setgid(group)
    elif group is not None:
        os.setgid(grp.getgrnam(group).gr_gid)

    # Remove group privileges
    if user is not None or group is not None:
        os.setgroups([])

    if type(user) is int:
        os.setuid(user)
    elif user is not None:
        os.setuid(pwd.getpwnam(user).pw_uid)


def default_library_path():
    script_dir = os.path.dirname(__file__)
    tthread_dir = os.path.join(script_dir, "..", "..", "libtthread.so")
    return os.path.realpath(tthread_dir)


def _exec_program(barrier, command, tthread_path, user, group, cgroup):
    env = os.environ.copy()
    env["LD_PRELOAD"] = tthread_path
    env["TTHREAD_NO_LOG"] = "1"
    env["LD_BIND_NOW"] = "1"
    try:
        barrier.wait(timeout=3)
    except BrokenBarrierError:
        print("Parent process timed out", file=sys.stderr)
    cgroup.addPids(os.getpid())
    drop_privileges(user, group)
    os.execlpe(*command, env)


def _run_perf(barrier, perf_command, perf_log, process, cgroup):
    command = [perf_command,
               "record",
               "--quiet",
               "--output", perf_log,
               "--event", "major-faults,intel_pt/tsc=1/u",
               "--cgroup", cgroup.name,
               "--all-cpus", ]
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
    return perf.Process(perf_process, process, cgroup)


def run(command,
        tthread_path=default_library_path(),
        perf_command="perf",
        perf_log="perf.data",
        user=None,
        group=None):
    cgroup_name = "inspector-%d" % os.getpid()
    cgroup = cgroups.PerfEvent(cgroup_name)
    cgroup.create()

    barrier = mp.Barrier(2)
    proc_args = (barrier, command, tthread_path, user, group, cgroup)
    process = mp.Process(target=_exec_program,
                         args=proc_args)
    process.start()
    return _run_perf(barrier, perf_command, perf_log, process, cgroup)
