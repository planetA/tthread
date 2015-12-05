import os, sys
import resource

import subprocess as sp
from itertools import product

import tthread
from tthread import run
from tthread.formats import DTLWriter

from .constants import BM_ROOT, BM_APPS, BM_DATA, BM_TRACE
from .benchmark import benchmarks

class Command:
    def __init__(self, args):
        self.args = args

        if args.verbose:
            self.verbose = True
        else:
            self.verbose = False

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

    def nproc(self, cpus):
        return str(sp.check_output(self.taskset_cmd(cpus) + ['nproc']).decode())

    def __call__(self):
        pass


class CompileBench(Command):
    def __call__(self):
        pid = sp.Popen(['make'], cwd=BM_ROOT)
        pid.wait()

        # Phoenix
        for app in benchmarks:
            continue
            if 'prepare' in benchmarks[app]:
                actions = benchmarks[app]['prepare']
                for action in actions:
                    print(action)
                    if action[0] == 'app':
                        prepare = action[1].split()
                        run_param = [os.path.join(BM_APPS, app, app)] + prepare
                        if self.verbose:
                            print("Prepare: " + " ".join(run_param))
                        run = sp.Popen(run_param, stdout=sp.DEVNULL, stderr=sp.DEVNULL)
                        run.wait()

        if self.verbose:
            print(" ".join([os.path.join(BM_ROOT, 'scripts/prepare.sh'), BM_ROOT]))
        sp.call([os.path.join(BM_ROOT, 'scripts/prepare.sh'), BM_ROOT])

class RunBench(RunCommand):
    def __init__(self, args):
        super(RunBench, self).__init__(args)

        self.n = args.n

    def __call__(self):
        super(RunBench, self).__call__()

        for (cpus, iteration, app) in product(self.cpulist, range(self.n), self.apps):
            taskset = self.taskset_cmd(cpus)
            nproc = str(self.nproc(cpus))

            dataset = benchmarks[app]['dataset'][self.args.type]
            dataset = dataset.replace("$NPROCS", nproc).split()
            # before = resource.getrusage(resource.RUSAGE_CHILDREN)
            # print(before.ru_utime)
            print(dataset)
            perf = 'perf stat -e cycles'.split()
            tthread = str('env LD_PRELOAD=' +
                           os.path.join(BM_ROOT, 'src/libtthread.so') + \
                          ' NPROCS=' + nproc).split()
            run_param = taskset + perf + tthread + \
                        [os.path.join(BM_APPS, app, app)] + dataset
            if self.verbose:
                print(" ".join(run_param))
                run = sp.Popen(run_param, stderr=sp.PIPE)
            else:
                run = sp.Popen(run_param, stderr=sp.PIPE, stdout=sp.DEVNULL)
            run.wait()
            if run.returncode != 0:
                print("Unexpected return code %d for command %s" % (run.returncode, run_param))
            # after = resource.getrusage(resource.RUSAGE_CHILDREN)
            # print(after.ru_utime, ' ', after.ru_utime - before.ru_utime)
            out = run.stderr.readlines()
            for line in out:
                line = str(line)
                if 'time elapsed' in line:
                    time = float(line.strip().split()[1])
                    print('App %s, CPU %s Time elapsed %f' % (app, cpus, time))

class TraceBench(RunCommand):
    def __init__(self, args):
        super(TraceBench, self).__init__(args)

    def __call__(self):
        super(TraceBench, self).__call__()

        for (cpus, app) in product(self.cpulist, self.apps):
            dataset = benchmarks[app]['dataset'][self.args.type].split()
            # before = resource.getrusage(resource.RUSAGE_CHILDREN)
            # print(before.ru_utime)
            tthread_lib = str('env LD_PRELOAD=' +
                              os.path.join(BM_ROOT, 'src/libtthread.so') + \
                              ' NPROCS=' + str(self.nproc(cpus))).split()
            taskset = self.taskset_cmd(cpus)
            run_param = taskset + \
                        tthread_lib + \
                        [os.path.join(BM_APPS, app, app)] + dataset
            if self.verbose:
                print(" ".join(run_param))
                process = tthread.run(run_param, tthread_lib, stderr=sp.PIPE)
            else:
                process = tthread.run(run_param, tthread_lib)
            log = process.wait()

            if not os.path.exists(BM_TRACE):
                os.makedirs(BM_TRACE)
            output = open(os.path.join(BM_TRACE, '%s_%s.dtl' % (app, cpus)), 'w')
            DTLWriter(log).write(output)
