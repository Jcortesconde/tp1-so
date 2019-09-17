#!/usr/bin/python3
# -*- coding: utf-8 -*-

import networkx as nx
import random

for i in range(100,1001,100):
    n = i
    # Creo un grafo completo
    G = nx.complete_graph(n)
    for (u, v, w) in G.edges(data=True):
        w['weight'] = random.randint(1, n)

    m = G.number_of_edges()
    tamanios = "{}\n{}\n".format(n, m)
    tamaniosBytes = bytes(tamanios, "utf-8")
    with open("experimentacion/completo/completo" + str(i) + ".txt", "wb") as file:  # te hace el open y el close
        file.write(tamaniosBytes)
        nx.write_weighted_edgelist(G, file)
