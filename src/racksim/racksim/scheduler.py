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
        if dst != src:
            src_numa = self.parent.machine.numas[src]
            src_numa.pages |= pages
            dst_numa = self.parent.machine.numas[dst]
            event = CommEvent(task, dst_numa, pages)
            event.time = self.parent.machine.arch.page_time(dst, src, len(pages))
            (delay, path) = self.parent.machine.arch.shortest_path(dst, src)
            event.time += delay
            task.time = event.time

            for hop in zip(path[0:-1], path[1:]):
                link = LinkDevice(hop)
                event.reserve(self.parent.machine.links[link.id])
