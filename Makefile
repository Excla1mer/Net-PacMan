CC := gcc
LIBS := -lpthread -lcsfml-graphics -lcsfml-window -lcsfml-system

all: client_graph clean

client_graph: client_graph.o
	$(CC) client_graph.o -o bin/client_graph $(LIBS)

client_graph.o:
	$(CC) -c src/client/client_graph.c -o client_graph.o $(LIBS)

clean:
	rm -rf *.o
