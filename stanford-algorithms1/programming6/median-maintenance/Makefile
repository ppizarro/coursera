all: median-maintenance

median-maintenance: median-maintenance.o parse-file.o heap.o
	gcc -O2 -Wall -o median-maintenance median-maintenance.c parse-file.c heap.c

clean:
	rm -rf *.o median-maintenance
