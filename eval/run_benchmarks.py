#!/usr/bin/python

import os, sys
import subprocess
import time
import re

if sys.version_info >= (3,3):
    import shlex
    quote = shlex.quote
else:
    import pipes
    quote = pipes.quote

SCRIPT_ROOT=os.path.dirname(os.path.realpath(__file__))

def sh(cmd, verbose=True):
    if verbose:
        args = ' '.join(map(lambda s: quote(s), cmd[1:]))
        sys.stderr.write("$ %s %s\n" % (cmd[0], args))
    return subprocess.check_output(cmd)

def taskset(cmd, cores=8, verbose=True):
    if cores == 8:
        mask = "0-15"
    elif cores == 4:
        mask = "0-7"
    elif cores == 2:
        mask = "0-3"
    else:
        raise "invalid number of cores: %s" % cores
    return sh(["taskset", "--cpu-list", mask] + cmd)

BENCHMARKS = [
        "kmeans",
        "canneal",
        "swaptions",
        "histogram",
        "linear_regression",
        "pca",
        "string_match",
        "word_count",
        "reverse_index",
        "blackscholes",
        "dedup",
        "matrix_multiply",
        "streamcluster",
        #"vips",
        #"raytrace",
        #"ferret",
        ]

def run_benchmark(name, cores):
    times = []
    for i in range(10):
        out = taskset(["ninja", name], cores)
        m = re.search('([0-9.]+) s. \(clock\)', out)
        if m == None:
            sys.stderr.write("Command does not contain timing information\n")
            sys.exit(1)
        times.append(float(m.group(1)))
    return (sum(times) - max(times) - min(times)) / 8

def main():
    os.chdir(os.path.join(SCRIPT_ROOT, ".."))
    for bench in BENCHMARKS:
        sys.stderr.write(">> run %s\n" % bench)
        for cores in [2,4,8]:
            sh(["cmake", "-DCMAKE_BUILD_TYPE=Release", "-DNCORES=%d" % cores, "-DBENCHMARK=On"])
            for lib in ["pthread", "tthread"]:
                mean = run_benchmark("bench-%s-%s" % (bench, lib), cores)
                print("%s,%s,%d,%d" %(bench, lib, cores, mean))

if __name__ == '__main__':
    main()
