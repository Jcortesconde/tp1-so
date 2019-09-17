	
#ifndef THREADS_H_
#define THREADS_H_

#include <iostream>
#include <vector>
#include <map>
#include <pthread.h>
#include <mutex>
#include "grafo.h"
#include <queue>
#include <atomic>

using namespace std;

//se llama queueInfo xq es lo que esta en la cola lol 
typedef struct queueInfo{
	int nodoThreadComida;
	int nodoThreadComedor;
	int otroThread;
	int peso;
	queueInfo(int a, int b, int p, int t){
		nodoThreadComida=a;
		nodoThreadComedor = b;
		peso=p;
		otroThread = t;
	}
}queueInfo;

typedef struct EstadoThread {
	int indice; //indice que identifica el thread y ademas representa el color del que pinta
	//el arbol de pintados.
	Grafo *pintados; 
	//colores pero interna del thread
	vector<int> nodos_recorridos;
	vector<int> distancia;
	vector<int> distanciaNodo;

   /*0 es el estado inicial
	*1 si puede ser comido
	*2 si ya fue agregado a una cola
	*-1 si se murio sin ser encolado
	este nos deja hablar entre el thread comido y los demas, asi no se encola dos veces y se come cuando ya termino de hacer cosas importantes
	*/
	atomic_int estado;
	//Quienes van a ser comidos por este thread.
	queue<queueInfo> threadsToEat;
	//no tendria que ser atomica, antes lo era, pero cuando se comen al padre puede ser que la lista este corrompida de forma no atomica
	atomic_int myEater; //si vale -1 entonces no fue comido, en otro caso fue comido por el valor que aparezca.
	pthread_mutex_t mutexMyEater;//myEater es una variable que define cosas como terminar el ciclo, o comer.
	pthread_mutex_t deadThread;//
	EstadoThread(int i, int cantNodos, int maxDistancia){
		indice=i;
		//Por ahora no colorié ninguno de los nodos
		distancia.resize(cantNodos);
		distancia.assign(cantNodos, maxDistancia);
		distanciaNodo.resize(cantNodos);
		distanciaNodo.assign(cantNodos,-1);
		nodos_recorridos.resize(cantNodos);
		nodos_recorridos.assign(cantNodos, -10);
		pintados = new Grafo();
		estado = 0;
		myEater = -1;
		pthread_mutex_init(&mutexMyEater, NULL);
		pthread_mutex_init(&deadThread, NULL);
	}
} EstadoThread;

#endif
