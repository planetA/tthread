
class Alarm:
    def __init__(self, time, item, callback = None):
        self.time = time
        self.item = item
        self.callback = callback

    def __lt__(self, other):
        return self.time < other.time

    def __eq__(self, other):
        return self.time == other.time

    def __hash(self):
        return hash(self.time, self.item)

    def __repr__(self):
        return "%s: %s" % (self.time, self.item)

    def complete(self):
        if self.callback:
            self.callback.complete()
        return self.item.complete()
