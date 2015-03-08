all: count-comparisons-first-pivot-quick-sort count-comparisons-last-pivot-quick-sort count-comparisons-median-of-three-pivot-quick-sort

count-comparisons-first-pivot-quick-sort: count-comparisons-first-pivot-quick-sort.o
	gcc -Wall count-comparisons-first-pivot-quick-sort.c -o count-comparisons-first-pivot-quick-sort

count-comparisons-last-pivot-quick-sort: count-comparisons-last-pivot-quick-sort.o
	gcc -Wall count-comparisons-last-pivot-quick-sort.c -o count-comparisons-last-pivot-quick-sort

count-comparisons-median-of-three-pivot-quick-sort: count-comparisons-median-of-three-pivot-quick-sort.o
	gcc -Wall count-comparisons-median-of-three-pivot-quick-sort.c -o count-comparisons-median-of-three-pivot-quick-sort

clean:
	rm -rf *.o count-comparisons-first-pivot-quick-sort count-comparisons-last-pivot-quick-sort count-comparisons-median-of-three-pivot-quick-sort
