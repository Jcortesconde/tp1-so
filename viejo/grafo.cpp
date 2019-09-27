#include "grafo.h"
#include <fstream>

int Grafo::inicializar(string nombreArchivo) {

  ifstream archivo;
  archivo.open(nombreArchivo.c_str());
  if(!archivo) {
    cerr << nombreArchivo << " no existe en el directorio" << endl;
    return 0;
  }

  //Primera línea es la cantidad de vértices
  int numVertices;
  archivo >> numVertices;
  //La segunda la cantidad de ejes
  int numEjesArchivo;
  archivo >> numEjesArchivo;

  listaDeAdyacencias = map<int,vector<Eje>>();

  int v1, v2, peso;
  //Crear los ejes entre ambos nodos
  for(int ejes = 0; ejes < numEjesArchivo; ejes++){

    if(archivo.eof()){
      cerr << "Faltan ejes en el archivo" << endl;
      return 0;
    }

    archivo >> v1 >> v2 >> peso;

    insertarEje(v1,v2,peso);
  }

  archivo.close();
  return 1;
}

vector<Eje>::iterator Grafo::vecinosBegin(int num){
  return listaDeAdyacencias[num].begin();
}

vector<Eje>::iterator Grafo::vecinosEnd(int num){
  return listaDeAdyacencias[num].end();
}


void Grafo::AgregarNodo(int nodo){
  listaDeAdyacencias[nodo];
}

int Grafo::cantidadNodos(){
  listaDeAdyacencias.size();
} 

void Grafo::insertarEje(int nodoA, int nodoB, int peso){
    //Agrego eje de nodoA a nodoB
    Eje ejeA(nodoB,peso);
    listaDeAdyacencias[nodoA].push_back(ejeA);

    //Agrego eje de nodoB a nodoA
    Eje ejeB(nodoA,peso);
    listaDeAdyacencias[nodoB].push_back(ejeB);

    incrementarTotalEjes();
}

void Grafo::incrementarTotalEjes(){
  numEjes += 1;
}

int Grafo::damePeso(int nodoA, int nodoB){
  int peso = -1;
  for(std::vector<Eje>::iterator itEje= listaDeAdyacencias[nodoA].begin(); itEje!=listaDeAdyacencias[nodoA].end(); ++itEje){
    if((*itEje).nodoDestino == nodoB){
      if(peso == -1){
        peso = (*itEje).peso;
      }
      peso = peso < (*itEje).peso ? peso : (*itEje).peso;
    }
  }
  return peso;
}

int Grafo::pesoTotal(){
  int acum = 0;
   for (std::map<int,vector<Eje>>::iterator it=listaDeAdyacencias.begin(); it!=listaDeAdyacencias.end(); ++it){
    for(std::vector<Eje>::iterator itEje= it->second.begin(); itEje!=it->second.end(); ++itEje){
      acum += (*itEje).peso;
    }
  }
  return acum/2;
}

void Grafo::imprimirGrafo() {
  cerr << "Cantidad de nodos: " << cantidadNodos() << endl;
  cerr << "Cantidad de ejes: " << numEjes << endl;
  for (std::map<int,vector<Eje>>::iterator it=listaDeAdyacencias.begin(); it!=listaDeAdyacencias.end(); ++it){
    cerr << "\t" << it->first << ": - ";
    for(std::vector<Eje>::iterator itEje= it->second.begin(); itEje!=it->second.end(); ++itEje){
      cerr << "(" << (*itEje).nodoDestino << "," << (*itEje).peso << ") - ";
    }
    cerr<<"\n";
  }
}
