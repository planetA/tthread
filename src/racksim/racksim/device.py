from heapq import *
from .task import *
from .event import *
from .alarm import *

class Device:
    def __init__(self, id):
        self.id = id
        self.active = set()
        self.running = set()

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
        self.pages = set()
        self.thunks = set()

    def reserve(self, event):
        self.active.add(event)

    def progress(self, now):
        event = self.active.pop()
        alarm = Alarm(now + 0, event, self)
        self.running.add(event)
        return [alarm]

    def complete(self):
        for event in self.running:
            if type(event) is CommEvent:
                print("Complete %s in %s" % (event, self))
            elif type(event) is Event:
                # That's stupid
                pass
            else:
                raise Exception("Unexpected event type %s" % type(event))
        self.running = set()
        # This should delete duplicated pages or install the requested
        # ones
        pass

class LinkDevice(Device):
    def __init__(self, id):
        super(LinkDevice, self).__init__(id)
        self.id = (min(id), max(id))

    def reserve(self, event):
        self.active.add(event)

    def progress(self, now):
        print("Progress %s LinkEvent %s " % (self, [event.time for event in self.active]))
        alarms = [Alarm(now + event.time, event) for event in self.active]
        self.running = self.active
        self.active = set()
        return alarms

class CpuDevice(Device):
    def reserve(self, event):
        self.active.add(event)

    def progress(self, now):
        event = self.active.pop()
        alarms = Alarm(now + event.task.thunk.cputime, event, self)
        self.running = [alarms]
        return self.running

    def complete(self):
        self.running = set()

    def is_active(self):
        return len(self.active) > 0 and len(self.running) == 0


class Machine:
    def __init__(self, arch):
        pass
