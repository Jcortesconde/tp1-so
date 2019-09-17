#ifndef GRAFO_H_
#define GRAFO_H_

#include <iostream>
#include <vector>
#include <map>
#include <pthread.h>
#include <mutex>

using namespace std;

typedef struct Eje {
  int nodoDestino;
  int peso;

  Eje(int nodoDestino, int peso) {
    this->nodoDestino = nodoDestino;
    this->peso = peso;
  }
  Eje():nodoDestino(0), peso(0) {}
} Eje;

class Grafo {

public:  
  int numEjes;

  map<int,vector<Eje>> listaDeAdyacencias;

  Grafo() {
    numEjes = 0;
  }

  void AgregarNodo(int nodo);
  int cantidadNodos();
  int inicializar(string nombreArchivo);
  void imprimirGrafo();
  vector<Eje>::iterator vecinosBegin(int num);
  vector<Eje>::iterator vecinosEnd(int num);
  void insertarEje(int nodoA, int nodoB, int peso);
  int damePeso(int nodoA, int nodoB);
  int pesoTotal();

private:
  void incrementarTotalEjes();
};

#endif
