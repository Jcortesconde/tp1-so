#include "grafo.h"
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <limits>
#include <stack>
#include <atomic>
#include <pthread.h>
#include <semaphore.h>
#include <chrono>

using namespace std;


// Estructura para los datos básicos de un thread.
struct ThreadInfo{
    int thread; // Número de thread.
    int threadId; // ID del thread.
};

// Datos para agregar a la cola de fusiones de cada thread.
struct FusionInfo{
    int thread;  // thread que se debe fusionar

    // El eje o arista a agregar:
    int fromNode;
    int toNode;
    int weight;
};

struct Thread{
 // TO DO

// Estructura que debe contener los colores de los vértices (actual y vecinos). Las distancias, el árbol, y la herramientas de sincronización necesarias para evitar race conditions y deadlocks.

};

// Imprimir el grafo resultado durante los experimentos
bool imprimirResultado = true;


// Se sugieren usar variables (unas atómicas y otras no) para:

// Contener el estado global de la estructura de threads.

// Para coordinar el número de cada thread durante la inizializacion de threads.

// Para para coordinar el id de cada thread durante la inizializacion y reinicializacion de threads.

// Para coordinar las modificaciones de los colores.

// Para contener la estructura global que indica el estado actual de cada nodo. 


//Retorna el nodo alcanzable a menor distancia
int buscarNodo(int thread){
  // TO DO
}


// Se pinta el nodo de negro para indicar que fue colocado en el árbol
void pintarNodo(int num, int thread){
   // TO DO

}

// Se pintan los vecinos de gris para marcar que son alcanzables desde el árbol (salvo los que ya son del árbol)
void pintarVecinos(Grafo *g, int num, int thread){
   // TO DO

}

//Reinicia las estructuras de un thread.
void reiniciarThread(int thread, Grafo* g){
    // TO DO
}


// Iniciar un thread.
int initThread(Grafo* g){
    // TO DO
}

void procesarNodo( int nodo, int thread, Grafo* g ){

    // TO DO. 

    // Procurar pintar nodo. 

    // Descubrir vecinos. 

    // Iniciar la gestión de funsiones.

}


// Trata de reservar el nodo que se pasa como parametro para el thread

ThreadInfo tomarNodo(int nodo, int thread){
  
   // TO DO
}

// Procurar agregar el thread con mayor id a la cola de fusiones del thread con menor id
void requestFuse(int thread, ThreadInfo other, int node){
    // TO DO

    // Se deben evitar race conditions, en los siguietes casos:
        // Un nodo hijo no puede estar en la cola de fusiones de otro nodo.
        // Solo se pueden agregar a la cola si el padre no está siendo fusionado por otro thread. 
}


// Realizar la fusión
void fuse(int parent, Grafo *g){

    // TO DO

    //Se determina el thread que tengo que fusionar

    //Se espera a que el thread esté listo para fusionarse

    //Se fusionan las colas de fusiones del hijo
 
    //Se fusionan los colores del hijo

    //Se fusionan las distancias del hijo

    //Fusionar Arboles
    
    //Se notifica al hijo que se termino la fusion
}

// Para buscar un nodo libre en el grafo.
int buscarNodoLibre(){
   // TO DO.
}


// Gestión principal del thread. Contiene el ciclo que le permite a cada thread hacer sus funciones.
void* mstParaleloThread(void *p){

    Grafo* g = (Grafo*) p;

    // Se obtiene el numero de thread y se inicializan sus estructuras

    int thread = initThread(g);

    // Se selecciona un nodo al azar del grafo para empezar.
    int nodoActual = rand() % g->numVertices;

    // Ciclo principal de cada thread
    while(true){

        // Se termina la ejecución si el grafo ya no tiene vertices libres. Se imprime el resultado y se termina el thread
        
        // Si el thread está en la cola de fusiones de otro thread, lo notifica que puede fusionarse. 
        
        // Se deben usar mecanismos de sincronización.
        
        // TO DO

	      // Si otro thread me está fusionando, esperar a que termine.

              		// Reinicializo las estructuras del thread y arranco de nuevo.
 
 	      // Si tiene elementos en la cola de fusion, debe fusionarlos.

             // Se busca el nodo más cercano que no esté en el árbol, pero que sea alcanzable
                  nodoActual = buscarNodo(thread);

             // Se procura reservar el nodo que se quiere tomar, indicando la apropiación en la estructura usada.

                 ThreadInfo tomarNodo(nodoActual, thread);

	     //Si se logra tomar, se procesa.
        
             //Si el nodo lo tiene otro thread, se tiene que fusionar

                  requestFuse(.....);
    }
}



void mstParalelo(Grafo *g, int cantThreads){

    //Verificar cantidad de threads para ejecutar el algoritmo

    if(cantThreads < 1){
        cerr << "El número de threads debe ser igual o mayor a 1" << endl;
    }

    //Si el numero de vertices del grafo es 0, imprimir el grafo vacio

    if(g->numVertices == 0){
        if(imprimirResultado){
            cout << endl << "********** RESULTADO *********** " << endl;
            Grafo().imprimirGrafo();
        }

        return;
    }

    // Se crean los threads 
    pthread_t threads[cantThreads];

    // Se inicializan las estructuras globales
    // TO DO.
    // Se deben usar pthread_create y pthread_join.
}

//Reinicia la experimentación.
void resetExperimentacion(){
   // TO DO 

}

//Procedimiento para realizar las pruebas o test mínimo de la cátedra.

void experimentacion(){
    imprimirResultado = false;
    std::cout << "instancia,n,grafo,threads, tiempo" << std::endl;
    int instancia = 0;
    string grafo;

    for (int n = 100; n<=1000 ; n += 100){
        for (int k = 0; k <=2; k++){
            Grafo g;
            if(k == 0){
                if( g.inicializar("test/experimentacion/arbol/arbol" + to_string(n) +".txt") != 1){
                    cerr << "No se pudo cargar el grafo correctamente" << endl;
                    return;
                }
            }
            if(k == 1){
                if( g.inicializar("test/experimentacion/ralo/ralo" + to_string(n) + ".txt") != 1){
                    cerr << "No se pudo cargar el grafo correctamente" << endl;
                    return;
                }
            }
            if(k == 2){
                if( g.inicializar("test/experimentacion/completo/completo" + to_string(n) + ".txt") != 1){
                    cerr << "No se pudo cargar el grafo correctamente" << endl;
                    return;
                }
            }
            for (int i = 0; i < 10; i++){

                if(k == 0){
                    grafo = "arbol";
                    auto start = std::chrono::steady_clock::now();
                    mstSecuencial(&g);
                    auto end = std::chrono::steady_clock::now();

                    std::cout << instancia << "," << n << "," << grafo << "," << 1 << ","
                              << std::chrono::duration <double, std::milli> (end-start).count()
                              << std::endl;
                    instancia++;
                    resetExperimentacion();
                }

                if(k == 1){
                    grafo = "ralo";
                    auto start = std::chrono::steady_clock::now();
                    mstSecuencial(&g);
                    auto end = std::chrono::steady_clock::now();

                    std::cout << instancia << "," << n << "," << grafo << "," << 1 << ","
                              << std::chrono::duration <double, std::milli> (end-start).count()
                              << std::endl;
                    instancia++;
                    resetExperimentacion();
                }

                if(k == 2){
                    grafo = "completo";
                    auto start = std::chrono::steady_clock::now();
                    mstSecuencial(&g);
                    auto end = std::chrono::steady_clock::now();

                    std::cout << instancia << "," << n << "," << grafo << "," << 1 << ","
                              << std::chrono::duration <double, std::milli> (end-start).count()
                              << std::endl;
                    instancia++;
                    resetExperimentacion();
                }
                for (int threads = 2; threads <= 32; threads *= 2){
                    if(k == 0){
                        grafo = "arbol";
                        auto start = std::chrono::steady_clock::now();
                        mstParalelo(&g, threads);
                        auto end = std::chrono::steady_clock::now();

                        std::cout << instancia << "," << n << "," << grafo << "," << threads << ","
                                  << std::chrono::duration <double, std::milli> (end-start).count()
                                  << std::endl;
                        instancia++;
                        resetExperimentacion();
                    }

                    if(k == 1){
                        grafo = "ralo";
                        auto start = std::chrono::steady_clock::now();
                        mstParalelo(&g, threads);
                        auto end = std::chrono::steady_clock::now();

                        std::cout << instancia << "," << n << "," << grafo << "," << threads << ","
                                  << std::chrono::duration <double, std::milli> (end-start).count()
                                  << std::endl;
                        instancia++;
                        resetExperimentacion();
                    }

                    if(k == 2){
                        grafo = "completo";
                        auto start = std::chrono::steady_clock::now();
                        mstParalelo(&g, threads);
                        auto end = std::chrono::steady_clock::now();

                        std::cout << instancia << "," << n << "," << grafo << "," << threads << ","
                                  << std::chrono::duration <double, std::milli> (end-start).count()
                                  << std::endl;
                        instancia++;
                        resetExperimentacion();
                    }
                }

            }
        }
    }


}

int main(int argc, char const * argv[]) {

    if(argc <= 1){
    	cerr << "Introduzca el nombre del archivo o el parámetro \"-e\" para hacer varias pruebas " << endl;
        return 1;
    }

    if (string(argv[1]) == "-e"){
        experimentacion();
        return 0;
    }

    string nombre;
    nombre = string(argv[1]);
    int threads = 1;
    if (argc > 2){
        algoritmo = string(argv[2]);
    }

    if (argc > 3){
        threads = atoi(argv[3]);
    }

    Grafo g;
    if( g.inicializar(nombre) == 1){

            mstParalelo(&g, threads);
	
    }else{
	    cerr << "Error: Grafo no cargado correctamente" << endl;
        return 1;
    }

  return 0;
}
