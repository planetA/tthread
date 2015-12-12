from .scheduler import Scheduler
from .thunk import ThunkId

import networkx as nx

class MmcpMigration(Scheduler):
    def order_thunks(self, prog):
        rel_priorities = {}
        rws = {}
        dag = nx.DiGraph(prog.edges)
        thunks = dag.nodes() # list of thunks
        for t in thunks:
            pages = set()
            for v in nx.bfs_tree(dag, t):
                pages |= v.rs | v.ws
            rel_priorities[t] = len(pages)
        priorities = [(t,sum([rel_priorities[t]-rel_priorities[v]
                              for v in rel_priorities]))
                      for t in nx.topological_sort(dag)]
        priorities = [t for t,_ in sorted(priorities,
                                          key=lambda x: (x[1],
                                                         -x[0].tid.thread()),
                                          reverse=True)]
        return priorities

    def schedule_to_processors(self, prog, rack, priorities):
        sched_rws = {p : set() for p in self.parent.cpulist}
        bc = nx.betweenness_centrality(rack.arch.numa_g)
        schedule = {p : set() for p in self.parent.cpulist}
        cpu2numa = lambda c : rack.arch.cpu2numa[c]
        for t in priorities:
            min_cost = 0
            min_p = None
            rate = 0
            allowed_p = []
            while len(allowed_p) == 0:
                allowed_p = list(filter(lambda p : len(schedule[p]&prog.concurrent[t.tid]) == rate, self.parent.cpulist))
                rate += 1

            for p in sorted(allowed_p, key=lambda p : bc[cpu2numa(p)], reverse=True):
                cost = sum([len(sched_rws[0]&(t.rs|t.ws))*nx.shortest_path_length(rack.arch.numa_g, cpu2numa(p), cpu2numa(o)) for o in rack.numas])
                if min_p is None or min_cost > cost:
                    min_cost = cost
                    min_p = p
                    continue
            schedule[min_p].add(t.tid)
            sched_rws[min_p] |= t.rs | t.ws
        print(schedule)
        for cpu in schedule:
            for tid in schedule[cpu]:
                prog.thunks[tid].cpu = cpu

    def schedule(self, prog, rack):
        priorities = self.order_thunks(prog)
        self.schedule_to_processors(prog, rack, priorities)
