
class ThunkId:
    def thread(self):
        return self.id[0]

    def thunk(self):
        return self.id[1]

    def __init__(self, thread, thunk):
        self.id = (int(thread), int(thunk))

    def __str__(self):
        return "(%d, %d)" % (self.id[0], self.id[1])

    def __repr__(self):
        return "(%d, %d)" % (self.id[0], self.id[1])

    def __hash__(self):
        return hash(self.id)

    def __eq__(self, other):
        return self.id == other.id

class ThunkData:
    def __init__(self, tid, cpu):
        self.tid = tid
        self.cpu = int(cpu)
        self.cputime = float("-inf")
        self.rs = set()
        self.ws = set()

    def print_pages(self):
        for addr in self.__ranges(self.rs):
            print ("read %s: [%d, %d]" % (self.tid, addr[0], addr[1]))
        for addr in self.__ranges(self.ws):
            print ("write %s: [%d, %d]" % (self.tid, addr[0], addr[1]))

    def __ranges(self, lst):
        s = e = None
        for i in sorted(lst):
            if s is None:
                s = e = i
            elif i == e or i == e + 1:
                e = i
            else:
                yield (s, e)
                s = e = i
        if s is not None:
            yield (s, e)

    def __str__(self):
        return "%s CPU: %d TIME %f " % (self.tid, self.cpu, self.cputime)

    def __repr__(self):
        return "%s CPU: %d TIME %f " % (self.tid, self.cpu, self.cputime)
