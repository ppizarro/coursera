def num_buses(n):
    """ (int) -> int

    Precondition: n >= 0

    Return the minimum number of buses required to transport n people.
    Each bus can hold 50 people.

    >>> num_buses(75)
    2
    """

    if n!= 0 and n < 50 :
        # covers children less than 50  and not equal to zero
        return 1
    if (n%50)== 0 :
        # covers zero and divisibles of 50
        return int(n/50)
    else :
        ## covers all the odd number of children example 51,103,etc
        return n// 50 + 1



def stock_price_summary(price_changes):
    """ (list of number) -> (number, number) tuple

    price_changes contains a list of stock price changes. Return a 2-item
    tuple where the first item is the sum of the gains in price_changes and
    the second is the sum of the losses in price_changes.

    >>> stock_price_summary([0.01, 0.03, -0.02, -0.14, 0, 0, 0.10, -0.01])
    (0.14, -0.17)
    """
    tuple_1 = 0
    tuple_2 = 0
    for i in price_changes :

        if i >= 0:
            tuple_1=round((tuple_1 + i),2)

        else:
            tuple_2=round((tuple_2 + i),2)

    return (tuple_1,tuple_2)

def swap_k(L, k):
    """ (list, int) -> NoneType

    Precondtion: 0 <= k <= len(L) // 2

    Swap the first k items of L with the last k items of L.

    >>> nums = [1, 2, 3, 4, 5, 6]
    >>> swap_k(nums, 2)
    >>> nums
    [5, 6, 3, 4, 1, 2]
    """
    new_list=L[:k]
    for i in L :
        for j in range(k):
            L[j] = L[-1-j]
    for o in new_list:

        for g in range(k):
            L[-1-g]=new_list[g]

    #print(L)
