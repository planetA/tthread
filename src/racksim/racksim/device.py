from heapq import *
from .task import *

class Alarm:
    def __init__(self, time, item, dev = None):
        self.time = time
        self.item = item
        self.dev = dev

    def __lt__(self, other):
        return self.time < other.time

    def __eq__(self, other):
        return self.time == other.time

    def __hash(self):
        return hash(self.time, self.item)

    def __repr__(self):
        return "%s: %s" % (self.time, self.item)

    def complete(self):
        if self.dev:
            self.dev.complete()
        return self.item.complete()

class Device:
    def __init__(self, id):
        self.id = id
        self.active = set()
        self.runnig = set()

    def __eq__(self, other):
        return (self.id, type(self)) == (other.id, type(other))

    def __hash__(self):
        return hash((self.id, type(self)))

    def __repr__(self):
        return "%s-%s" % (type(self), self.id)

    def is_active(self):
        return len(self.active) > 0

    def complete(self):
        pass

class NumaDevice(Device):
    def __init__(self, id):
        super(NumaDevice, self).__init__(id)
        self.rs = set()
        self.ws = set()

    def reserve(self, msg):
        self.active.add(msg)
        msg.task.counter += 1

    def progress(self, now):
        msg = self.active.pop()
        alarms = Alarm(now + 0, msg, self)
        return [alarms]

    def complete(self):
        # This should delete duplicated pages or install the requested
        # ones
        pass

class LinkDevice(Device):
    def __init__(self, id):
        super(LinkDevice, self).__init__(id)
        self.id = (min(id), max(id))

    def reserve(self, msg):
        self.active.add(msg)
        msg.task.counter += 1

    def progress(self, now):
        alarms = [Alarm(now + msg.time(), msg) for msg in self.active]
        self.running = self.active
        self.active = set()
        return alarms

class CpuDevice(Device):
    def reserve(self, msg):
        self.active.add(msg)
        msg.task.counter += 1

    def progress(self, now):
        msg = self.active.pop()
        alarms = Alarm(now + msg.task.thunk.cputime, msg, self)
        self.running = [alarms]
        return self.running

    def complete(self):
        self.runnig = set()

    def is_active(self):
        return len(self.active) > 0 and len(self.runnig) == 0

class Message:
    """
    Message is a piece of work for a device.
    """
    def __init__(self, task, src, dst = None):
        self.task = task
        self.src = src
        self.dst = dst

    def complete(self):
        self.task.counter -= 1
        if self.task.counter == 0:
            return self.task
        else:
            return None

    def time(self):
        return 0

    def __repr__(self):
        return "%s->%s: %s" % (self.src, self.dst, self.task)

class Machine:
    def __init__(self, arch):
        pass

class MachineNumaNet(Machine):
    """Machine of type NumaNet: Represented by a network of NUMA domains
    and a set of CPUs. Each NUMA domain has some CPUs connected to it.

    If a CPU fetches a page from local NUMA domain it take unit costs.

    If a CPU fetches a page from remote NUMA domain several NUMA nodes are
    involved

    """
    def __init__(self, execution):
        self.arch = execution.arch
        self.devices = set()
        self.links = {}
        self.pes = {}
        self.numas = {}
        self.alarms = []
        for node in self.arch.numa_g.edges():
            print("NODE: ", node, type(node))
            link = LinkDevice(node)
            self.devices.add(link)
            self.links[link.id] = link
        for node in range(self.arch.cores):
            cpu = CpuDevice(node)
            self.devices.add(cpu)
            self.pes[cpu.id] = cpu
        for domain in range(self.arch.domains):
            numa = NumaDevice(domain)
            self.devices.add(numa)
            self.numas[numa.id] = numa
        print(self.links)
        print("CPU-o" , self.arch.cpu_o)
        print("NUMA-g" , self.arch.numa_g.edges())


    def schedule(self, task):
        if type(task) is CommTask:
            print("Need fetch r %d w %d pages" % (len(task.thunk.rs), len(task.thunk.ws)))
            for adj in range(len(self.pes)):
                src = self.arch.cpu2numa[task.thunk.cpu]
                dst = self.arch.cpu2numa[adj]
                if src == dst:
                    continue
                msg = Message(task, task.thunk.cpu, adj)

                path = nx.shortest_path(self.arch.numa_g, src, dst)
                for hop in zip(path[0:-1], path[1:]):
                    link = LinkDevice(hop)
                    self.links[link.id].reserve(msg)
        elif type(task) is ThunkTask:
            pe = CpuDevice(task.thunk.cpu)
            msg = Message(task, task.thunk.cpu)
            self.pes[pe.id].reserve(msg)
        elif type(task) is CommitTask:
            local_domain = self.arch.cpu2numa[task.thunk.cpu]
            msg = Message(task, local_domain)
            self.numas[local_domain].reserve(msg)
        else:
            raise Exception("Unknown task to schedule of type %s" % type(task))

    def progress(self, now):
        for dev in self.devices:
            if dev.is_active():
                for alarm in dev.progress(now):
                    heappush(self.alarms, alarm)

    def complete(self):
        earliest = self.alarms[0].time
        finished = []
        # Time of the first element
        while self.alarms and (self.alarms[0].time == earliest or not finished):
            alarm = heappop(self.alarms)
            task = alarm.complete()
            if task:
                finished.append(task)
        return (alarm.time, finished)
