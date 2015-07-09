#!/usr/bin/env python

import sys, os
import argparse
import tthread
import csv, json
import subprocess

from tthread import formats

def abort(msg):
    print(msg, file=sys.stderr)
    sys.exit(1)

formats = {
        "tsv": formats.TsvWriter,
        "tsv2": formats.Tsv2Writer,
        #"json": formats.JsonWriter,
}

supported_formats = ", ".join(formats.keys())

def parse_arguments():
    parser = argparse.ArgumentParser(description="Process some integers.")
    parser.add_argument("--libtthread-path",
            nargs="?",
            default=tthread.default_library_path(),
            help="path to libtthread.so (default: ../../libtthread.so - relative to script path)")
    parser.add_argument("--output", nargs="?",
            help="path to trace file (default: stdout); if no trace file is specified, "
            "stdout of programm is redirected to stderr")
    parser.add_argument("--format", nargs="?",
            default="tsv",
            help="default format to write access log (supported: %s; default: tsv)" % supported_formats)
    parser.add_argument("command", nargs=1,
            help="command to execute with")
    parser.add_argument("arguments", nargs="*",
            default=[],
            help="arguments passed to command")
    return parser.parse_args()

def check_executable(command):
    try:
        subprocess.check_call(command)
        return True
    except subprocess.CalledProcessError:
        return False

def main():
    if sys.version_info < (3,0):
        abort("this script requires Python 3.x, not Python 2.x")
    args = parse_arguments()

    if not os.path.isfile(args.libtthread_path):
        abort("Libthreads not found at '%s', "
                "use --libtthread-path to specify it" % args.libtthread_path)

    formatter = formats.get(args.format, None)
    if formatter == None:
        abort("unsupported format %s, supported formats are %s" % (args.format, supported_formats))

    if args.output == None:
        stdout = sys.stderr
        output = sys.stdout
    else:
        stdout = sys.stdout
        try:
            output = open(args.output)
        except EnvironmentError as e:
            abort("failed to open output '%s'" % e)

    command = args.command + args.arguments
    try:
        print("$ " + " ".join(command))
        process = tthread.run(command, args.libtthread_path, stdout=stdout)
        log = process.wait()
        if log.return_code != 0:
            print("process exited with: %d" % log.return_code, file=sys.stderr)
    except tthread.Error as e:
        abort("Execution fails: %s" % e)
    formatter(log).write(output)
