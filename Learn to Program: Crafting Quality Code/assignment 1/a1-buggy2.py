def num_buses(n):
    """ (int) -> int
    """
    if n < 1:
        return 0
    else:
        return int(n//50.1 + 1)



def stock_price_summary(price_changes):
    """ (list of number) -> (number, number) tuple
    """
    pos = 0.0
    neg = 0.0
    for price in price_changes:
        if price > 0:
            pos = pos + price
        elif price < 0:
            neg = neg + price

    return pos, neg



def swap_k(L, k):
    """ (list) -> NoneType
    """
    beginning = L[0:k]
    ending = L[-k:]
    L[0:k] = ending
    L[-k:] = beginning

if __name__ == '__main__':
    import doctest
    doctest.testmod()

