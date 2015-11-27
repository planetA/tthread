import random

from .event import CommEvent
from .device import LinkDevice

class Scheduler:
    def __init__(self, parent):
        self.parent = parent
        self.machine = None

    def setMachine(self, machine):
        self.machine = machine

class FirstTouch(Scheduler):
    def __call__(self, dst, pages, task):
        self.parent.machine.numas[dst].pages |= pages

class MagicTouch(Scheduler):
    def __call__(self, dst, pages, task):
        MAGIC_DOMAIN = 1
        src = MAGIC_DOMAIN
        dst_numa = self.parent.machine.numas[dst]
        if dst != src:
            src_numa = self.parent.machine.numas[src]
            src_numa.pages |= pages
            event = CommEvent(task, dst_numa, pages)
            event.time = self.parent.machine.arch.page_time(dst, src, len(pages))
            (delay, path) = self.parent.machine.arch.shortest_path(dst, src)
            event.time += delay
            task.time = event.time

            for hop in zip(path[0:-1], path[1:]):
                link = LinkDevice(hop)
                event.reserve(self.parent.machine.links[link.id])
        else:
            dst_numa.pages |= pages

class RandomTouch(Scheduler):
    def __init__(self, parent):
        super(RandomTouch, self).__init__(parent)
        random.seed(4)

    def __call__(self, dst, pages, task):
        buckets = [set() for i in range(len(self.parent.machine.numas))]
        for page in pages:
            bucket = random.choice(buckets)
            bucket.add(page)

        for src in range(len(buckets)):
            dst_numa = self.parent.machine.numas[dst]
            if dst != src:
                src_numa = self.parent.machine.numas[src]
                src_numa.pages |= buckets[src]
                event = CommEvent(task, dst_numa, buckets[src])
                event.time = self.parent.machine.arch.page_time(dst, src, len(buckets[src]))
                (delay, path) = self.parent.machine.arch.shortest_path(dst, src)
                event.time += delay
                task.time = event.time

                for hop in zip(path[0:-1], path[1:]):
                    link = LinkDevice(hop)
                    event.reserve(self.parent.machine.links[link.id])
            else:
                dst_numa.pages |= pages

class NoMoveCommit(Scheduler):
    def __call__(self, task):
        arch = self.parent.machine.arch
        numas = self.parent.machine.numas
        pes = self.parent.machine.pes
        links = self.parent.machine.links

        src = arch.cpu2numa[task.thunk.cpu]
        src_numa = numas[src]
        pages = task.thunk.ws
        pages = pages - src_numa.pages
        print("Task %s commits %d pages" % (task, len(pages)))
        for adj in range(len(pes)):
            dst = arch.cpu2numa[adj]
            if dst == src:
                continue

            dst_numa = numas[dst]
            commit = dst_numa.pages & pages
            if len(commit) == 0:
                continue
            pages = pages - commit
            event = CommEvent(task, src_numa, commit)
            event.time = arch.page_time(src, dst, len(commit))
            (delay, path) = arch.shortest_path(src, dst)
            event.time += delay
            task.time = event.time

            for hop in zip(path[0:-1], path[1:]):
                link = LinkDevice(hop)
                event.reserve(links[link.id])
                delay += 0
        event = CommEvent(task, src_numa, pages)
        event.reserve(src_numa)

class MoveHereCommit(Scheduler):
    def __call__(self, task):
        pass
