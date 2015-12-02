import os, sys
import resource

from subprocess import call, Popen, PIPE, DEVNULL
from itertools import product

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

        self.tasksets = []
        if args.c:
            for cpulist in args.c:
                self.tasksets.append(['taskset', '-c', cpulist])

        self.n = args.n

    def __call__(self):
        for (taskset, iteration, app) in product(self.tasksets, range(self.n), self.runs):
            dataset = benchmarks[app]['dataset'][self.args.type]
            # before = resource.getrusage(resource.RUSAGE_CHILDREN)
            # print(before.ru_utime)
            perf = 'perf stat -e cycles'.split()
            tthread = str('env LD_PRELOAD=' +
                           os.path.join(BM_ROOT, 'src/libtthread.so')).split()
            run_param = taskset + perf + tthread + \
                        [os.path.join(BM_APPS, app, app),
                         dataset]
            print(run_param)
            run = Popen(run_param, stderr=PIPE, stdout=DEVNULL)
            run.wait()
            # after = resource.getrusage(resource.RUSAGE_CHILDREN)
            # print(after.ru_utime, ' ', after.ru_utime - before.ru_utime)
            out = run.stderr.readlines()
            for line in out:
                line = str(line)
                if 'time elapsed' in line:
                    time = float(line.strip().split()[1])
                    print('Time elapsed %f' % time)
