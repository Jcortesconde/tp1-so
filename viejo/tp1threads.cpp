#include "grafo.h"
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <limits>
#include "threads.h"
#include <atomic>
using namespace std;

int BLANCO = -10;
int GRIS = -20;

//COLORES de los nodos
vector<atomic_int *>  colores;

vector<pthread_t> threads;
vector<EstadoThread *> infoThreads;
vector<pthread_mutex_t> eating_mutex;//se utiliza para que se coma ordenadamente, se bloquea el comedor y el comido

pthread_mutex_t imprimir;
pthread_mutex_t eatingRootMutex;
pthread_mutex_t WaitToCreate;

Grafo * g = new Grafo();
int uno=1;
int cero=0;
int NOEATER = -1;

/* MAXIMO VALOR DE INT */
int IMAX = numeric_limits<int>::max()>>1;


//Pinto el nodo de negro para marcar que fue puesto en el árbol
int pintarNodo(int num, EstadoThread * threadInfo){
  int expected= BLANCO;
  int desired = threadInfo -> indice;
  bool pinte = colores[num] -> compare_exchange_strong(expected, desired);
    

  if(pinte){    
    threadInfo -> nodos_recorridos[num] = desired; 
    //Para no volver a elegirlo
    threadInfo -> distancia[num] = IMAX;
    threadInfo -> pintados -> AgregarNodo(num);
    return desired;
  }
  return expected;
}


//Pinto los vecinos de gris para marcar que son alcanzables desde el árbol (salvo los que ya son del árbol)
void pintarVecinos(int nodoActual, EstadoThread * threadInfo){
  for(vector<Eje>::iterator v = g -> vecinosBegin(nodoActual); v != g -> vecinosEnd(nodoActual); ++v){
  
    //Es la primera vez que descubro el nodo
    if(threadInfo -> nodos_recorridos[v->nodoDestino] == BLANCO){
      //Lo marco como accesible
      threadInfo -> nodos_recorridos[v->nodoDestino] = GRIS;
      //Le pongo el peso desde donde lo puedo acceder
      threadInfo -> distancia[v->nodoDestino] = v->peso;
      //Guardo que nodo me garantiza esa distancia
      threadInfo -> distanciaNodo[v->nodoDestino] = nodoActual;
    }else if(threadInfo -> nodos_recorridos[v->nodoDestino] == GRIS){
      //Si ya era accesible, veo si encontré un camino más corto
      threadInfo -> distancia[v->nodoDestino] = min(v->peso,threadInfo -> distancia[v->nodoDestino]);
      //Guardo que nodo me garantiza esa distancia
      if(threadInfo -> distancia[v->nodoDestino] == v->peso)
        threadInfo -> distanciaNodo[v->nodoDestino] = nodoActual;
    }
  }
}

void actualizarDistanciasDespuesDeComer(EstadoThread *otroThread, EstadoThread *thread){
  for(int i = 0; i<g->cantidadNodos(); i++){
    int posibleThread = otroThread-> nodos_recorridos[i];
    int posibleDistancia = otroThread-> distancia[i];
    int posibleDistanciaNodo = otroThread-> distanciaNodo[i];
    //es un posible nodo del thread comido y no esta pintado en el thread comedor
    if(posibleThread == GRIS && thread -> nodos_recorridos[i] < 0){
      thread -> nodos_recorridos[i] = GRIS;
      thread -> distancia[i] = min(posibleDistancia,thread -> distancia[i]);
      if(thread -> distancia[i] == posibleDistancia){
        thread -> distanciaNodo[i] = posibleDistanciaNodo;
      }
    }
  }
}

void comerCola(EstadoThread *otroThread, EstadoThread *thread){  
  while(!otroThread -> threadsToEat.empty()){
    queueInfo elemento = otroThread -> threadsToEat.front();
    otroThread -> threadsToEat.pop();
    thread -> threadsToEat.push(elemento);
  }
}
//si el nodo no tiene el color del thread, le cambia el color para q sea del thread
//si ya era del thread se lo pone igual
void actualizarNodo(int nodo, EstadoThread* thread){
  int myColor = thread -> indice;
  colores[nodo] -> exchange(myColor); 
  thread -> distancia[nodo] = IMAX;
  thread -> nodos_recorridos[nodo] = myColor;

  thread->pintados->AgregarNodo(nodo);  
}

void agregarUnNodoConEje(EstadoThread *thread,int nodoThreadComedor, int nodoThreadComida, int peso){
  thread -> pintados-> insertarEje(nodoThreadComida, nodoThreadComedor, peso);
    actualizarNodo(nodoThreadComedor, thread);
    actualizarNodo(nodoThreadComida, thread);
}


void copiarListaDeAdyacencia(map<int,vector<Eje>> listaAdyDeComida, EstadoThread *thread){
  for (std::map<int,vector<Eje>>::iterator it=listaAdyDeComida.begin(); it!=listaAdyDeComida.end(); ++it){
    int nodoComido = it-> first; 
    for(std::vector<Eje>::iterator itEje= it->second.begin(); itEje!=it->second.end(); ++itEje){
      Eje & eje = *itEje;     

      //hacemos esto para agregar solo una vez 
      if(nodoComido < eje.nodoDestino){
        //si ya existia el nodo en el grafo no hace nada, si no aumenta el contador.
        agregarUnNodoConEje(thread, nodoComido, eje.nodoDestino, eje.peso);

      }
    }
  }  
}


void comer(EstadoThread *thread){
  queueInfo comida = thread->threadsToEat.front();
  thread->threadsToEat.pop();

  pthread_mutex_lock(&imprimir);
  fprintf(stderr, "[%d] va comer a %d\n", thread -> indice, comida.otroThread);
  pthread_mutex_unlock(&imprimir);


  int myColor = thread -> indice;
  EstadoThread * otroThread = infoThreads[comida.otroThread];
  bool alreadyInOther = false;
 
  int cant = 0;
  //bussywaiting for the other thread to die.

  while(otroThread-> estado != 1){
    cant++;
  }


  otroThread -> estado = 0;


  int nodosThread = thread -> pintados -> cantidadNodos();
  int nodosOtroThread = otroThread -> pintados -> cantidadNodos();
  map<int, vector<Eje>> listaAdyDeComida = otroThread ->pintados -> listaDeAdyacencias;
  //si el otro thread  tiene ejes
if(otroThread->pintados->numEjes>0){
    copiarListaDeAdyacencia(listaAdyDeComida, thread );
    //tengo que agregar el eje que me conecta
  if(nodosThread > 0){

    agregarUnNodoConEje(thread, comida.nodoThreadComedor, comida.nodoThreadComida, comida.peso);
  }
}
else if(nodosThread > 0 && nodosOtroThread > 0){
  //tengo que agregar el eje que me conecta
    agregarUnNodoConEje(thread, comida.nodoThreadComedor, comida.nodoThreadComida, comida.peso);
  }
  //no tengo eje que conecta
  else{
    actualizarNodo(comida.nodoThreadComida, thread);
  }  
  //tengo que actualizar las distancias
  actualizarDistanciasDespuesDeComer(otroThread,thread);
  comerCola(otroThread,thread);

}

bool addToQueue(int threadQuePintara, EstadoThread* thread, int nodo, int nodoAdj){
  int added = 0;
  int yaMurio=-1;
  int myColor = thread -> indice;
  //si nadie lo agrego a su cola lo agrego yo
  if(infoThreads[threadQuePintara]-> estado.compare_exchange_strong(added, 2) 
    || infoThreads[threadQuePintara]-> estado.compare_exchange_strong(yaMurio, 1)){
    queueInfo nuevaQueueInfo(nodo,
              nodoAdj,
              g -> damePeso(nodo,nodoAdj),
              threadQuePintara);    
    pthread_mutex_lock(&eating_mutex[myColor]);

    thread->threadsToEat.push(nuevaQueueInfo);

    pthread_mutex_unlock(&eating_mutex[myColor]);
  }
}

void eatHandler(EstadoThread * thread,int threadQuePinto, int nodo, int nodoAdj){

  pthread_mutex_lock(&imprimir);
  fprintf(stderr, "[%d] esta handeleando a %d\n", thread -> indice, threadQuePinto);
  pthread_mutex_unlock(&imprimir);

  if (thread->indice > threadQuePinto){
    thread->myEater.exchange(threadQuePinto);
    addToQueue(thread->indice, infoThreads[threadQuePinto],nodo, nodoAdj);
    
    pthread_mutex_lock(&imprimir);
    fprintf(stderr, "[%d] va a ser comido por %d\n", thread -> indice, threadQuePinto);
    pthread_mutex_unlock(&imprimir);

  }
  else{
    int supremeEater= infoThreads[threadQuePinto]->myEater;    
    int expected = -1;
    int value = thread->indice;
    //supreme = 0
    //threadQuePinto = 5

    pthread_mutex_lock(&imprimir);
    fprintf(stderr, "[%d] tiene que comer a %d, o a su comedor mas grande\n", thread -> indice, threadQuePinto);
    pthread_mutex_unlock(&imprimir);

    while(! infoThreads[threadQuePinto]->myEater.compare_exchange_strong(expected, value)){
      if(  expected < thread->indice){
      pthread_mutex_lock(&imprimir);
      fprintf(stderr, "[%d] es mas chico %d?\n", thread -> indice,  supremeEater);
      pthread_mutex_unlock(&imprimir);
          break;
      }
      threadQuePinto = supremeEater;
      supremeEater = infoThreads[threadQuePinto]->myEater;
      expected = -1;
    } 
    if(thread -> indice != supremeEater){
      if(supremeEater==-1){
          pthread_mutex_lock(&imprimir);
          fprintf(stderr, "[%d] es el primero de esta cadena, el anterior fue %d\n", thread -> indice, threadQuePinto);
          pthread_mutex_unlock(&imprimir);
         addToQueue(threadQuePinto,thread, nodo,nodoAdj);
      }
      else{

        pthread_mutex_lock(&imprimir);
        fprintf(stderr, "[%d] habia alguien mas chico %d, me comen\n", thread -> indice,  supremeEater);
        pthread_mutex_unlock(&imprimir);
        thread->myEater.exchange(supremeEater);
        addToQueue(thread->indice,infoThreads[supremeEater],nodo, nodoAdj);
      }
    }
  }
}

void * mstSecuencial(void *thread_void){

  //INICIALIZO
  EstadoThread * threadInfo = (EstadoThread *) thread_void;
  int myColor = threadInfo->indice;
  // Semilla random
  srand(time(0));
  //Selecciono un nodo al azar del grafo para empezar
  int nodoActual = rand() % g -> cantidadNodos();
  int valor = 0;//var de debugging
  
  for(int i = 0;  i< g->cantidadNodos()*2
                  && threadInfo->pintados->numEjes < g -> cantidadNodos() -1 
                  && threadInfo -> estado == 0;  i++){
    bool comimos = false;

    pthread_mutex_lock(&eating_mutex[myColor]);
    pthread_mutex_lock(&imprimir);
    fprintf(stderr, "[%d] esta por comer\n", myColor);
    pthread_mutex_unlock(&imprimir);

    while(!threadInfo->threadsToEat.empty()){

      comer(threadInfo);
      comimos = true;
    }   

    pthread_mutex_unlock(&eating_mutex[myColor]);

    pthread_mutex_lock(&imprimir);
    fprintf(stderr, "[%d] dejo de comer\n", myColor);
    pthread_mutex_unlock(&imprimir);


    if(threadInfo->pintados->numEjes == g -> cantidadNodos() -1 ){
      break;
    }

    if(comimos){
      nodoActual = min_element(threadInfo-> distancia.begin(),threadInfo-> distancia.end()) - threadInfo -> distancia.begin(); 
    }  
    //Lo pinto de NEGRO para marcar que lo agregué al árbol y borro la distancia
    int threadQuePinto = pintarNodo(nodoActual, threadInfo);
    if(threadQuePinto != myColor){ // esta pintado x un thread y ese thread no es este.

      if(threadInfo -> myEater == -1){   
        pthread_mutex_lock(&imprimir);
        fprintf(stderr, "[%d] se encontro a %d\n", myColor, threadQuePinto);
        pthread_mutex_unlock(&imprimir);

        eatHandler(threadInfo,threadQuePinto,nodoActual, threadInfo->distanciaNodo[nodoActual]);

        pthread_mutex_lock(&imprimir);
        fprintf(stderr, "[%d] handeleo la comida con %d\n", myColor, threadQuePinto);
        pthread_mutex_unlock(&imprimir);
      }
    }
    else{
      //La primera vez no lo agrego porque necesito dos nodos para unir

      pthread_mutex_lock(&imprimir);
      fprintf(stderr, "[%d] agrega el nodo %d\n", myColor, nodoActual);
      pthread_mutex_unlock(&imprimir);
      int otroNodo = threadInfo ->  distanciaNodo[nodoActual];
      if(otroNodo != -1){
          threadInfo -> pintados -> insertarEje(nodoActual, otroNodo, g -> damePeso(nodoActual,otroNodo));
      }
    }
  

  //Descubrir vecinos: los pinto y calculo distancias
  pintarVecinos(nodoActual, threadInfo);
    //Busco el nodo más cercano que no esté en el árbol, pero sea alcanzable
  nodoActual = min_element(threadInfo-> distancia.begin(),threadInfo-> distancia.end()) - threadInfo -> distancia.begin();
    

  }
  
  if(myColor == 0){
   //LO AGREGAMOS PARA CASOS DE GRAFOS CHICOS DONDE 0 PUEDE PINTAR ATODOS LOS NODOS ANTES DE QUE LOS DEMAS PINTEN.
    //SOLO PARA QUE EL OUTPUT DE LA ULTIMA LINEA SEA 0.
    pthread_mutex_lock(&WaitToCreate);
    for(int i = 1; i < threads.size(); i++){
      int error = pthread_join(threads[i], NULL);
      }
    pthread_mutex_unlock(&WaitToCreate);
  }
  pthread_mutex_lock(&eating_mutex[myColor]);

  pthread_mutex_lock(&imprimir);
  fprintf(stderr, "[%d] Ultima cena\n", myColor);
  pthread_mutex_unlock(&imprimir);
  while(!threadInfo->threadsToEat.empty()){
    comer(threadInfo);
  }
  
  pthread_mutex_lock(&imprimir);
  fprintf(stderr, "[%d] Ultima cena terminada\n", myColor);
  pthread_mutex_unlock(&imprimir);

  pthread_mutex_unlock(&eating_mutex[myColor]);  
  int encolado=2;
  int inicializado=0;
  pthread_mutex_lock(&infoThreads[myColor]->deadThread);
  threadInfo -> estado.compare_exchange_strong(encolado,1);
  threadInfo->estado.compare_exchange_strong(inicializado,-1);  
  pthread_mutex_unlock(&infoThreads[myColor]->deadThread);

  pthread_mutex_lock(&imprimir);
  fprintf(stderr, "OUTPUT %d %d %d %d\n", myColor, threadInfo -> pintados -> cantidadNodos(),threadInfo -> pintados -> numEjes, threadInfo -> pintados -> pesoTotal());
  pthread_mutex_unlock(&imprimir);

}

int main(int argc, char const * argv[]) {
  int cantThreads = 5;
  if(argc <= 2){
  cerr << "Ingrese nombre de archivo y cantidad de threads" << endl;
  return 0;
  }

  string nombre;
  nombre = string(argv[1]);
  cantThreads = atoi(argv[2]);

  if( g -> inicializar(nombre) == 1){

  //Corro el algoirtmo secuencial de g

  //Imprimo el grafo
  g -> imprimirGrafo();

  //Arbol resultado
  //Por ahora no colorié ninguno de los nodos
  colores.resize(g-> cantidadNodos());

  for(int i=0; i<g->cantidadNodos(); i++){
    int aux  = BLANCO;
   
    colores[i]= new atomic_int();
    *colores[i]= aux;
    
  }

  infoThreads.resize(cantThreads);
  eating_mutex.resize(cantThreads);
  for(int i = 0; i<cantThreads; i++){
    pthread_mutex_init(&eating_mutex[i], NULL);
    infoThreads[i] = new EstadoThread(i, g -> cantidadNodos(), IMAX);
  }

  pthread_mutex_init(&imprimir, NULL);
  pthread_mutex_init(&eatingRootMutex, NULL);
  pthread_mutex_init(&WaitToCreate, NULL);

  threads.resize(cantThreads);
  
  pthread_mutex_lock(&WaitToCreate);
  for(int i = 0; i<cantThreads; i++){
    pthread_create(&(threads)[i], NULL, mstSecuencial, (void *) infoThreads[i]);
  }
  pthread_mutex_unlock(&WaitToCreate);
     
  pthread_join((threads)[0], NULL);    

  } else {
      cerr << "No se pudo cargar el grafo correctamente" << endl;
  }

  return 0;
}