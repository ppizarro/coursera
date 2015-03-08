all: 2sum-naive 2sum

%.o: %.c parse-file.h
	gcc -c $< -o $@ -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include

2sum-naive: 2sum-naive.o parse-file-naive.o
	gcc -O2 -Wall -o 2sum-naive 2sum-naive.o parse-file-naive.o

2sum: 2sum.o parse-file.o
	gcc -O2 -Wall -o 2sum 2sum.o parse-file.o -lglib-2.0

clean:
	rm -rf *.o 2sum-naive 2sum

