import heapq
import networkx as nx

from .thunk import *

class EventDAG:

    def __transitive_reduction(self, g):
        for n1 in g.nodes_iter():
            if g.has_edge(n1, n1):
                g.remove_edge(n1, n1)
            for n2 in g.successors(n1):
                for n3 in g.successors(n2):
                    for n4 in nx.dfs_preorder_nodes(g, n3):
                        if g.has_edge(n1, n4):
                            g.remove_edge(n1, n4)

    def __init__(self, thunkEdges):
        g = nx.DiGraph(dict(thunkEdges))
        self.__transitive_reduction(g)
        self.dag = nx.DiGraph()
        for tid in g.nodes():
            commEv = CommEvent(tid)
            thunkEv = ThunkEvent(tid)
            self.dag.add_node(commEv)
            self.dag.add_node(thunkEv)
            self.dag.add_edge(commEv, thunkEv)
            in_thunks = map(lambda x : ThunkEvent(x[0]), g.in_edges(tid))
            for i in in_thunks:
                self.dag.add_edge(i, commEv)
            out_thunks = map(lambda x : CommEvent(x[1]), g.out_edges(tid))
            for o in out_thunks:
                if type(o) is ThunkEvent:
                    raise Exception("TT")
                self.dag.add_edge(thunkEv, o)

        # Verify correct DAG
        for n in self.dag.nodes():
            if type(n) is CommEvent:
                if len(self.dag.successors(n)) > 1:
                    raise Exception("CommEvent %s has more than 1 successor." % n)


class EventQueue:
    pass

class Event:
    def __init__(self, thunk):
        if type(thunk) is not ThunkData:
            raise Exception("Expected tid of type ThunkData, found %s" % type(thunk))
        self.thunk = thunk
        self.tid = thunk.tid
        self.wait = 0
        self.counter = 0

    def __eq__(self, other):
        return self.tid == other.tid

    def __hash__(self):
        return hash((self.tid, type(self)))

    def __repr__(self):
        return "%s" % self.thunk

class ThunkEvent(Event):
    def __repr__(self):
        return "t%s" % self.thunk

class CommEvent(Event):
    def __repr__(self):
        return "c%s" % self.thunk
