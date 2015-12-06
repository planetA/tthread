from optparse import OptionParser
from sys import exit
import csv
import json

from .programm import Programm
from .architecture import Architecture
from .execution import Execution
from .scheduler import FirstTouch, MagicTouch, RandomTouch, Scheduler
from .scheduler import NoMoveCommit, MoveHereCommit

class RackSim:
    def __parse_dtl(self, dtl_filename):
        with open(dtl_filename) as dtl_file:
            dtl_reader = csv.reader(dtl_file, delimiter='\t')
            c = 10

            self.prog = Programm()
            for row in dtl_reader:
                if row[0] == "thunk":
                    self.prog.add_thunk(row[1],row[2],row[3])
                elif row[0] == "finish":
                    self.prog.add_finish(row[1], row[2])
                elif row[0] == "end":
                    self.prog.add_end(row[1], row[2], row[3])
                elif row[0] == "read":
                    self.prog.add_read(row[1], row[2], row[3], row[4])
                elif row[0] == "write":
                    self.prog.add_write(row[1], row[2], row[3], row[4])
                else:
                    raise Exception("Unknown event type: %s" % row[0])
                # c -= 1
                # if c <= 0: break

    def __mst_read_params(self, mst_reader, n):
        params = {}
        for i in range(n):
            (k, v) = next(mst_reader)
            params[k] = v
        return params

    def __mst_read_table(self, mst_reader):
        (n, m) = map(int, list(next(mst_reader)))
        if m == 1:
            return [next(mst_reader)[0] for i in range(n)]
        if n == 1:
            return next(mst_reader)
        res = []
        for i in range(n):
            res.append(next(mst_reader))
        return res

    def __parse_mst(self, mst_filename):
        """
        Format of file:

        Sections
        CPU -> NUMA domain map
        CPU overhead table (o)
        CPU overhead table (O)
        NUMA bandwidth table (g)
        NUMA bandwidth table (G)
        NUMA latency map (L)

        Table structure
        <table name> <n params>
        <param> <value>
        <n> <m>
        <table>

        Empty lines are permitted
        """

        with open(mst_filename) as mst_file:
            mst_reader = csv.reader(mst_file, delimiter=' ')
            self.arch = Architecture(next(mst_reader)[0])

            while True:
                try:
                    tab_header = next(mst_reader)
                    if len(tab_header) == 0:
                        continue
                    tab_name = tab_header[0]
                    nparams = int(tab_header[1])
                    params = self.__mst_read_params(mst_reader, nparams)
                    tab = self.__mst_read_table(mst_reader)
                    self.arch.set_table(tab_name, params, tab)
                except StopIteration:
                    break

    def __parse_sched(self, sched_filename):
        sched_variants = {
            "init_page" : {"first_touch" : FirstTouch,
                           "magic" : MagicTouch,
                           "random" : RandomTouch},
            "commit" : {"nomove" : NoMoveCommit,
                        "movehere" : MoveHereCommit}
        }
        sched_params = json.load(open(sched_filename, 'r'))

        self.scheduler = Scheduler(None)
        for k in sched_params:
            v = sched_params[k]
            if k in sched_variants:
                if v in sched_variants[k]:
                    setattr(self.scheduler, k, sched_variants[k][v](self.scheduler))
                else:
                    raise Exception("Unexpected value %s for parameter %s" % (v, k))


    def __init__(self):
        parser = OptionParser()
        parser.add_option("-d", "--dtl", dest="dtl",
                          help="Load trace in dtl format.", metavar="FILE")
        parser.add_option("-m", "--mst", dest="mst",
                          help="Load machine specification in mst format.", metavar="FILE")
        parser.add_option("-s", "--sched", dest="sched",
                          help="Load scheduling parameters key-value format.", metavar="FILE")

        (options, args) = parser.parse_args()
        if options.dtl is not None:
            self.__parse_dtl(options.dtl)
        else:
            print("Missing algorithm specification")
            parser.print_help()
            exit()

        if options.mst is not None:
            self.__parse_mst(options.mst)
        else:
            print("Missing architecture specification")
            parser.print_help()
            exit()

        if options.sched is not None:
            self.__parse_sched(options.sched)
        else:
            print("Missing scheduling parametrs specification")
            parser.print_help()
            exit()

        print("Hi from RackSim {%s}" % options.dtl)

    def __init__(self, trace, mst, sched):
        self.__parse_dtl(trace)
        self.__parse_mst(mst)
        self.__parse_sched(sched)

    def run(self):
        execution = Execution(self.arch, self.prog, self.scheduler)
        print("Run racksim")
        return execution.run()
