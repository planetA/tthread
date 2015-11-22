
class Event:
    """
    Event used to determine when a device finishes task
    """
    def __init__(self, task):
        self.task = task
        self.time = 0

    def reserve(self, device):
        self.task.counter += 1
        device.reserve(self)

    def complete(self):
        self.task.counter -= 1
        if self.task.counter == 0:
            return self.task
        else:
            return None

    def __repr__(self):
        return "%s" % (self.task)

class CommEvent(Event):
    def __init__(self, task, numa, pages):
        self.task = task
        # Numa domain to get the memory
        self.numa = numa
        self.pages = pages

    def complete(self):
        self.task.counter -= 1
        if self.task.counter == 0:
            return self.task
        else:
            return None
