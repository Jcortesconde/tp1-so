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
    atomic<bool> encolado;
    mutex puedo_pintar;
    mutex listo_para_encolar;
    threadParams(int index, int cantNodos){
    	nodoActual = -1;
    	arbol_local = Grafo();
    	id = index;
    	distancia.assign(cantNodos,IMAX);
		  distanciaNodo.assign(cantNodos,-1);
		  colores = vector<int>(cantNodos, BLANCO);
      bool unBool = false;
      encolado.store(unBool);
    }
};

struct infoMerge{
	threadParams * emisor;
	int nodo_emisor;
	int nodo_receptor;
	int peso;
	infoMerge(threadParams* emi, int n_emisor, int n_receptor, int p){
		emisor = emi;
		nodo_emisor = n_emisor;
		nodo_receptor = n_receptor;
		peso = p;
	}
};


void restart_thread(threadParams*);

//COLORES de los nodos
vector<int> g_colores;

vector<queue<infoMerge>> g_colas_fusion;
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
	for(int i = 0; i<toPrint.size(); i++){
		printf("(%d %d) ", i, toPrint[i]);
	}
	printf("\n");
}

void imprimirCola(queue<threadParams*> &toPrint){
	cout << "{ ";
	for(int i = 0; i < toPrint.size(); i++){
		cout << toPrint.front()->id << ", ";
		toPrint.pop();
	}
	cout << "}" << endl;
}


void pintarVecinos(int nodo, threadParams* param){

	for(vector<Eje>::iterator v = g_grafo.vecinosBegin(nodo); v != g_grafo.vecinosEnd(nodo); ++v){
	//Es la primera vez que descubro el nodo
	if(param->colores[v->nodoDestino] == BLANCO){
		//Lo marco como accesible
		param->colores[v->nodoDestino] = GRIS;
		//Le pongo el peso desde donde lo puedo acceder
		param->distancia[v->nodoDestino] = v->peso;
		//Guardo que nodo me garantiza esa distancia
		param->distanciaNodo[v->nodoDestino] = nodo;
	}else if(param->colores[v->nodoDestino] == GRIS){
		//Si ya era accesible, veo si encontré un camino más corto
		param->distancia[v->nodoDestino] = min(v->peso,param->distancia[v->nodoDestino]);
		//Guardo que nodo me garantiza esa distancia
		if(param->distancia[v->nodoDestino] == v->peso){
			param->distanciaNodo[v->nodoDestino] = nodo;
		}

	}
  }

}

//El emisor le pasa su data al receptor
void fusionarArboles(infoMerge& emisor_info, threadParams* receptor){
	//debugMutex.lock();
	threadParams * emisor = emisor_info.emisor;
  emisor->listo_para_encolar.lock();

	for(int i = 0; i < g_colores.size(); i++){
		if(emisor->colores[i] == emisor->id){
			assert(receptor -> colores[i] != receptor->id);
			receptor->colores[i] = receptor->id;
			receptor->distancia[i] = IMAX;
			receptor->distanciaNodo[i] = emisor->distanciaNodo[i];

			//cambio el color en el grafo
			g_colores_mutex[i].lock();
			g_colores[i] = receptor->id;
			g_colores_mutex[i].unlock();
		}
		if(emisor->colores[i] == GRIS && receptor->colores[i] != receptor->id){
			receptor->distancia[i] = receptor->distancia[i] < emisor->distancia[i]
								? receptor->distancia[i] : emisor->distancia[i];
			receptor->distanciaNodo[i] = receptor->distancia[i] < emisor->distancia[i]
								? receptor->distanciaNodo[i] : emisor->distanciaNodo[i];
			receptor->colores[i] = GRIS;

		}
	}

	auto cantEjes = 0;
	for(auto vertice : emisor->arbol_local.listaDeAdyacencias){
		for(auto eje : vertice.second){
			if(receptor->arbol_local.listaDeAdyacencias.count(vertice.first) != 0){
				receptor->arbol_local.listaDeAdyacencias.at(vertice.first).push_back(eje);
				cantEjes++;
			}else{
				receptor->arbol_local.listaDeAdyacencias.insert(make_pair(vertice.first, vector<Eje>()));
				receptor->arbol_local.listaDeAdyacencias.at(vertice.first).push_back(eje);
				cantEjes++;
			}
		}
	}
	receptor->arbol_local.numEjes += cantEjes / 2;

	if(receptor-> arbol_local.numVertices  > 0 && emisor -> arbol_local.numVertices  > 0){
		//Agregamos el eje sobre el que se hizo la fusion
		int nodo = emisor_info.nodo_receptor;
		int otroNodo = emisor_info.nodo_emisor;
		int pesoArista = emisor_info.peso;

		assert(nodo != -1);
		assert(otroNodo != -1);

		receptor->arbol_local.insertarEje(nodo, otroNodo, pesoArista);
	}
	receptor->arbol_local.numVertices += emisor->arbol_local.numVertices;
	auto numEjesPost = receptor->arbol_local.numEjes;

    g_colas_mutex[emisor->id].lock();

	//Fusiono las colas
	while(!g_colas_fusion[emisor->id].empty()){
		auto thread = g_colas_fusion[emisor->id].front();
		g_colas_fusion[emisor->id].pop();
    	g_colas_fusion[receptor->id].push(thread);
	}
	g_colas_mutex[emisor->id].unlock();
}



bool chequeoDeCola(threadParams* thread){
	g_colas_mutex[thread->id].lock();
	while(!g_colas_fusion[thread->id].empty()){
		infoMerge otroThread = g_colas_fusion[thread->id].front();
		g_colas_fusion[thread->id].pop();
		fusionarArboles(otroThread, thread);
		atendidos[(otroThread.emisor)->id] = true;
    (otroThread.emisor)->puedo_pintar.unlock();
	}
	g_colas_mutex[thread->id].unlock();
	return false;
}

bool meEstanEncolando(threadParams* param) {
  if (param->encolado.load()) {
    while(!atendidos[param->id]){}
    return true;
  }
  return false;
}

//retorno si pude pintar el nodo de mi color o no
bool pintarNodo(int num, threadParams* param){
	g_colores_mutex[num].lock();
	int colorFusion = g_colores[num];
	//printf("[%d] queria pintar %d y era de %d\n", param -> id, num, colorFusion);
	assert(colorFusion != param->id);
  bool expected = false;
  bool value = true;
	if(colorFusion != BLANCO){
		//Si mi id es mayor al del thread con el que colisione, me encolo y espero
 		if(colorFusion < param->id && param->encolado.compare_exchange_strong(expected, value)){
			//Lockeo la cola a la cual me voy a pushear
      		param->puedo_pintar.unlock();
			g_colas_mutex[g_colores[num]].lock();
			//Me pusheo a la cola
			infoMerge emisor(param, param->distanciaNodo[num], num,param->distancia[num]);
			g_colas_fusion[g_colores[num]].push(emisor);
			//Unlockeo la cola a la cual me pushie
			g_colas_mutex[g_colores[num]].unlock();

			// Espero a que me atiendan
			//Unlockeo el nodo por si otro tambien lo quiere pintar
			g_colores_mutex[num].unlock();
	 		while(!atendidos[param->id]){}
				return false;
		} else if (colorFusion < param->id) {
      while(!atendidos[param->id]){}
      return false;
    }
		else{
			//Unlockeo el nodo por si otro tambien lo quiere pintar
      threadParams* threadAComer = params[colorFusion];
    	g_colores_mutex[num].unlock();
      threadAComer->puedo_pintar.lock();
      if (threadAComer->encolado.compare_exchange_strong(expected, value)) {
        g_colas_mutex[param->id].lock();
  			//Pusheo al que voy a comer a la cola
  			infoMerge emisor(threadAComer, num, param->distanciaNodo[num],param->distancia[num]);
  			g_colas_fusion[param->id].push(emisor);
  			//Unlockeo la cola a la cual me pushie
  			g_colas_mutex[param->id].unlock();
      } else {
        threadAComer->puedo_pintar.unlock();
      }
      param->listo_para_encolar.lock();
			while(!g_colas_fusion[param->id].empty()){
				chequeoDeCola(param);
			};
			return true;
		}
	} else if(!meEstanEncolando(param)){
    if (param->id != 0) {
      //printf("[%d] pinté el nodo %d\n", param->id, num);
    }
      //Como voy a pintar, tengo que esperar a marcar mis vecinos antes de dejar que me coman
      param->listo_para_encolar.lock();
			param->arbol_local.numVertices += 1;
			g_colores[num] = param->id;
			param->colores[num] = param->id;
			if(param->arbol_local.numVertices > 1){
				param->arbol_local.insertarEje(num,param->distanciaNodo[num],param->distancia[num]);
			}
			else{
				param -> distanciaNodo[num] = -2;//es la raiz
			}
			param->distancia[num] = IMAX;
      g_colores_mutex[num].unlock();
      return true;
		}
  g_colores_mutex[num].unlock();
	return false;
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
	threadParams* param = ((threadParams*)p_param);
	param->nodoActual = rand() % g_grafo.numVertices;
	debugMutex.lock();
	debugMutex.unlock();
	  while(param->arbol_local.numVertices != g_grafo.numVertices){
		auto numEjes = param->arbol_local.numEjes;
	  	if(algunoTermino) goto end;
	  	int numeroVertices = param->arbol_local.numVertices;
		//Lo pinto de NEGRO para marcar que lo agregué al árbol y borro la distancia
    param->puedo_pintar.lock();
		if(meEstanEncolando(param) || !pintarNodo(param->nodoActual,param)){
		    goto restart;
		}
    param->puedo_pintar.unlock();
		//QUIERO ACTUALIZAR MIS VECINOS SOLO SI PINTE ALGO O ME FUSIONE, SI NO, NO!
		if(numeroVertices < param->arbol_local.numVertices) {
			//Descubrir vecinos: los pinto y calculo distancias
			//printf("[%d] busco pintar vecinos y actualizar nodo actual\n", param->id);
			if(param->colores[param->nodoActual] == param->id){
				//printf("[%d] pinta vecinos y actualiza nodo actual\n", param->id);

				pintarVecinos(param->nodoActual, param);
        //Una vez que termino de marcar mis vecinos estoy liste para que me coman
			//Busco el nodo más cercano que no esté en el árbol, pero sea alcanzable
			param->nodoActual = min_element(param->distancia.begin(),param->distancia.end()) - param->distancia.begin();
			}
		}
    param->listo_para_encolar.unlock();
	  }

	algunoTermino = true;
	//param.arbol_local.imprimirGrafo();
	assert(param->arbol_local.numEjes == (g_grafo.numVertices - 1));
	assert(g_grafo.numVertices == param->arbol_local.numVertices);
	printf("termino peso: %d\n",param->arbol_local.pesoTotal());
	for(int i = 0; i < atendidos.size(); i++){
		atendidos[i] = true;
	}
	restart:
	restart_thread( param);
  end:
  return nullptr;
}
void restart_thread( threadParams* param){
	param->arbol_local = Grafo();
	param->distancia.assign(g_grafo.numVertices,IMAX);
	param->distanciaNodo.assign(g_grafo.numVertices,-1);
	param->colores = vector<int>(g_grafo.numVertices, BLANCO);
  param->puedo_pintar.unlock();
  param->listo_para_encolar.unlock();
  bool encoladoReset = false;
  param->encolado.store(encoladoReset);
  atendidos[param->id] = false;
	mstThread(param);
}


void mstParalelo(int cantThreads){
	pthread_t thread[cantThreads];
	//Array de structs que contienen los parametros de los threads
    //Le asignamos un numero random unico a cada thread
	vector<int> randomNodes(g_grafo.numVertices);
	//INICIALIZACION DE VARIABLES GLOBALES
	g_colores = vector<int>(g_grafo.numVertices, BLANCO);
	g_colores_mutex = vector<mutex>(g_grafo.numVertices);
	g_colas_fusion = vector<queue<infoMerge>>(cantThreads);
	g_colas_mutex = vector<mutex>(cantThreads);
	g_fusion_mutex = vector<mutex>(cantThreads);
	atendidos = vector<bool>(cantThreads,false);

	params.resize(cantThreads);
	for(int i = 0; i < cantThreads; i++){
		params[i] = new threadParams(i, g_grafo.numVertices);
	}

	//Lanzamos los threads
    for (int i = cantThreads -1; i > -1; --i)
    	pthread_create(&thread[i], nullptr, mstThread, params[i]);
 	for (int i = cantThreads -1; i > -1; --i)
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
	 	  	mstSecuencial(&g_grafo);
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
