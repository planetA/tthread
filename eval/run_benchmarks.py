#!/usr/bin/python

import os, sys
import subprocess
import time
import argparse
import glob, threading

if sys.version_info >= (3,3):
    import shlex
    quote = shlex.quote
else:
    import pipes
    quote = pipes.quote

def sh(cmd, verbose=True):
    if verbose:
        args = ' '.join(map(lambda s: quote(s), cmd[1:]))
        sys.stderr.write("$ %s %s\n" % (cmd[0], args))
    return subprocess.call(cmd)

SCRIPT_ROOT = os.path.dirname(os.path.realpath(__file__))
TOTAL_THREADS = len(glob.glob("/sys/devices/system/cpu/cpu*"))

def set_cpus(threads=TOTAL_THREADS, verbose=True):
    for i in list(range(1, TOTAL_THREADS - 1)):
        enable = (i % (TOTAL_THREADS / threads)) == 0
        with open("/sys/devices/system/cpu/cpu%d/online" % i, "w") as f:
            if enable:
                f.write("1")
            else:
                f.write("0")

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
    set_cpus(cores)
    for i in range(6):
        start = time.time()
        sh(["ninja", name])
        duration = time.time() - start
        times.append(duration)
    return (sum(times) - max(times) - min(times)) / 8

def main():
    parser = argparse.ArgumentParser(description="Run benchmarks.")
    parser.add_argument("output", nargs="?", default=".", help="output directory to write measurements")
    args = parser.parse_args()

    os.chdir(os.path.join(SCRIPT_ROOT, ".."))
    for threads in [16,8,4,2]:
        for bench in BENCHMARKS:
            try:
                path = os.path.join(args.output, bench + ".csv")
                with open(path, "a+") as f:
                    sys.stderr.write(">> run %s\n" % bench)
                    sh(["cmake", "-DCMAKE_BUILD_TYPE=Release", "-DNCORES=%d" % threads, "-DBENCHMARK=On"])
                    mean1 = run_benchmark("bench-%s-pthread" % bench, threads)
                    mean2 = run_benchmark("bench-%s-tthread" % bench, threads)
                    f.write("%d;%f;%f\n" %(threads, mean1, mean2))
                    f.flush()
            except OSError as e:
                print("failed to run %s: %s" %(bench,e))

if __name__ == '__main__':
    main()
