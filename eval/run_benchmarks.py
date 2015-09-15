#!/usr/bin/python

import os, sys
import subprocess
import time
import argparse

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
    return subprocess.call(cmd)

def taskset(cmd, threads=16, verbose=True):
    if threads == 16:
        mask = "0-15"
    elif threads == 8:
        mask = "0,2,4,6,8,10,12,14"
    elif threads == 4:
        mask = "0,4,8,12"
    elif threads == 2:
        mask = "0,6"
    else:
        raise "invalid number of threads: %s" % threads
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
    for i in range(6):
        start = time.time()
        taskset(["ninja", name], cores)
        duration = time.time() - start
        times.append(duration)
    return (sum(times) - max(times) - min(times)) / 8

def main():
    parser = argparse.ArgumentParser(description="Run benchmarks.")
    parser.add_argument("output", nargs="?", default=".", help="output directory to write measurements")
    args = parser.parse_args()

    os.chdir(os.path.join(SCRIPT_ROOT, ".."))
    for bench in BENCHMARKS:
        path = os.path.join(args.output, bench + ".csv")
        with open(path, "a+") as f:
            sys.stderr.write(">> run %s\n" % bench)
            for threads in [16,8,4,2]:
                sh(["cmake", "-DCMAKE_BUILD_TYPE=Release", "-DNCORES=%d" % threads, "-DBENCHMARK=On"])
                mean1 = run_benchmark("bench-%s-pthread" % bench, threads)
                mean2 = run_benchmark("bench-%s-tthread" % bench, threads)
                f.write("%d;%f;%f" %(threads, mean1, mean2))

if __name__ == '__main__':
    main()
