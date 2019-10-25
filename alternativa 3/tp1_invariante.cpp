#include "grafo.h"
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <limits>
#include <pthread.h>
#include <stdio.h>
#include <mutex>
#include <queue>
#include <atomic>
#include <assert.h>
using namespace std;

#define BLANCO -10
#define GRIS -20
#define NEGRO -30

/* MAXIMO VALOR DE INT */
int IMAX = numeric_limits<int>::max();



struct threadParams{
    int id;
    int nodoActual;
    Grafo arbol_local;
    //Podria usarse un heap!
    vector<int> distancia; //(g->cantNodos, IMAX);
    vector<int> distanciaNodo;
    vector<int> colores;
    threadParams(int index, int cantNodos){
    	nodoActual = -1;
    	arbol_local = Grafo();
    	id = index;
    	distancia.assign(cantNodos,IMAX);
		distanciaNodo.assign(cantNodos,-1);
		colores = vector<int>(cantNodos, BLANCO);
    }
};


void restart_thread(threadParams);

//COLORES de los nodos
vector<int> g_colores;

vector<queue<threadParams*>> g_colas_fusion;
vector<threadParams*> params;

//Grafo (variable global)
Grafo g_grafo; 

//MUTEX DEL SISTEMA
vector<mutex> g_fusion_mutex;
vector<mutex> g_colores_mutex;
vector<mutex> g_colas_mutex;
//VECTOR DE ATENDIDOS
vector<bool> atendidos;
int itTotales = 0;
int itThread1 = 0;

//DISTANCIA mínima del nodo al árbol
vector<int> g_distancia;
vector<int> g_distanciaNodo;

//Mutex de debug
mutex debugMutex;

//Variable atomica
bool algunoTermino;


//Pinto el nodo de negro para marcar que fue puesto en el árbol
void pintarNodo(int num){
  g_colores[num] = NEGRO;
  //Para no volver a elegirlo
  g_distancia[num] = IMAX;
}

//Pinto los vecinos de gris para marcar que son alcanzables desde el árbol (salvo los que ya son del árbol)
void pintarVecinos(Grafo *g, int num){
  for(vector<Eje>::iterator v = g->vecinosBegin(num); v != g->vecinosEnd(num); ++v){
	//Es la primera vez que descubro el nodo
	if(g_colores[v->nodoDestino] == BLANCO){
		//Lo marco como accesible
		g_colores[v->nodoDestino] = GRIS;
		//Le pongo el peso desde donde lo puedo acceder
		g_distancia[v->nodoDestino] = v->peso;
		//Guardo que nodo me garantiza esa distancia
		g_distanciaNodo[v->nodoDestino] = num;
	}else if(g_colores[v->nodoDestino] == GRIS){
		//Si ya era accesible, veo si encontré un camino más corto
		g_distancia[v->nodoDestino] = min(v->peso,g_distancia[v->nodoDestino]);
		//Guardo que nodo me garantiza esa distancia
		if(g_distancia[v->nodoDestino] == v->peso)
		g_distanciaNodo[v->nodoDestino] = num;
	}
  }
}

void mstSecuencial(Grafo *g){
  //Imprimo el grafo
//   g->imprimirGrafo();

  //INICIALIZO
  //Semilla random
  srand(time(0));
  //Arbol resultado
  Grafo arbol;
  //Por ahora no colorié ninguno de los nodos
  g_colores.assign(g->numVertices,BLANCO);
  //Por ahora no calculé ninguna distancia
  g_distancia.assign(g->numVertices,IMAX);
  g_distanciaNodo.assign(g->numVertices,-1);

  //Selecciono un nodo al azar del grafo para empezar
  int nodoActual = rand() % g->numVertices;

  for(int i = 0; i < g->numVertices; i++){
	itTotales++;
	arbol.numVertices += 1;

	//La primera vez no lo agrego porque necesito dos nodos para unir
	if(i > 0){
	  arbol.insertarEje(nodoActual,g_distanciaNodo[nodoActual],g_distancia[nodoActual]);
	}

	//Lo pinto de NEGRO para marcar que lo agregué al árbol y borro la distancia
	pintarNodo(nodoActual);

	//Descubrir vecinos: los pinto y calculo distancias
	pintarVecinos(g,nodoActual);

	//Busco el nodo más cercano que no esté en el árbol, pero sea alcanzable
	nodoActual = min_element(g_distancia.begin(),g_distancia.end()) - g_distancia.begin();
  }
  cout << arbol.pesoTotal() << endl;
  //arbol.imprimirGrafo();
}

void imprimirVector(vector<int> &toPrint){
	for(auto i : toPrint){
		cout << i << " ";
	}
	cout << endl;
}

void imprimirCola(queue<threadParams*> &toPrint){
	cout << "{ ";
	for(int i = 0; i < toPrint.size(); i++){
		cout << toPrint.front()->id << ", ";
		toPrint.pop();
	}
	cout << "}" << endl;
}


void pintarVecinos(int nodo, threadParams &param){

	for(vector<Eje>::iterator v = g_grafo.vecinosBegin(nodo); v != g_grafo.vecinosEnd(nodo); ++v){
	//Es la primera vez que descubro el nodo
	if(param.colores[v->nodoDestino] == BLANCO){
		//Lo marco como accesible
		param.colores[v->nodoDestino] = GRIS;
		//Le pongo el peso desde donde lo puedo acceder
		param.distancia[v->nodoDestino] = v->peso;
		//Guardo que nodo me garantiza esa distancia
		param.distanciaNodo[v->nodoDestino] = nodo;
	}else if(param.colores[v->nodoDestino] == GRIS){
		//Si ya era accesible, veo si encontré un camino más corto
		param.distancia[v->nodoDestino] = min(v->peso,param.distancia[v->nodoDestino]);
		//Guardo que nodo me garantiza esa distancia
		if(param.distancia[v->nodoDestino] == v->peso){
			param.distanciaNodo[v->nodoDestino] = nodo;
		}
			
	}
  }	

}
//El emisor le pasa su data al receptor
void fusionarArboles(threadParams &emisor, threadParams &receptor){
	for(int i = 0; i < g_colores.size(); i++){
		if(emisor.colores[i] == emisor.id){
			receptor.colores[i] = receptor.id;
			receptor.distancia[i] = IMAX;
			//if(receptor.distanciaNodo[i] == -1) receptor.distanciaNodo[i] = emisor.distanciaNodo[i];
			
			//cambio el color en el grafo
			g_colores_mutex[i].lock();
			g_colores[i] = receptor.id;
			g_colores_mutex[i].unlock();
		}
		if(emisor.colores[i] == GRIS && receptor.colores[i] != receptor.id){
			receptor.distancia[i] = receptor.distancia[i] < emisor.distancia[i]
								? receptor.distancia[i] : emisor.distancia[i];
			receptor.distanciaNodo[i] = receptor.distancia[i] < emisor.distancia[i]
								? receptor.distanciaNodo[i] : emisor.distanciaNodo[i];					
			receptor.colores[i] = GRIS;
			
		}

	}

	auto cantEjes = 0;
	for(auto vertice : emisor.arbol_local.listaDeAdyacencias){
		for(auto eje : vertice.second){
			if(receptor.arbol_local.listaDeAdyacencias.count(vertice.first) != 0){
				receptor.arbol_local.listaDeAdyacencias.at(vertice.first).push_back(eje);
				cantEjes++;
			}else{
				receptor.arbol_local.listaDeAdyacencias.insert(make_pair(vertice.first, vector<Eje>()));
				receptor.arbol_local.listaDeAdyacencias.at(vertice.first).push_back(eje);
				cantEjes++;
			}
		}
	}
	receptor.arbol_local.numEjes += cantEjes / 2;

	//Agregamos el eje sobre el que se hizo la fusion
	int nodo, otroNodo, pesoArista;
	nodo = emisor.nodoActual;
	otroNodo = 	emisor.distanciaNodo[nodo];
	pesoArista = emisor.distancia[nodo];

	receptor.arbol_local.insertarEje(nodo, otroNodo, pesoArista);
	
	receptor.arbol_local.numVertices += emisor.arbol_local.numVertices;
	auto numEjesPost = receptor.arbol_local.numEjes;

    g_colas_mutex[emisor.id].lock();
    
	//Fusiono las colas
	while(!g_colas_fusion[emisor.id].empty()){
		auto thread = g_colas_fusion[emisor.id].front();
		g_colas_fusion[emisor.id].pop();
    	g_colas_fusion[receptor.id].push(thread);
	}
	g_colas_mutex[emisor.id].unlock();
}



bool chequeoDeCola(threadParams &thread){
	g_colas_mutex[thread.id].lock();
	while(!g_colas_fusion[thread.id].empty()){
		threadParams* otroThread = g_colas_fusion[thread.id].front();
		g_colas_fusion[thread.id].pop();
		fusionarArboles(*otroThread, thread);
		atendidos[otroThread->id] = true;
	}
	g_colas_mutex[thread.id].unlock();
	return false;
}

//retorno si pude pintar el nodo de mi color o no
bool pintarNodo(int num, threadParams &param){
	g_colores_mutex[num].lock();
	int colorFusion = g_colores[num];
	assert(colorFusion != param.id);
	if(colorFusion != BLANCO){
		//Si mi id es mayor al del thread con el que colisione, me encolo y espero
		if(colorFusion < param.id){
			//Lockeo la cola a la cual me voy a pushear
			g_colas_mutex[g_colores[num]].lock();
			//Me pusheo a la cola
			g_colas_fusion[g_colores[num]].push(&param);
			//Unlockeo la cola a la cual me pushie
			g_colas_mutex[g_colores[num]].unlock();

			// Espero a que me atiendan
			//Unlockeo el nodo por si otro tambien lo quiere pintar
			g_colores_mutex[num].unlock();
	        
			while(!atendidos[param.id]){}
				return false; 
		}
		else{
			//Unlockeo el nodo por si otro tambien lo quiere pintar
			g_colores_mutex[num].unlock();
			while(!g_colas_fusion[param.id].empty()){
				chequeoDeCola(param);
			};
			return true;
		}
	} else{
			param.arbol_local.numVertices += 1;
			g_colores[num] = param.id;
			param.colores[num] = param.id;
			if(param.arbol_local.numVertices > 1){
				param.arbol_local.insertarEje(num,param.distanciaNodo[num],param.distancia[num]);
			}
			param.distancia[num] = IMAX;
			
		}
    g_colores_mutex[num].unlock();
	return true;	
}

bool todoDeUnColor(int id){
	for(int i = 0; i < g_colores.size(); i++){
		g_colores_mutex[i].lock();
		if(g_colores[i] != id){
			g_colores_mutex[i].unlock();
			return false;
		}

		g_colores_mutex[i].unlock();
	}

	return true;
}

void* mstThread(void* p_param){
	auto pesoEjes = 0;
	threadParams param = *((threadParams*)p_param);
	param.nodoActual = 0;
	  while(param.arbol_local.numVertices != g_grafo.numVertices){
		auto numEjes = param.arbol_local.numEjes;
	  	if(algunoTermino) goto end;
	  	int numeroVertices = param.arbol_local.numVertices;
		//Lo pinto de NEGRO para marcar que lo agregué al árbol y borro la distancia
		if(!pintarNodo(param.nodoActual,param)){
		    goto restart;
		}
		
		//QUIERO ACTUALIZAR MIS VECINOS SOLO SI PINTE ALGO O ME FUSIONE, SI NO, NO!
		if(numeroVertices < param.arbol_local.numVertices){
			//Descubrir vecinos: los pinto y calculo distancias
			if(param.colores[param.nodoActual] == param.id){
				pintarVecinos(param.nodoActual, param);
			//Busco el nodo más cercano que no esté en el árbol, pero sea alcanzable
			
			param.nodoActual = min_element(param.distancia.begin(),param.distancia.end()) - param.distancia.begin();
			}
			

		}

	  }
	algunoTermino = true;
	//param.arbol_local.imprimirGrafo();
	assert(param.arbol_local.numEjes == (g_grafo.numVertices - 1));
	assert(g_grafo.numVertices == param.arbol_local.numVertices);
	cout << param.arbol_local.pesoTotal() << endl;
	for(int i = 0; i < atendidos.size(); i++){
		atendidos[i] = true;
	}
	restart:
	restart_thread( param);
    end:
    return nullptr;
}
void restart_thread( threadParams param){
	param.arbol_local = Grafo();
	param.distancia.assign(g_grafo.numVertices,IMAX);
	param.distanciaNodo.assign(g_grafo.numVertices,-1);
	param.colores = vector<int>(g_grafo.numVertices, BLANCO);
	mstThread(&param);
}


void mstParalelo(int cantThreads){
	pthread_t thread[cantThreads];
	//Array de structs que contienen los parametros de los threads
    //Le asignamos un numero random unico a cada thread
	vector<int> randomNodes(g_grafo.numVertices);
	//INICIALIZACION DE VARIABLES GLOBALES
	g_colores = vector<int>(g_grafo.numVertices, BLANCO);
	g_colores_mutex = vector<mutex>(g_grafo.numVertices);
	g_colas_fusion = vector<queue<threadParams*>>(cantThreads);
	g_colas_mutex = vector<mutex>(cantThreads);
	g_fusion_mutex = vector<mutex>(cantThreads);
	atendidos = vector<bool>(cantThreads,false);
	
	params.resize(cantThreads);
	for(int i = 0; i < cantThreads; i++){

		params[i] = new threadParams(i, g_grafo.numVertices);
	}

	for(int i = 0; i < cantThreads; i++){
		threadParams* param = params[i];
		//Descubrir vecinos: los pinto y calculo distancias
		pintarVecinos(param->nodoActual, *param);
		param->distancia[param->nodoActual] = IMAX;
		//Busco el nodo más cercano que no esté en el árbol, pero sea alcanzable
		param->nodoActual = min_element(param->distancia.begin(),
														param->distancia.end()) - param->distancia.begin();
	}


	//Lanzamos los threads
    for (int i = 0; i < cantThreads; ++i)
    	pthread_create(&thread[i], nullptr, mstThread, params[i]);
 	for (int i = 0; i < cantThreads; ++i)
    	pthread_join(thread[i], nullptr);
}


int main(int argc, char const * argv[]) {

  if(argc <= 2){
	cerr << "PARAM 1: Nombre de archivo" << endl;
	cerr << "PARAM 2: Cantidad de threads" << endl;
	return 0;
  }

   

  string nombre;
  nombre = string(argv[1]);
  int cantThreads = atoi(argv[2]);
  if(cantThreads == 1){
	  if(g_grafo.inicializar(nombre) == 1){
	 	  	(&g_grafo);
		  //cout << chrono::duration <double, milli> (diffSec).count() << ',';
	  }else{
		  cerr << "No se pudo cargar el grafo correctamente" << endl;
	  }
	  
  }else{
	  if( g_grafo.inicializar(nombre) == 1){
  		mstParalelo(cantThreads);
	}else{
		cerr << "No se pudo cargar el grafo correctamente" << endl;
	}
  }
  

	//cout << "Iniciando con " << cant_threads <<" threads" << endl;
  

  return 0;
}
