import csv
import numpy as np
import pandas as pd
import networkx as nx
import cpalgorithm as cp

G=nx.karate_club_graph()

be = cp.KM_ER(1)
Q = []
be.detect(G)
c = be.get_pair_id()
x = be.get_coreness()
print(c,x)
