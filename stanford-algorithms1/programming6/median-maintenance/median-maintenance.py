'''
Programming assignment 6.2:

implement the "Median Maintenance" algorithm,
maintating the median of a stream of numbers,
arriving one by one, using O(log(n))
operations each step.

@author: Mikhail Dubov
'''

import heapq

heap_low = None
heap_high = None

def MedianMaintenance_init():
    
    global heap_low, heap_high
    
    heap_low = []
    heap_high = []
    
    
def MedianMaintenance_insert(x):
    '''
    Maintains the median of a stream
    of numbers in 'real-time'
    using O(log(n)) time for each
    insert operation.
    
    (implementation via heaps)
    '''
    
    global heap_low, heap_high
    
    # heapq is a MIN-heap, so we need 
    # to revert elements in heap_low
    # (which should be a MAX-heap).
    
    if (len(heap_low) == 0):
        heapq.heappush(heap_low, -x)
    else:
        m = -heap_low[0]
        if x > m:
            heapq.heappush(heap_high, x)
            if len(heap_high) > len(heap_low):
                y = heapq.heappop(heap_high)
                heapq.heappush(heap_low, -y)
        else:
            heapq.heappush(heap_low, -x)
            if len(heap_low) - len(heap_high) > 1:
                y = -heapq.heappop(heap_low)
                heapq.heappush(heap_high, y)
    
    return -heap_low[0]


def test():
    
    data = [1,5,2,4,3]
    
    MedianMaintenance_init()
    
    for x in data:
        print(MedianMaintenance_insert(x))
        

def main():
    
    lines = open('Median.txt').read().splitlines()
    data = map(lambda x: int(x), lines)
    medians = []
    
    MedianMaintenance_init()
    
    for x in data:
        median = MedianMaintenance_insert(x)
        medians.append(median)
        
    print(reduce(lambda x,y: (x+y) % 10000, medians))


if __name__ == '__main__':
    main()
    