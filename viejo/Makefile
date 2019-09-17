.PHONY: all clean

CC=g++
CPP_STANDARD=c++11
CFLAGS=-c -pthread -g -ggdb -std=$(CPP_STANDARD)
LFLAGS=-pthread -g -ggdb -std=$(CPP_STANDARD)

OBJS=grafo.o

all: TP1

TP1test: $(OBJS) tp1threads.cpp
	$(CC) $(LFLAGS) $(OBJS) tp1threads.cpp -o Analizador/TP1

TP1simple: $(OBJS) tp1.cpp
	$(CC) $(LFLAGS) $(OBJS) tp1.cpp -o Analizador/TP1simple

TP1: $(OBJS) tp1threads.cpp
	$(CC) $(LFLAGS) $(OBJS) tp1threads.cpp -o TP1

grafo.o: grafo.h grafo.cpp
	$(CC) $(CFLAGS) grafo.cpp -o grafo.o

clean:
	rm $(OBJS)
	rm TP1

test: TP1
	./TP1 $(input)

#time es cunatas ejecuciones van a correr los distintos casos 
#y folder es la carpeta con los grafos generados.
testGlobal: TP1test TP1simple
			python Analizador/tryAndTime.py  $(time) $(folder)