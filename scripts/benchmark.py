#!/usr/bin/env python3

from argparse import ArgumentParser
from sys import exit

import manager

# Goals:
#  1. Gather page traces
#  2. Do measurements with dthreads
#  3. Do measurements with tthreads
#  4. Gather topology information
#  5. Run simulations with the gathered traces
#  6. Complie benchmarks
#  7. Compile library
#  8. Gather runtime traces

# Points to implement:
# 1. Run with slurm
# 2. Affinity
# 3. Only user level
# 4. Run also without slurm


def parse_args():
    parser = ArgumentParser()
    parser.set_defaults(comm=None)
    # parser.add_argument('command',
    #                     help='Command to execute',
    #                     choices=['page-trace', 'run-trace',
    #                              'compile-lib', 'compile-bench',
    #                              'bench-tthread', 'bench-dthread',
    #                              'sim', 'topo'])
    commands = parser.add_subparsers(help = 'Choose mode of operation')

    trace = commands.add_parser('trace')

    make = commands.add_parser('make', help='Compile benchmarks and library')
    make.set_defaults(comm=manager.CompileBench)

    run = commands.add_parser('run')
    run.add_argument('apps', nargs='*')
    run.add_argument('--type', help='Type of execution',
                     choices=['test', 'real'], default='test')
    run.set_defaults(comm=manager.RunBench)

    sim = commands.add_parser('sim')
    topo = commands.add_parser('topo')
    args = parser.parse_args()

    if args.comm is None:
        parser.print_help()
        return None
    else:
        print(args.comm)
        return args.comm(args)

    mode = None
    if args.command == 'page-trace':
        print(args.command)
    elif args.command == 'run-trace':
        print(args.command)
    elif args.command == 'compile-lib':
        print(args.command)
    elif args.command == 'compile-bench':
        mode = manager.CompileBench()
        print(args.command)
    elif args.command == 'bench-tthread':
        print(args.command)
    elif args.command == 'bench-dthread':
        print(args.command)
    elif args.command == 'sim':
        print(args.command)
    elif args.command == 'topo':
        print(args.command)
    # return (mode, args)

def main():
    mode = parse_args()
    if mode:
        mode()

if __name__ == '__main__':
    main()
