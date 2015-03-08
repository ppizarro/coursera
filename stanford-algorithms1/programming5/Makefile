all: heap-test dijkstra-shortest-path

heap-test: heap-test.o heap.o
	gcc -O2 -Wall -o heap-test heap-test.c heap.c

dijkstra-shortest-path: dijkstra-shortest-path.o parse-file.o heap.o vertex.o dlist.o
	gcc -O2 -Wall -o dijkstra-shortest-path dijkstra-shortest-path.c parse-file.c heap.c vertex.c dlist.c

clean:
	rm -rf *.o heap-test dijkstra-shortest-path
