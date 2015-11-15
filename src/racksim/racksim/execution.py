import bisect

from .programm import Programm
from .architecture import Architecture

class Timeslot:
    def __init__(self, begin, end, thunk):
        if end <= begin:
            raise Exception("Begin goes after end")
        self.begin = begin
        self.end = end
        self.thunk = thunk

    def __eq__(self, other):
        return ((self.begin, self.end) == (self.begin, self.end))

    def __lt__(self, other):
        return ((self.begin, self.end) < (self.begin, self.end))

    def concurrent(self, other):
        return not ((self.end < other.begin) or (other.end < self.begin))

class Timeline:
    def __init__(self):
        self.time = []

class Execution:
    def __init__(self, arch, prog):
        if type(arch) is not Architecture:
            raise Exception("Expected parameter of type Architecture, got %s"
                            % str(type(arch)))
        if type(prog) is not Programm:
            raise Exception("Expected parameter of type Programm, got %s"
                            % str(type(prog)))
        self.arch = arch
        self.prog = prog
        self.timelines = [Timeline() for i in range(self.arch.cores)]

    def run(self):
        for thunk in self.prog.run():
            print("Schedule thunk %s " % (thunk))
