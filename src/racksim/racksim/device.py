from heapq import *
from .event import *

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
        return self.id == other.id

    def __hash__(self):
        return hash(self.id)

    def __repr__(self):
        return "%s-%s" % (type(self), self.id)

    def is_active(self):
        return len(self.active) > 0

    def complete(self):
        pass

class LinkDevice(Device):
    def __init__(self, id):
        super(LinkDevice, self).__init__(id)
        self.id = (min(id), max(id))

    def reserve(self, msg):
        self.active.add(msg)
        msg.event.counter += 1
        print("Adding to %s msg %s %s" %(self, msg, self.is_active()))

    def progress(self, now):
        alarms = [Alarm(now + msg.time(), msg) for msg in self.active]
        self.running = self.active
        self.active = set()
        return alarms

class CpuDevice(Device):
    def reserve(self, msg):
        self.active.add(msg)
        msg.event.counter += 1

    def progress(self, now):
        msg = self.active.pop()
        alarms = Alarm(now + msg.event.thunk.cputime, msg, self)
        self.running = [alarms]
        self.active = set()
        return self.running

    def complete(self):
        self.runnig = set()

    def is_active(self):
        return len(self.active) > 0 and len(self.runnig) == 0

class Message:
    """
    Message is a piece of work for a device.
    """
    def __init__(self, event, src, dst = None):
        self.event = event
        self.src = src
        self.dst = dst

    def complete(self):
        self.event.counter -= 1
        if self.event.counter == 0:
            return self.event
        else:
            return None

    def time(self):
        return 0

    def __repr__(self):
        return "%s->%s: %s" % (self.src, self.dst, self.event)

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
        print(self.links)
        print("CPU-o" , self.arch.cpu_o)
        print("NUMA-g" , self.arch.numa_g.edges())


    def schedule(self, event):
        if type(event) is CommEvent:
            print("ScheduleCommEvent %d %s" % (len(event.thunk.rs), event))
            for adj in range(len(self.pes)):
                src = self.arch.cpu2numa[event.thunk.cpu]
                dst = self.arch.cpu2numa[adj]
                if src == dst:
                    continue
                print("Reserve ", (event.thunk.cpu, adj))
                msg = Message(event, event.thunk.cpu, adj)

                path = nx.shortest_path(self.arch.numa_g, src, dst)
                for hop in zip(path[0:-1], path[1:]):
                    link = LinkDevice(hop)
                    self.links[link.id].reserve(msg)
        elif type(event) is ThunkEvent:
            pe = CpuDevice(event.thunk.cpu)
            msg = Message(event, event.thunk.cpu)
            self.pes[pe.id].reserve(msg)
            print("ScheduleThunkEvent")
        else:
            raise Exception("Unknown event to schedule of type %s" % type(event))

    def progress(self, now):
        for dev in self.devices:
            if dev.is_active():
                print("Active dev ", dev)
                for alarm in dev.progress(now):
                    heappush(self.alarms, alarm)

    def complete(self):
        print("Complete")
        earliest = self.alarms[0].time
        print(self.alarms, earliest)
        finished = []
        # Time of the first element
        while self.alarms and (self.alarms[0].time == earliest or not finished):
            alarm = heappop(self.alarms)
            event = alarm.complete()
            print("Trigger alalrm", alarm, event)
            if event:
                finished.append(event)
        return (alarm.time, finished)
