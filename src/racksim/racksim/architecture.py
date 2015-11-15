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
            self.cpu2numa = []
            self.cpu_o = []
            self.cpu_O = []
            self.numa_g = nx.Graph()
            self.numa_G = nx.Graph()
            self.numa_L = nx.Graph()

        print(model)
