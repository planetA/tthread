#!/usr/bin/env python

import inspector
import sys
import argparse


def abort(msg):
    print(msg, file=sys.stderr)
    sys.exit(1)


def parse_arguments():
    desc = "Run program with tthread and intel PT"
    parser = argparse.ArgumentParser(description=desc)
    h1 = "path to libtthread.so " \
         "(default: ../../libtthread.so - relative to script path)"
    parser.add_argument("--libtthread-path",
                        default=inspector.default_library_path(),
                        help=h1)
    parser.add_argument("--perf-path",
                        default="perf",
                        help="Path to perf tool")
    parser.add_argument("command", nargs=1,
                        help="command to execute with")
    parser.add_argument("arguments", nargs="*",
                        default=[],
                        help="arguments passed to command")
    return parser.parse_args()


def main():
    if sys.version_info < (3, 0):
        abort("this script requires Python 3.x, not Python 2.x")
    args = parse_arguments()
    command = args.command + args.arguments
    process = inspector.run(command,
                            args.libtthread_path,
                            perf_cmd=args.perf_path)
    process.wait()
