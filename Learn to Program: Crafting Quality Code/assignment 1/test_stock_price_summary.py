#!/usr/bin/python3

import a1
import unittest


class TestStockPriceSummary(unittest.TestCase):
    """ Test class for function a1.stock_price_summary. """

    def test_stock_price_summary_with_empty_list(self):
        """ Test stock_price_summary function with a empty list """
        price_changes = []
        actual = a1.stock_price_summary(price_changes)
        expected = (0.0, 0.0)
        self.assertEqual(actual, expected)

    def test_stock_price_summary_with_one_positive_value(self):
        """ Test stock_price_summary function with a list
            with one positive value """
        price_changes = [ 1.0 ]
        actual = a1.stock_price_summary(price_changes)
        expected = (1.0, 0.0)
        self.assertEqual(actual, expected)

    def test_stock_price_summary_with_one_negative_value(self):
        """ Test stock_price_summary function with a list
            with one negative value """
        price_changes = [ -1.0 ]
        actual = a1.stock_price_summary(price_changes)
        expected = (0.0, -1.0)
        self.assertEqual(actual, expected)

    def test_stock_price_summary_with_multiple_positive_values(self):
        """ Test stock_price_summary function with a list
            with only multiple positive values """
        price_changes = [ 0.01, 0.03, 0.14 ]
        actual = a1.stock_price_summary(price_changes)
        expected = (0.18, 0.0)
        self.assertEqual(actual, expected)

    def test_stock_price_summary_with_multiple_negative_values(self):
        """ Test stock_price_summary function with a list
            with only multiple negative values """
        price_changes = [ -0.01, -0.03, -0.14 ]
        actual = a1.stock_price_summary(price_changes)
        expected = (0.0, -0.18)
        self.assertEqual(actual, expected)

    def test_stock_price_summary_with_mixed_positive_negative_values(self):
        """ Test stock_price_summary function with a list
            with mixed positive and negative values """
        price_changes = [ 0.01, -0.01, 0.03, -0.03, 0.14, -0.14 ]
        actual = a1.stock_price_summary(price_changes)
        expected = (0.18, -0.18)
        self.assertEqual(actual, expected)

if __name__ == '__main__':
    unittest.main(exit=False)
