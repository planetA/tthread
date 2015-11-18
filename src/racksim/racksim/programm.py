from collections import defaultdict

import networkx as nx
import networkx.algorithms.dag as dag
import matplotlib.pyplot as plt

from .thunk import ThunkId, ThunkData
from .event import Event, CommEvent, ThunkEvent, EventDAG

class Programm:
    def __init__(self):
        self.thunks = {}
        self.order = []
        self.cur = {}
        self.edges = defaultdict(lambda:list(),{})
        self.finished = {}

    def __del__(self):
        return
        for t in self.order:
            print ("%s: %s" % (t, self.thunks[t]))
            self.thunks[t].print_pages()

    def add_thunk(self, thread, thunk, cpu):
        tid = ThunkId(thread, thunk)
        if tid in self.thunks:
            raise Exception("Unexpected thunk duplication: %s" % tid)
        thunk_data = ThunkData(tid, cpu)
        self.thunks[tid] = thunk_data
        self.order.append(tid)
        if tid.thread() in self.cur:
            self.finished[tid.thread()] = self.cur[tid.thread()]
        for out in self.finished:
            out_thunk = self.thunks[ThunkId(out, self.finished[out])]
            self.edges[out_thunk].append(thunk_data)
        self.cur[tid.thread()] = tid.thunk()

    def add_finish(self, thread, thunk):
        tid = ThunkId(thread, thunk)
        if tid not in self.thunks:
            raise Exception("Expected to find thunk %s, finish found." % tid)
        self.finished[tid.thread()] = tid.thunk()
        del self.cur[tid.thread()]

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

    def show(self):
        nx.draw_networkx(self.edag.dag, pos=nx.graphviz_layout(self.edag.dag))
        plt.show()

    def start(self):
        print(self.edges)
        self.edag = EventDAG(self.edges)
        self.entry = CommEvent(self.thunks[ThunkId(0, 1)])

        for event in self.edag.dag.nodes():
            event.wait = len(self.edag.dag.in_edges(event))

        if len(self.edag.dag.predecessors(self.entry)) != 0:
            raise Exception("Thunk %s is not the programm entry." % self.entry)
        # print (self.edges)
        # for thunk in self.order:
        #     yield self.thunks[thunk]
        #     return
