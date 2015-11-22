import heapq
import networkx as nx

from .thunk import *

class TaskDAG:

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
            commEv = CommTask(tid)
            thunkEv = ThunkTask(tid)
            commitEv = CommitTask(tid)
            self.dag.add_node(commEv)
            self.dag.add_node(thunkEv)
            self.dag.add_node(commitEv)
            self.dag.add_edge(commEv, thunkEv)
            self.dag.add_edge(thunkEv, commitEv)
            in_thunks = map(lambda x : ThunkTask(x[0]), g.in_edges(tid))
            for i in in_thunks:
                self.dag.add_edge(i, commEv)
            out_thunks = map(lambda x : CommTask(x[1]), g.out_edges(tid))
            for o in out_thunks:
                if type(o) is ThunkTask:
                    raise Exception("TT")
                self.dag.add_edge(commitEv, o)

        # See XXX of successors
        self.dic = {ev : ev for ev in self.dag.nodes()}
        # Verify correct DAG
        for n in self.dag.nodes():
            if type(n) is CommTask:
                if len(self.dag.successors(n)) > 1:
                    raise Exception("CommTask %s has more than 1 successor." % n)

    def successors(self, node):
        # XXX: That's rather a hack, because for some reason
        # successors sometimes return copies of objects and not the
        # objects itself
        return [self.dic[ev] for ev in self.dag.successors(node)]

class TaskQueue:
    pass

class Task:
    def __init__(self, thunk):
        if type(thunk) is not ThunkData:
            raise Exception("Expected tid of type ThunkData, found %s" % type(thunk))
        self.thunk = thunk
        self.tid = thunk.tid
        self.wait = None
        self.counter = 0

    def __eq__(self, other):
        return self.tid == other.tid

    def __hash__(self):
        return hash((self.tid, type(self)))

    def __repr__(self):
        return "%s" % self.thunk

class ThunkTask(Task):
    def __repr__(self):
        return "t%s" % self.thunk

class CommTask(Task):
    def __init__(self, thunk):
        super(CommTask, self).__init__(thunk)
        self.time = 0

    def __repr__(self):
        return "c%s CPU %d TIME %f" % (self.thunk.tid, self.thunk.cpu, self.time)

class CommitTask(Task):
    def __repr__(self):
        return "m%s" % self.thunk
