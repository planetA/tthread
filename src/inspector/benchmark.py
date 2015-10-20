import os
import sys
import glob
import argparse
import json
import subprocess
import inspector
if sys.version_info >= (3, 3):
    from shlex import quote
else:
    from pipes import quote

SCRIPT_ROOT = os.path.dirname(os.path.realpath(__file__))
EVAL_ROOT = os.path.realpath(os.path.join(SCRIPT_ROOT, "../../eval"))
TOTAL_THREADS = len(glob.glob("/sys/devices/system/cpu/cpu*"))
TEST_PATH = os.path.join(EVAL_ROOT, "tests")
DATASET_HOME = os.path.join(EVAL_ROOT, "datasets")


class NCores:
    def to_param(self, cores):
        return cores


class CannealThreads(NCores):
    def to_param(self, cores):
        if cores == 8:
            return 7
        elif cores == 4:
            return 3
        else:  # == 2
            return 1


class DedupThreads(NCores):
    def to_param(self, cores):
        if cores == 8:
            return 2
        else:  # cores == 4 or cores == 2
            return 1


def set_online_cpus(threads=TOTAL_THREADS, verbose=True):
    for i in list(range(1, TOTAL_THREADS - 1)):
        enable = (i % int(TOTAL_THREADS / threads)) == 0
        with open("/sys/devices/system/cpu/cpu%d/online" % i, "w") as f:
            if enable:
                f.write("1\n")
            else:
                f.write("0\n")


def sh(cmd, verbose=True):
    if verbose:
        args = ' '.join(map(lambda s: quote(s), cmd[1:]))
        sys.stderr.write("$ %s %s\n" % (cmd[0], args))
    return subprocess.call(cmd)


def test_path(subdir):
    return os.path.join(TEST_PATH, subdir)


def dataset_home(subdir):
    return os.path.join(DATASET_HOME, subdir)


class Benchmark():
    def __init__(self, name, args, command=None):
        self.name = name
        self.args = args
        if command is None:
            self.command = name
        else:
            self.command = command
        self.perf_command = "perf"

    def run(self, cores, perf_log, with_pt, with_tthread):
        def format_arg(arg):
            if issubclass(type(arg), NCores):
                return str(arg.to_param(cores))
            return str(arg)
        args = list(map(format_arg, self.args))
        os.chdir(test_path(self.name))
        cmd = ["./" + self.command] + args
        durations = []
        if with_tthread:
            libtthread = inspector.default_tthread_path()
        else:
            libtthread = None
        for c in cmd:
            assert type(c) is not None
        for i in range(6):
            print("$ " + " ".join(cmd)
                  + (" pt" if with_pt else "")
                  + (" tthread" if with_tthread else ""))
            if os.path.exists(perf_log):
                os.remove(perf_log)
            proc = inspector.run(cmd,
                                 perf_command=self.perf_command,
                                 processor_trace=with_pt,
                                 tthread_path=libtthread,
                                 perf_log=perf_log)
            status = proc.wait()
            if status.exit_code != 0:
                raise OSError("command: %s\nfailed with: %d" %
                              (" ".join(cmd), status.exit_code))
            durations.append(status.duration)
        return (sum(durations) - max(durations) - min(durations)) / 8

benchmarks = [
    Benchmark("canneal",
              [CannealThreads(),
               15000,
               2000,
               test_path("canneal/400000.nets"),
               128]),
    Benchmark("dedup",
              ["-c",
               "-p",
               "-t", DedupThreads,
               "-i", test_path("vips/FC-6-x86_64-disc1.iso"),
               "-o", "output.dat.ddp"]),
    Benchmark("ferret",
              [test_path("ferret/corel"),
               "lsh", test_path("ferret/queries"),
               10,
               20,
               1,
               "output.txt"]),
    Benchmark("swaptions",
              ["-ns", 128,
               "-sm", 50000,
               "-nt", NCores()]),
    Benchmark("streamcluster",
              [10,
               20,
               128,
               16384,
               16384,
               1000,
               "none",
               "output.txt",
               NCores()]),
    Benchmark("vips",
              ["im_benchmark",
               test_path("vips/orion_18000x18000.v"),
               "output.v"]),
    Benchmark("raytrace",
              [test_path("raytrace/thai_statue.obj"),
               "-automove",
               "-nthreads",
               NCores(),
               "-frames 200",
               "-res 1920 1080"],
              command="rtview"),
    Benchmark("histogram", [dataset_home("histogram_datafiles/large.bmp")]),
    Benchmark("linear_regression",
              [dataset_home("linear_regression_datafiles/"
                            "key_file_500MB.txt")]),
    Benchmark("reverse_index", [dataset_home("reverse_index_datafiles")]),
    Benchmark("string_match",
              [dataset_home("string_match_datafiles/key_file_500MB.txt")]),
    Benchmark("word_count",
              [dataset_home("word_count_datafiles/word_100MB.txt")]),
    Benchmark("kmeans", ["-d", 3, "-c", 1000, "-p", 100000, "-s", 1000]),
    Benchmark("matrix_multiply", [2000, 2000]),
    Benchmark("pca", ["-r", 4000, "-c", 4000, "-s", 100])
]


def parse_args():
    parser = argparse.ArgumentParser(description="Run benchmarks.")
    parser.add_argument("--perf-command",
                        default="perf",
                        help="Path to perf tool")
    parser.add_argument("--perf-log",
                        default="perf.data",
                        help="Path to perf log")
    parser.add_argument("output",
                        default=".",
                        help="output directory to write measurements")
    return parser.parse_args()


def main():
    args = parse_args()
    output = os.path.realpath(args.output)
    perf_log = os.path.realpath(args.perf_log)

    if "/" in args.perf_command:
        # resolve relatives command paths
        perf_command = os.path.realpath(args.perf_command)
    else:
        perf_command = args.perf_command

    os.chdir(os.path.join(SCRIPT_ROOT, "../.."))

    sh(["cmake", "-DCMAKE_BUILD_TYPE=Release", "-DBENCHMARK=On"])
    sh(["cmake", "--build", "."])
    sh(["cmake", "--build", ".", "--target", "build-parsec"])
    sh(["cmake", "--build", ".", "--target", "build-phoenix"])

    path = os.path.join(output, "log.json")

    if os.path.exists(path):
        log = json.load(open(path))
    else:
        log = {}

    for threads in [16, 8, 4, 2]:
        os.environ["IM_CONCURRENCY"] = str(threads)
        set_online_cpus(threads)
        for bench in benchmarks:
            run_name = "%s-%d" % (bench.name, threads)
            if run_name in log:
                print(">> skip %s" % run_name)
                continue
            bench.perf_command = perf_command
            try:
                sys.stderr.write(">> run %s\n" % bench.name)

                def run(pt, tthread):
                    return bench.run(threads, perf_log, pt, tthread)

                both    = run(True, True)
                pt      = run(True, False)
                tthread = run(False, True)
                pthread = run(False, False)

                row = (threads, pthread, tthread, pt, both)
                log[run_name] = row
                with open(path, "w") as f:
                    json.dump(log, f)
            except OSError as e:
                print("failed to run %s: %s" % (bench.name, e))

if __name__ == '__main__':
    main()
