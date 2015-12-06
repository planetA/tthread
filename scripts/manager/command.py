import os, sys
import pathlib
import resource
import sqlite3

import subprocess as sp
from itertools import product

import tthread
from tthread import run
from tthread.formats import DTLWriter

import racksim

import csv

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
            taskset = self.taskset_cmd(cpus)
            nproc = str(self.nproc(cpus))

            dataset = benchmarks[app]['dataset'][self.args.type]
            dataset = dataset.replace("$NPROCS", nproc).split()
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
            if log.return_code != 0:
                print("process exited with: %d" % log.return_code, file=sys.stderr)

            if not os.path.exists(BM_TRACE):
                os.makedirs(BM_TRACE)
            output = open(os.path.join(BM_TRACE, '%s_%s_%s.dtl' % (app, self.args.type, cpus)), 'w')
            DTLWriter(log).write(output)

class SimCommand(Command):
    def __init__(self, args):
        super(SimCommand, self).__init__(args)
        if not args.dtl:
            self.trace_dir = os.path.join(self.args.dir, 'traces')
            self.file_list = os.listdir(self.trace_dir)
        else:
            self.trace_dir = '.'
            self.file_list = args.dtl

    def __cpustr2list(self, cpustr):
        res = []
        for r in cpustr.split(','):
            b = int(r.split('-')[0])
            e = int(r.split('-')[-1]) + 1
            res.extend(range(b, e))
        return res

    def __call__(self):
        arch = os.path.basename(self.args.dir)

        sim_db = os.path.join(self.args.dir, '../sim.db')
        if not os.path.isfile(sim_db):
            fieldnames = ['arch', 'sched', 'app', 'threads', 'cpulist', 'time']
            conn = sqlite3.connect(sim_db)
            c = conn.cursor()
            c.execute('''CREATE TABLE runtime
                         (arch TEXT, sched TEXT, app TEXT, problem TEXT, threads INTEGER,
                          cpulist TEXT, time REAL, sim INTEGER,
                          CONSTRAINT configuration PRIMARY KEY (arch, sched, app, problem, cpulist, sim))''')
            conn.commit()
            conn.close()
        conn = sqlite3.connect(sim_db)
        c = conn.cursor()

        for (trace_file, mst, sched) in product(self.file_list, self.args.mst, self.args.sched):
            trace_path=os.path.join(self.trace_dir, trace_file)
            (app, problem, cpustr) = trace_file.split('.')[0].rsplit('_', 2)
            end = racksim.RackSim(trace_path, mst, sched).run()
            cpulist = self.__cpustr2list(cpustr)
            print (end)
            if end == float('-Inf'):
                end = -1
                print(mst, sched, arch, trace_path, app, problem, cpustr, cpulist, end)
            c.execute("INSERT INTO runtime VALUES ('%s', '%s', '%s', '%s', %d, '%s', %f, %d)" % \
                      (arch, sched, app, problem, len(cpulist), cpustr, float(end), 1))
        conn.commit()
        conn.close()
