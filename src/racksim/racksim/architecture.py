import numpy
import networkx as nx

class Architecture:
    """
    Class describes the simulated machine, specificall network
    topology
    """

    def __init__(self, model):
        if type(model) is not str:
            raise Exception("Expected model to be of type str.")

        self.model = model
        if self.model == "LogGOP":
            # These are more declarations than initializations
            self.cpu2numa = None
            self.cpu_o = None
            self.cpu_O = None
            self.numa_g = None
            self.numa_G = None
            self.numa_L = None
            # Number of CPU cores
            self.cores = None
            # Number of NUMA domains
            self.domains = None

            self.names = {
                "CPU-to-NUMA" : "cpu2numa",
                "CPU-o" : "cpu_o",
                "CPU-O" : "cpu_O",
                "NUMA-g" : "numa_g",
                "NUMA-G" : "numa_G",
                "NUMA-L" : "numa_L"
                }
            self.params = {tab : None for tab in self.names}
        print(model)

    def set_table(self, tab_name, params, tab_data):
        if tab_name not in self.names:
            raise Exception("Unexpected table: %s" % tab_name)

        if getattr(self, self.names[tab_name]) is not None:
            Exception("Duplicate table definition: %s" % tab_name)

        # Check specific table initialization
        if tab_name in ["CPU-to-NUMA", "CPU-o", "CPU-O"]:
            if self.cores is None:
                self.cores = len(tab_data)
            elif self.cores != len(tab_data):
                raise Exception("Expected %d cores, found %d" %(self.cores, len(tab_data)))
            tab = [float(i) for i in tab_data]
        elif tab_name in ["NUMA-g", "NUMA-L", "NUMA-G"]:
            if self.domains is None:
                self.domains = len(tab_data)
            elif self.domains != len(tab_data):
                raise Exception("Expected %d domains, found %d" %(self.domains, len(tab_data)))
            adj_matr = numpy.matrix([[float(cell) for cell in row] for row in tab_data])
            tab = nx.from_numpy_matrix(adj_matr)

        setattr(self, self.names[tab_name], tab)

    def __repr__(self):
        tabs = []
        for name in self.names:
            tab = getattr(self, self.names[name])
            if type(tab) is nx.Graph:
                tab = str(tab.edges(data=True))
            tabs.append("%s: %s" %(name, tab))
        return '\n'.join(tabs)

