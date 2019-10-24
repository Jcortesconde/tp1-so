.PHONY: all clean

CC=g++
CPP_STANDARD=c++11
CFLAGS=-c -g -pthread -std=$(CPP_STANDARD)
LFLAGS=-g -pthread -std=$(CPP_STANDARD)

OBJS=grafo.o

all: TP1 TP1T TP1INV

TP1: $(OBJS) tp1.cpp
	$(CC) $(LFLAGS) $(OBJS) tp1.cpp -o TP1

TP1T: $(OBJS) tp1_testing.cpp
	$(CC) $(LFLAGS) $(OBJS) tp1_testing.cpp -o TP1_testing

TP1INV: $(OBJS) tp1_invariante.cpp
	$(CC) $(LFLAGS) $(OBJS) tp1_invariante.cpp -o TP1INV

grafo.o: grafo.h grafo.cpp
	$(CC) $(CFLAGS) grafo.cpp -o grafo.o

clean:
	rm $(OBJS)
	rm TP1
	rm TP1INV
	rm TP1_testing
test: TP1
	./TP1 $(input)
