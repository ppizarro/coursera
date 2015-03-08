# Coursera: Algorithms: Design and Analysis, Part 1 (Stanford)

### Karatsura: multiplicar inteiros

'''
 5678 x 1234

 a b  x c d

 a = 56
 b = 78
 c = 12
 d = 34

 5678 x 1234 = ( a*10^(n/2) + b) * ( c*10^(n/2) + d)
             = ac*10^n + ad*10^(n/2) + bc*10(n/2) + bd
             = ac*10^n + (ad + bc)*10^(n/2) + bd

 (ad + bc) = (a+b)*(c+d) - a*c - b*d
'''

Calcular:
1. a*c
1. b*d
1. (a+b)*(c+d)
1. (ad + bc) = (3) - (1) - (2)

Resposta: (1)*10^n + (4)*10^n/2 + (2)

Conclusão: com esse algoritmo faremos 3 multiplicações de (n/2) digitos

### Merge sort: O(n log(n))

dividir para conquistar algoritmo - 2(6nlog)n + 6n - very flat quickly

 log(2): #d times you divide by 2 until you get 1

 Proof of claim: At each level j=0,1,2,..., log(2)n, there are 2^j subproblems, each of size n/(2^j)
	Merger <= 6M

 <= 2^j (# of level-j subproblems) x 6 (n/(2^j)) (subproblem size at level j) = 6n (independent of j !!!) 

Total <= 6n *work per level) x (log(2)n + 1) (# of levels) = 6nlog(2)n + 6n

- Identity function: f(n) = n
- Quadratic function: f(n) = n^2

Outros piores que o Merge Sort:
- Selection Sort: n² (quadratic function)
- Insertion Sort: 1/2 x n²
- Bubble Sort: n²

Asymptotic analysis: focus on running time for large input sizes n

fast algorithm == worst-case running time grows slowly with input size

T(n) = O(f(n)) <= Cf(n), for a C and n >= n0

Claim: if T(n) = a(k)n^k + ... + a(1)n + a0 then T(n) = O(n^k)

Proof: Choose n0 = 1 and c= |a(k)| + |a(k-1)| + ... + |a1|

Need to show than qualquer n>=1, T(n) <= cn^k

we have, for every n>=1,

T(n) <= |a(k)|n^k + ... + |a(1)|n + |a(0)|

T(n) <= |a(k)|n^k + ... + |a(1)|n^k + |a(0)|n^k

T(n) <= cn^k

* O (O Big Notation) = less than or equal to (upper bound)
* Mega (Big omega notation) = greater than or equal to (lower bound)
* Theta (Big theta notation) = equal to

### Counting Inversions: O(n log(n))

Counting Inversions = # A[i] > A[j], where i<j

1, 2, 3, 4, 5, 6 = zero inversions

6, 5, 4, 3, 2, 1 = 15 inversions => n(n-1)/2

O(n) = n log n

### Strassen's Subcubic Matrix Multiplication Algorithm: O(n²)

O(n) = n^3

O(n) = n^2 (strassen)

### Algorithm for closest pair problem: O(n log(n))

O(n) = n log n

### Recurrence Format: Master Method or Master Theorem

 1. Base case: T(n) <= a constant for all sufficiently small n.
 1. For all large n: T(n) <= aT(n/b) + O(n^d)

	- a = number of recursive calls (>=1)
	- b = input size shrinkage factor (>1)
	- d = exponent in running time of "Combine step" (>=0)

	a,b,d independent of n

Master Method:

1. T(n) = O(n^d * log n) if a = b^d (case 1) todos os levels tem o mesmo trabalho
1. T(n) = O(n^d)         if a < b^d (case 2) a cada level o trabalho diminui, quase todo o trabalho está na raíz
1. T(n) = O(n^log(b)a)   if a > b^d (case 3) == O(a^log(b)n) a cada level o trabalho aumenta, quase todo o trabalho está nas folhas (a^log(b)n) # de folhas

### QuickSort

O(n) = n log n (average - with random pivots)

* Minimum #levels: Θ(log(n))
* Maximum #levels: Θ(n)

The best case is when the algorithm always picks the median as a pivot, in which case the recursion is essentially identical to that in MergeSort. In the worst case the min or the max is always chosen as the pivot, resulting in linear depth.

### Randomized Selection

RSelect(Array A, length n, order statistics i) 

Running time of RSelect <= 8 * c * n

O(n) = n

### Graphs and Minimum Cuts

Graphs:
- Vertices: aka nodes (v) [vertex]
- Edges: pairs of vertices (E)
    - Can be undirected (unordered pair)
    - or directed (ordered pair) - aka arcs (arrows)

Cuts of Graphs:
- Definition: a cut of a graph (V,E) is a partition of V into two non-empty sets A and B.

- Definition: the crossing edges of a cut (A,B) are those with:
    - one endpoint in each of (A, B) [undirected]
    - tail in A, head in B [directed]

The Minimum Cut Problem:
- Input: an undirected graph G = (V,E)
- Goal: compute a cut with fewest number of crossing edges (a min cut). Parallel edges allowed.

A Few Applications:
- identify network bottlenecks / weaknesses
- community detection in social networks
- image segmentation

n vertices => minimum #edges = n -1 and maximum #edges = n * (n - 1) / 2

Sparse X Dense Graphs

- n = # of vertices
- m = # of edges

### The Adjacency Matrix

Vertices = 1, 2, 3, 4, ...

A = n x n where Aij = 1 => Graph has an i-j edge

Variants:
- Aij = # of i-j edges (if parallel edges)
- Aij = weight of i-j edge (if any)
- Aij = +1 if i->j or -1 if i<-j

### Random Contraction Algorithm

P = 2 / (n*(n-1)) >= 1 / n^2 de escolher um edge cujo um dos vertices está no grupo A e o outro no grupo B.

probability fail = 1 - 1/(n^2)

Repeat N trials

Probabilty[all N trials fail] <= (1 - 1/n^2)^N

1 + x <= e^x

Pr[all N trial fail] <= (e^(-1/n^2))^N

So:
 - if N=n^2, Pr[all fail] <= (e^(-1/n^2))^(n^2) = 1/e
 - if N=n^2 * ln(n), Pr[all fail] <= (1/e)^ln n = 1/n

Running time: polynomial in n and m but slow (r(n²*m))

But: can get big speedups [to roughly O(n²)] with more ideas.

 # of minimum cuts 

[a tree with n vertices has (n - 1) minimum cuts]

Question: what's the largest number of min cuts that a graph with n vertices can have?

Answer: (n 2) = (n * (n - 1)) / 2

Pr[output = (Ai,Bi)] >= (2 / n * (n -1)) = 1 / (n 2)

### Graph Search

#### Breadth-First Search - BFS
- O(m + n) time using a queue (FIFO)
- explore nodes in "layers"
- can compute shortest paths
- can compute connected components of an undirected graph.

#### Depth-First Search - DFS
- O(m + n) time using a stack (LIFO) or via recursion
- explore aggressively like a maze, backtrack only when necessary.
- compute topological ordering of directed acyclic graph
- compute connected components in directed graphs

connected componentes = the "pieces" of graph. Equivalence classes of relation u~v <=> E u-v path in G

topologic ordering
- sequence tasks while respecting all precedence constraints.
- G has directed cycle => no topological ordering.
- every directed acycle graph has a sink vertex.

### strongly connected components - SCC
- SCC of a directed graph G are the equivalence classes of the relation
u~v <=> E path u -> v and E path v -> u in G
- kosaraju's Two-Pass Algorithm:
 1. Let Grev = G with all arcs reversed
 1. run DFS-Loop on Grev (goal: compute "magical ordering" of nodes) - let f(v)="finishing time" of each v e V.
 1. run DFS-Loop on G (goal: discover the SSCs one-by-one) - proceeding nodes in decreasing order of finishing times - SSCs = nodes with the same "leader"

Web Graph:
- vertices = web pages
- directed edges = hyperlinks
- Reference: [Broder et al www 2000]
- Recommended reading: Easley + Kleinberg, "Networks, Crowds, Markets"

### Dijkstra's Shortest-Path Algorithm

- each edge has nonnegative length l
- para aplicacoes com comprimento negativo, Dijkstra não funciona, seria melhor utilizar o Bellman Ford

while X != V:

Main loop cri'd
- Among all edges (v,w) E E with v pertencente X, w não pertencente a X, pick the one that minimizes
	A[v] + Lvw (Dijkstra's greedy criterion)
	A[v] => already computed 	in earlier interation
- Add w* to X
- Set A[w*] = A[v*] + Lv*w*
- Set B[w*] = B[v*] U (v*,w*)

- Naive implementation => run-Time: O(nm)

- Use data structure to get an algorithm speed-up.
- It's the Heap structure

- With Heaps => run-time: O(mlogn)

## Data Structures

- Point: organize data so that it can be accessed
- Examples: lists, stacks, queues, heaps, search trees, hash tables, bloom filters, union-find, etc.
- Why so many? different data structures support different sets of operations => suitable for different type of tasks.
- Rule of Thumb: choose the "minimal" data structure that supports all the operations that you need.

### Heap Data Structure

 a container for objects that have keys. Ex.: employer records, network edges, events, etc.

- INSERT: add a new object to a heap. Running time: O(log n)
- EXTRACT-MIN: remove an object in heap with a minimum key value. . Running time: O(log n) [n = # of objects in the heap]

Also:
- HEAPIFY (N batched Inserts in O(N) time ao inves de O(N log N))
- DELETE  (O(log N) time)

Canonical use of heaps: fast way to do repeated minimum computations

Example:

SelectionSort: o(n*n) quadratic - repetitive scans

#### HeadpSort:
1. insert all n array elements into a heap
1. Extract-Min to pluck at elements in sorted order

RunningTime = 2*N heap operations = O(N log N) time

=> optimal for a "comparison based" sorting algorithm]

Application: Event Manager
- "Priority Queue" - synonym for heap

Application: Median Maintanence
- two heaps => Hlow and Hhigh

Application: Speeding Up Dijkstra

Array Implementation:
- parent(i) = i/2 if i even, [i/2] if i odd (round down)
- children(i) = 2*i and 2*i+1

Implementation Insert (given key k):
- Step 1: stick K at the end of last level.
- Step 2: Bubble-Up (sift-up/heapify-up) k until heap property is restored (i.e., key of k's parent is <= k).

number of levels = log2 N (N = # of items in heap)

Implementation of Extract-Min (Bubble-Down)
- Step 1: delete root.
- Step 2: move last leaf to be new root.
- Step 3: interatively Bubble-Down until heap property has been restored. (always swap with smaller child!)

### Balanced Binary Search Tree

Sorted Arrays:

Operations		 					Running Time
- Search			 				  O(logN)
- Select (given order statistic i)      	        	  O(1)
- Min/Max							  O(1)
- Predecessor/Successor (given pointer to a key)		  O(1)
- Rank (i.e., # of keys less than or equal to a given value)	  O(logN)
- Output in sorted order                                          O(n)
- Insert and Deletions						  O(n)

Balanced Binary Search Tree = Sorted Arrays + Insert/Delete in O(logN) running time

Operations		 					Running Time
- Search			 				  O(logN)                                     => also supported by Hash table
- Select (given order statistic i)      	        	  O(logN) => quite slower than sorted arrays
- Min/Max							  O(logN) => quite slower than sorted arrays  => also supported by Heap
- Predecessor/Successor (given pointer to a key)		  O(logN) => quite slower than sorted arrays
- Rank (i.e., # of keys less than or equal to a given value)	  O(logN)
- Output in sorted order                                          O(n)
- Insert and Deletions						  O(logN) => faster than sorted arrays        => also supported by Heap, Hash table

O(log N) - balanced binary tree => height = log N

O(Height) - unbalanced binary tree

Exactly one node per key

Most basic version each node has:
- left child pointer: all key below this node has lower keys
- right child pointer: all key below this node has higher keys
- parent pointer

### Red-Black Trees
 [see also AVL trees, splay trees (self-adjusting trees), B trees, B+ trees (databases)]

1. each node red or black (bit information)
1. root is black
1. no two reds in a row (red node => only black childrens)
1. every root-NULL path (unsuccessful search) has same number of black nodes

left and right rotations

Insertion in a Red-Black Tree:

Idea for Insert/Delete: proceed as in a normal binary search tree, then recolor and/or perform rotations until invariants are restored;

1. Insert X as usual - makes X a leaf
1. Try coloring X red
1. If X's parent Y is black, done
1. else Y is red

### Hash Tables

Prupose: maintain a (possibly evolving) set of stuff. (transactions, people + associated data, IP address, etc.)

- Insert: Add a new record
- Delete: delete existing record
- Lookup: check for a particular record

Amazing guarantee => all operations in O(1) time! (constant time)

Collision => birsthday paradox (23 people -> 50% collision / same birsthday)

- Solution #1: chaining
- Solution #2: open addressing (one only object per bucket) => linear or using two hash functions: position + step

* performance depends on the choice of hash function!

Quick-and-Dirty Hash Functions:

"hash code" -> "compression function"

N = # of buckets

How to choose N?

1. choose N to be a prime (within constant factor of # of objects in table)
1. not too close to a power of 2
1. not too close to a power of 10

Load factor of a hash table: alpha

alpha = # of objects in hash table / # of buckets in hash table

* Upshot #1: for good hit performance, need to control load
* Upshot #2: for good hit performance, need a good hash function i.e., spreads data evenly across buckets.

Pathological Data in the Real World.

Solutions:

1. use a cryptographic hash function (e.q., SHA-2): infeasible to reverse engineer a pathological data set
2. use randomization: design a Family H of hash functions such that, any data sets S, "almost all" functions hash spread S out "pretty evenly". Universal Hashing.

### Bloom Filters

