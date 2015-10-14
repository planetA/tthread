import os
import multiprocessing as mp

from . import cgroups, perf
from . import tthread


def default_library_path():
    script_dir = os.path.dirname(__file__)
    tthread_dir = os.path.join(script_dir, "..", "..", "libtthread.so")
    return os.path.realpath(tthread_dir)


def run(command,
        tthread_path=default_library_path(),
        perf_command="perf",
        perf_log="perf.data",
        user=None,
        group=None,
        processor_trace=True):

    cgroup_name = "inspector-%d" % os.getpid()
    cgroup = cgroups.PerfEvent(cgroup_name)
    cgroup.create()

    barrier = mp.Barrier(2)
    tthread_cmd = tthread.Command(tthread_path=tthread_path,
                                  user=user,
                                  group=group,
                                  cgroup=cgroup)
    process = mp.Process(target=tthread_cmd.exec,
                         args=(command, barrier,))
    process.start()

    return perf.run(perf_command,
                    perf_log,
                    barrier,
                    process,
                    cgroup,
                    processor_trace=processor_trace)
