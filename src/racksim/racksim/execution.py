from enum import Enum
import bisect

from .task import CommTask, ThunkTask
from .programm import Programm
from .architecture import Architecture
from .device import Device, Machine, MachineNumaNet

class PageState(Enum):
    inuse = 1
    committed = 2

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
        return not ((self.end <= other.begin) or (other.end <= self.begin))

class Timeline:
    def __init__(self):
        self.time = []
        self.reaped = True

    def append(self, thunk):
        if len(self.time) > 0:
            last = self.time[-1]
            new_thunk = Timeslot(last.end, last.end + thunk.cputime, thunk)
        else:
            new_thunk = Timeslot(0, thunk.cputime, thunk)
        self.time.append(new_thunk)
        self.reaped = False

    def reap(self, memory):
        if self.reaped:
            return
        self.reaped = True
        if len(self.time) == 0:
            Exception("Expected empty timeline to be reaped")
        last = self.time[-1].thunk
        print("Reap thunk %s" % last)
        for page in last.rs | last.ws:
            memory.pagestate[page] = PageState.committed
            if page == 34357618688:
                print("Reaping page %d" % page)
            if len(memory.pagemap[page]) > 1:
                memory.pagemap[page].remove(last.cpu)

    def __repr__(self):
        sched = []
        for slot in self.time:
            sched.append("[%f-%f): %s" % (slot.begin, slot.end, slot.thunk))
        return "\n".join(sched)

class Memory:
    def __init__(self, domains):
        self.memory = [set() for i in range(domains)]
        self.pagemap = {}
        self.pagestate = {}

    def move(self, page, cpu):
        if page in self.pagemap:
            if self.pagestate[page] == PageState.inuse:
                self.pagemap[page].append(cpu)
            elif self.pagestate[page] == PageState.committed:
                self.pagemap[page] = set([cpu])
                self.pagestate[page] = PageState.inuse
        else:
            self.pagemap[page] = set([cpu])
            self.pagestate[page] = PageState.inuse

class Execution:
    def memmove(self, thunk):
        print("Move memory for thunk %s" % thunk)
        for page in thunk.rs | thunk.ws:
            self.memory.move(page, thunk.cpu)

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
        self.memory = Memory(self.arch.domains)
        self.rack = MachineNumaNet(self)
        # Set of active devices
        self.active = set()

    def run(self):
        """
        Task state transisitons are:
        blocked -> ready -> active -> running -> finished
        """

        self.prog.start()

        now = 0
        ready = {self.prog.entry}
        print('Start execution')
        print('='*60)
        import ipdb
        while self.prog.exit.wait > 0 or ready:
            # ipdb.set_trace()
            while ready:
                task = ready.pop()
                self.rack.schedule(task) # ready -> active

            self.rack.progress(now) # active -> running

            (now, tasks) = self.rack.complete() # running -> finished

            for task in tasks:
                # if type(task) is ThunkTask:
                print("%s : (%s) %s" % (now, type(task), task))
                # if now == 26758111.0: ipdb.set_trace()
                for succ in  self.prog.edag.successors(task):
                    succ.wait -= 1
                    if succ.wait == 0:
                        ready.add(succ)

        # for thunk in self.prog.run():
        #     # Update page statuses before scheduling next thunk
        #     for timeline in self.timelines:
        #         timeline.reap(self.memory)
        #     self.timelines[thunk.cpu].append(thunk)
        #     self.memmove(thunk)
        #     # thunkTask = ThunkTask(evId)
        #     # commTasks = self.memory.makeCommTasks(thunkTask)
        #     # thunkTask and commTasks have two Id's
        #     # evId += 2
        #     print("Schedule thunk %s " % (thunk))

        # for task in self.taskq:
        #     if task.isReady():
        #         task.process()

        # print()
        # for timeline in self.timelines:
        #     print (timeline)
        #     print()
