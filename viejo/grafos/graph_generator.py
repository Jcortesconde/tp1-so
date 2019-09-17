from random import *
import subprocess
import os
from timeit import default_timer as timer

''' Generating Tests'''
Size = 50   # from B to N size node
Z    = 1000  # how many times each size
R    = 1    # how many times each case
W    = 100  #maximum weight

def generar_completo(n):
  nodos = {}
  for nodo in range(n):
      lista_adj = []
      for nodo_adj in range(nodo, n):
          if(nodo != nodo_adj):
              peso = randint(0, W)
              lista_adj.append((nodo_adj, peso))
              
      nodos[nodo] = lista_adj

  for nodo in nodos:
    for nodo_adj, w in nodos[nodo]:
      if nodo_adj > nodo:
        nodos[nodo_adj].append((nodo,w))

  return nodos

def sacar_ejes(grafo,cant_nodos, ejes_a_sacar):
  while ejes_a_sacar != 0:
    nodo1 = randint(0, cant_nodos-1)
    nodo2 = randint(0, cant_nodos-1)
    w = -1
    if nodo1 != nodo2:
      for nodo_adj, weight in grafo[nodo1]:
        if(nodo2 == nodo_adj):
          w = weight
          grafo[nodo1].remove((nodo2, w))
          grafo[nodo2].remove((nodo1, w))
          ejes_a_sacar -= 1
          break
  return grafo

def dame_componente_conexa(grafo, visitados):
  primero = -1
  comp_conexa = []
  for nodo in grafo:
    if not visitados[nodo]:
      primero = nodo
      break
  if primero == -1:
    return comp_conexa
    
  cola = [primero]
  comp_conexa.append(nodo)
  visitados[primero] = True  
  while len(cola) != 0:
    nodo = cola.pop()
    for nodo_adj, w in grafo[nodo]:
      if( not visitados[nodo_adj]):
        visitados[nodo_adj] = True
        comp_conexa.append(nodo_adj)
        cola.append(nodo_adj)

  return comp_conexa

def unir_componentes(grafo):
  componentes = []
  visitados = [False for i, elem in enumerate(grafo)]
  componente = dame_componente_conexa(grafo, visitados)
  while(componente != []):
    componentes.append(componente)
    componente = dame_componente_conexa(grafo, visitados)

  cant_componentes = len(componentes)
  shuffle(componentes)
  for i in range(cant_componentes-1):
    ind1 = randint(0, len(componentes[i])-1)
    ind2 = randint(0, len(componentes[i+1])-1)
    nodo1 = componentes[i][ind1]
    nodo2 = componentes[i+1][ind2]
    peso = randint(0, W)
    grafo[nodo1].append((nodo2, W))
    grafo[nodo2].append((nodo1, W))

  return cant_componentes


#genera un grafo de n nodos con e ejes.
def generar_grafo_random(n, e):
  kn = generar_completo(n)
  kn_cant_ejes = (n*(n-1))//2
  grafo = sacar_ejes(kn, n, kn_cant_ejes - e)
  print(n, e, end = " ")
  cant_comp = unir_componentes(grafo)
  print(cant_comp)
  return grafo


def test_completo():
    # Test peor caso de m (completo en ambos sentidos)
    #   y cantidad de vertices fijo
    implementaciones = ["exa", "gol", "grasp", "blocal"]

    for i in implementaciones:
        with open('./data/res_%s'%i, 'w') as file_res:
            file_res.write('Frontera_%s,Clique_Size_%s\n'%(i, i))

    res_size = open("./data/res_size", 'w')
    res_size.write("Size,m\n")
    res_size.close()

    with open("./data/times.csv", "w") as times:
        times.write("Size")
        for i in implementaciones:
            times.write(",Time_%s"%i)
        times.write("\n")

    with open("./data/tests.csv", "w") as tests:
        tests.write("Tests,\n")

    for z in range(Z):
        if z % 5 == 0:
            print(z)
        for size in range(10, Size):
            grafo, ejes = generar_grafo_random(size)
            test = open("./data/test", "w")
            test.write(grafo)
            test.close()

            tests = open("./data/tests.csv", "a")
            tests.write(grafo+'\n')
            tests.close()

            for _ in range(0, R):

                ls_res = []
                for i in implementaciones:
                    start_ej = timer()
                    comm = "cat ./data/test | ./%s >> ./data/res_%s"%(i, i)
                    os.system(comm)
                    end_ej = timer()
                    ls_res.append(end_ej - start_ej)

                time_res = str(size)
                for line in ls_res:
                    time_res += ',' + str(line)
                time_res += '\n'
                times_csv = open("./data/times.csv", "a")
                times_csv.write(time_res)
                times_csv.close()
                
                res_size = open("./data/res_size", 'a')
                res_size.write(str(size)+','+str(ejes)+'\n')
                res_size.close()

def densidad_eje(n,densidad):
  arbol = n-1
  kn = (n*(n-1))//2
  ejes = densidad*(kn - arbol)/100 + arbol
  return int(ejes)

if __name__ == '__main__':
  for n in range(100, 300, 20):
    kn = (n*(n-1))//2
    arbol = n-1
    for densidad in range(0, 101, 10):
      ejes = densidad_eje(n, densidad)
      print(n,ejes, densidad, end = " ")
      grafo = generar_grafo_random(n, ejes)
      list_string = []
      for index, nodo in enumerate(grafo):
        for nod_adj, peso in grafo[nodo]:
          if nod_adj < nodo:
            break
          else:
            list_string.append(str(nodo)+' '+str(nod_adj) +' '+str(peso)+'\n')
      file = str(n)+"\n"+str(ejes)+"\n"
      file += "".join(list_string)
      file_ds = open("./nodos_"+str(n)+"-ejes_"+str(ejes)+"-densidad_"+str(densidad)+".txt", "w+")
      file_ds.write(file)
      file_ds.close()