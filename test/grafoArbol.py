#!/usr/bin/python3
# -*- coding: utf-8 -*-

import networkx as nx
import random
for i in range(100,1001,100):
    n = i

    G = nx.random_powerlaw_tree(n,tries=1000000)

    for (u, v, w) in G.edges(data=True):
        w['weight'] = random.randint(0, 10)

    m = G.number_of_edges()
    tamanios = "{}\n{}\n".format(n, m)
    tamaniosBytes = bytes(tamanios, "utf-8")
    with open("experimentacion/arbol/arbol" + str(i) + ".txt", "wb") as file:  # te hace el open y el close
        file.write(tamaniosBytes)
        nx.write_weighted_edgelist(G, file)