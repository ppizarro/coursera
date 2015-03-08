all: random-contraction-min-cut

random-contraction-min-cut: random-contraction-min-cut.o parse-file.o vertex.o edge.o
	gcc -O2 -Wall random-contraction-min-cut.c parse-file.c vertex.c edge.c -o random-contraction-min-cut

clean:
	rm -rf *.o random-contraction-min-cut
