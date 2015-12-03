import os, sys
import resource

from subprocess import call, Popen, PIPE, DEVNULL
from itertools import product

import tthread
from tthread import run
from tthread.formats import DTLWriter

from .constants import BM_ROOT, BM_APPS, BM_DATA, BM_TRACE
from .benchmark import benchmarks

class Command:
    def __init__(self, args):
        self.args = args

class RunCommand(Command):
    def __init__(self, args):
        super(RunCommand, self).__init__(args)
        self.apps = []
        if args.apps:
            for app in args.apps:
                if app in benchmarks:
                    self.apps.append(app)
        else:
            for app in benchmarks:
                self.apps.append(app)

        if args.c:
            self.cpulist = args.c
        else:
            self.cpulist = [None]

    def taskset_cmd(self, cpus):
        if cpus:
            return ['taskset', '-c', cpus]
        else:
            return []

    def __call__(self):
        for app in self.apps:
            if 'prepare' in benchmarks[app]:
                prepare = benchmarks[app]['prepare'][self.args.type].split()
                run_param = [os.path.join(BM_APPS, app, app)] + prepare
                run = Popen(run_param, stdout=DEVNULL, stderr=DEVNULL)
                run.wait()


class CompileBench(Command):
    def __call__(self):
        pid = Popen(['make'], cwd=BM_ROOT)
        pid.wait()

class RunBench(RunCommand):
    def __init__(self, args):
        super(RunBench, self).__init__(args)

        self.n = args.n

    def __call__(self):
        super(RunBench, self).__call__()

        for (cpus, iteration, app) in product(self.cpulist, range(self.n), self.apps):
            taskset = self.taskset_cmd(cpus)
            dataset = benchmarks[app]['dataset'][self.args.type].split()
            # before = resource.getrusage(resource.RUSAGE_CHILDREN)
            # print(before.ru_utime)
            perf = 'perf stat -e cycles'.split()
            tthread = str('env LD_PRELOAD=' +
                           os.path.join(BM_ROOT, 'src/libtthread.so')).split()
            run_param = taskset + perf + tthread + \
                        [os.path.join(BM_APPS, app, app)] + dataset
            run = Popen(run_param, stderr=PIPE# , stdout=DEVNULL
            )
            run.wait()
            # after = resource.getrusage(resource.RUSAGE_CHILDREN)
            # print(after.ru_utime, ' ', after.ru_utime - before.ru_utime)
            out = run.stderr.readlines()
            print(out)
            for line in out:
                line = str(line)
                if 'time elapsed' in line:
                    time = float(line.strip().split()[1])
                    print('Time elapsed %f' % time)

class TraceBench(RunCommand):
    def __init__(self, args):
        super(TraceBench, self).__init__(args)

    def __call__(self):
        super(RunBench, self).__call__()

        for (cpus, app) in product(self.cpulist, self.apps):
            taskset = self.taskset_cmd(cpus)
            output = open(os.path.join(BM_TRACE, '%s_%s.dtl' % (app, cpus)), 'w')
            print(output)
            dataset = benchmarks[app]['dataset'][self.args.type].split()
            # before = resource.getrusage(resource.RUSAGE_CHILDREN)
            # print(before.ru_utime)
            perf = 'perf stat -e cycles'.split()
            tthread_lib = str('env LD_PRELOAD=' +
                           os.path.join(BM_ROOT, 'src/libtthread.so')).split()
            # run_param = taskset + perf +
            run_param = taskset + \
                        tthread_lib + \
                        [os.path.join(BM_APPS, app, app)] + dataset
            print(run_param)
            process = tthread.run(run_param, tthread_lib)
            log = process.wait()
            DTLWriter(log).write(output)
            continue
            # after = resource.getrusage(resource.RUSAGE_CHILDREN)
            # print(after.ru_utime, ' ', after.ru_utime - before.ru_utime)
            out = run.stderr.readlines()
            for line in out:
                line = str(line)
                if 'time elapsed' in line:
                    time = float(line.strip().split()[1])
                    print('Time elapsed %f' % time)
