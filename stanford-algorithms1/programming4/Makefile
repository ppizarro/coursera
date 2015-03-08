all: strongly-connected-components

strongly-connected-components: strongly-connected-components.o vertex.o dlist.o
	gcc -O2 -Wall -o strongly-connected-components strongly-connected-components.c vertex.c dlist.c

clean:
	rm -rf *.o strongly-connected-components
