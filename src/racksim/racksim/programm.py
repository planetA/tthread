from .thunk import ThunkId, ThunkData

class Programm:
    def __init__(self):
        self.thunks = {}
        self.order = []
        self.cur = {}

    def __del__(self):
        return
        for t in self.order:
            print ("%s: %s" % (t, self.thunks[t]))
            self.thunks[t].print_pages()

    def add_thunk(self, thread, thunk, cpu):
        tid = ThunkId(thread, thunk)
        if tid in self.thunks:
            raise Exception("Unexpected thunk duplication: %s" % tid)
        self.thunks[tid] = ThunkData(tid, cpu)
        self.order.append(tid)
        self.cur[tid.thread()] = tid.thunk()

    def add_read(self, thread, thunk, begin, end):
        tid = ThunkId(thread, thunk)
        if tid not in self.thunks:
            raise Exception("Expected thunk to exist: %s" % tid)
        # We are passed range of type [], we need it to be [)
        self.thunks[tid].rs.update(range(int(begin), int(end) + 1))

    def add_write(self, thread, thunk, begin, end):
        tid = ThunkId(thread, thunk)
        if tid not in self.thunks:
            raise Exception("Expected thunk to exist: %s" % tid)
        # We are passed range of type [], we need it to be [)
        self.thunks[tid].ws.update(range(int(begin), int(end) + 1))

    def add_finish(self, thread, thunk):
        tid = ThunkId(thread, thunk)
        if tid not in self.thunks:
            raise Exception("Expected to find thunk %s, finish found." % tid)
        pass

    def add_end(self, thread, thunk, cpu_time):
        tid = ThunkId(thread, thunk)
        # Master thunk has no start in the logs
        if tid.thunk() == 0:
            self.thunks[tid] = ThunkData(tid, 0)
            self.thunks[tid].cputime = 0
        if tid not in self.thunks:
            raise Exception("Expected to find thunk %s, finish found." % tid)
        if self.thunks[tid].cputime != float("-inf"):
            raise Exception("Expected to not find end of %s." % tid)
        self.thunks[tid].cputime = float(cpu_time)

    def run(self):
        for thunk in self.order:
            yield self.thunks[thunk]
