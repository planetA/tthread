import os, sys
import resource
from subprocess import call, Popen, PIPE

from .constants import BM_ROOT, BM_APPS, BM_DATA
from .benchmark import benchmarks

class Command:
    def __init__(self, args):
        self.args = args

class CompileBench(Command):
    def __call__(self):
        pid = Popen(['make'], cwd=BM_ROOT)
        pid.wait()

class RunBench(Command):
    def __init__(self, args):
        super(RunBench, self).__init__(args)
        self.runs = []
        if args.apps:
            for app in args.apps:
                if app in benchmarks:
                    self.runs.append(app)
        else:
            for app in benchmarks:
                self.runs.append(app)

    def __call__(self):
        for app in self.runs:
            dataset = benchmarks[app]['dataset'][self.args.type]
            # before = resource.getrusage(resource.RUSAGE_CHILDREN)
            # print(before.ru_utime)
            taskset = 'taskset -c 0-1'.split()
            perf = 'perf stat -e cycles'.split()
            tthread = str('env LD_PRELOAD=' +
                           os.path.join(BM_ROOT, 'src/libtthread.so')).split()
            run = Popen(taskset + perf + tthread +
                        [os.path.join(BM_APPS, app, app),
                         dataset], stderr=PIPE)
            run.wait()
            # after = resource.getrusage(resource.RUSAGE_CHILDREN)
            # print(after.ru_utime, ' ', after.ru_utime - before.ru_utime)
            out = run.stderr.readlines()
            for line in out:
                line = str(line)
                if 'time elapsed' in line:
                    time = float(line.strip().split()[1])
                    print('Time elapsed %f' % time)
        pass
