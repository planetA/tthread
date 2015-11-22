from .device import *
from .event import *

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
        # TODO: XXX: This is an obvious target for Visitor pattern
        if type(task) is CommTask:
            src = self.arch.cpu2numa[task.thunk.cpu]
            src_numa = self.numas[src]
            pages = task.thunk.rs | task.thunk.ws
            print("%s on %s requires %d pages %d are present"
                  % (task, src_numa, len(pages), len(pages & src_numa.pages)))
            pages = pages - src_numa.pages
            # Order of iteration is not important unless we can have
            # more than one copy per page
            for adj in range(len(self.pes)):
                dst = self.arch.cpu2numa[adj]
                if src == dst:
                    continue

                dst_numa = self.numas[dst]
                fetch = dst_numa.pages & pages
                if len(fetch) == 0:
                    continue

                print("Fetch %d from %s" % (len(fetch), dst_numa))
                pages = pages - fetch
                event = CommEvent(task, src_numa, fetch)
                event.time = self.arch.page_time(src, dst, len(fetch))
                (delay, path) = self.arch.shortest_path(src, dst)
                event.time += delay

                for hop in zip(path[0:-1], path[1:]):
                    link = LinkDevice(hop)
                    print("Reserve delayed %s link %s for event %s parties %s" %
                          (delay, link, event, hop))
                    event.reserve(self.links[link.id])
                    # XXX: update delay
                    delay += 0
            if len(pages):
                # First touch policy
                src_numa.pages |= pages
                print("first touch pages %s (%d)" % (src_numa, len(src_numa.pages)))
            event = CommEvent(task, src_numa, pages)
            event.reserve(src_numa)
        elif type(task) is ThunkTask:
            pe = CpuDevice(task.thunk.cpu)
            event = Event(task)
            event.reserve(self.pes[pe.id])
        elif type(task) is CommitTask:
            local_domain = self.arch.cpu2numa[task.thunk.cpu]
            event = Event(task)
            event.reserve(self.numas[local_domain])
        else:
            raise Exception("Unknown task to schedule of type %s" % type(task))

    def progress(self, now):
        for dev in self.devices:
            if dev.is_active():
                for alarm in dev.progress(now):
                    print("Set alarm %s " % alarm)
                    heappush(self.alarms, alarm)

    def complete(self):
        # complete chain: alarm -> (device event)
        earliest = self.alarms[0].time
        finished = []
        # Time of the first element
        while self.alarms and (self.alarms[0].time == earliest or not finished):
            alarm = heappop(self.alarms)
            task = alarm.complete()
            if task:
                finished.append(task)
        return (alarm.time, finished)
