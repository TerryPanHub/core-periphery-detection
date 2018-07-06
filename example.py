import csv
import numpy as np
import pandas as pd
import networkx as nx
import cpalgorithm as cp

G=nx.karate_club_graph()
G=nx.florentine_families_graph()
	
be = cp.KM_config()

be.detect(G)

c = be.get_pair_id()
x = be.is_core()
significance, p_values, q_tilde, s_tilde = cp.qstest(c, x, G, be)

print(significance, p_values)
