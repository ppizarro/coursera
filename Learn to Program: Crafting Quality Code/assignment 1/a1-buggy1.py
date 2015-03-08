'''
Buggy implementations of A1 for Learn to Program: Crafting Quality Code
John Klein 4/2013

'''
def num_buses(n):
    """ (int) -> int

    Return the minimum number of buses required to transport n people.
    Each bus can hold 50 people.

    >>> num_buses(75)
    2
    """
    # this is a buggy implementation, because it returns a float not an int
    return n / 50

def stock_price_summary(price_changes):
    """ (list of number) -> (number, number) tuple

    price_changes contains a list of stock price changes. Return a 2-item
    tuple where the first item is the sum of the gains in price_changes and
    the second is the sum of the losses in price_changes.

    >>> stock_price_summary([0.01, 0.03, -0.02, -0.14, 0, 0, 0.10, -0.01])
    (0.14, -0.17)
    """
    gains = losses = 0
    for n in price_changes:
        if n > 0:
            gains = gains + n
        else:
            losses = losses + n
    # buggy, 'cause the tuple is in the wrong order
    return losses, gains

def swap_k(L, k):
    """ (list) -> NoneType

    Precondtion: k <= len(L) // 2

    Swap the first k items of L with the last k items of L.

    >>> nums = [1, 2, 3, 4, 5, 6]
    >>> swap_k(nums, 2)
    >>> nums
    [5, 6, 3, 4, 1, 2]
    """
    for i in range(k):
        # we should save L[i] before assigning it, but we don't
        L[i] = L[-i]
        # so this sets L[-i] to exactly what is was previously
        L[-i] = L[i]
        # now end of string overwrites beginning of string
    # let's add insult to injury by returning the list also
    return L



if __name__ == '__main__':
    import doctest
    doctest.testmod()
