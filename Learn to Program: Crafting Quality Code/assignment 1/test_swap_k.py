#!/usr/bin/python3

import a1
import unittest


class TestSwapK(unittest.TestCase):
    """ Test class for function a1.swap_k. """

    def test_swap_k_empty_list(self):
        """ Test swap_k function with a empty list """
        L = []
        a1.swap_k(L, 0)
        expected = []
        self.assertEqual(L, expected)

    def test_swap_k_one_element(self):
        """ Test swap_k function with a list of the one element """
        L = [ 1 ]
        a1.swap_k(L, 0)
        expected = [ 1 ]
        self.assertEqual(L, expected)

    def test_swap_k_two_elements_and_k_zero(self):
        """ Test swap_k function with a list of the length 2 and k=0 """
        L = [ 1, 2 ]
        a1.swap_k(L, 0)
        expected = [ 1, 2 ]
        self.assertEqual(L, expected)

    def test_swap_k_two_elements_and_k_1(self):
        """ Test swap_k function with a list of the length 2 and k=len(L)//2 """
        L = [ 1, 2 ]
        a1.swap_k(L, 1)
        expected = [ 2, 1 ]
        self.assertEqual(L, expected)

    def test_swap_k_even_length_and_k_0(self):
        """ Test swap_k function with a even length list and k=0 """
        L = [ 1, 2, 3, 4 ]
        a1.swap_k(L, 0)
        expected = [ 1, 2, 3, 4 ]
        self.assertEqual(L, expected)

    def test_swap_k_even_length_and_k_maximum(self):
        """ Test swap_k function with a even length list and k=len(L)//2 """
        L = [ 1, 2, 3, 4 ]
        a1.swap_k(L, 2)
        expected = [ 3, 4, 1, 2 ]
        self.assertEqual(L, expected)

    def test_swap_k_even_length(self):
        """ Test swap_k function with a even length list and 0<k<len(L)//2 """
        L = [ 1, 2, 3, 4 ]
        a1.swap_k(L, 1)
        expected = [ 4, 2, 3, 1 ]
        self.assertEqual(L, expected)

    def test_swap_k_odd_length_and_k_0(self):
        """ Test swap_k function with a odd length list and k=0 """
        L = [ 1, 2, 3, 4, 5 ]
        a1.swap_k(L, 0)
        expected = [ 1, 2, 3, 4, 5 ]
        self.assertEqual(L, expected)

    def test_swap_k_odd_length_and_k_maximum(self):
        """ Test swap_k function with a odd length list and k=len(L)//2 """
        L = [ 1, 2, 3, 4, 5 ]
        a1.swap_k(L, 2)
        expected = [ 4, 5, 3, 1, 2 ]
        self.assertEqual(L, expected)

    def test_swap_k_odd_length(self):
        """ Test swap_k function with a odd length list and 0<k<len(L)//2 """
        L = [ 1, 2, 3, 4, 5 ]
        a1.swap_k(L, 1)
        expected = [ 5, 2, 3, 4, 1 ]
        self.assertEqual(L, expected)

if __name__ == '__main__':
    unittest.main(exit=False)
