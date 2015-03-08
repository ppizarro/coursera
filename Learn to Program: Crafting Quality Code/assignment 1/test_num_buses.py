#!/usr/bin/python3

import a1
import unittest


class TestNumBuses(unittest.TestCase):
    """ Test class for function a1.num_buses. """

    def test_num_buses_required_transport_no_people(self):
        """ Test the number of buses required to transport no people """
        actual = a1.num_buses(0)
        expected = 0
        self.assertEqual(actual, expected)

    def test_num_buses_required_transport_less_50_people(self):
        """ Test the number of buses required to transport less than 50 and more than zero people """
        actual = a1.num_buses(12)
        expected = 1
        self.assertEqual(actual, expected)

    def test_num_buses_required_transport_50_people(self):
        """ Test the number of buses required to transport 50 people """
        actual = a1.num_buses(50)
        expected = 1
        self.assertEqual(actual, expected)

    def test_num_buses_required_transport_more_50_and_multiple_50_people(self):
        """ Test the number of buses required to transport more than 50 and multiple 50 people """
        actual = a1.num_buses(150)
        expected = 3
        self.assertEqual(actual, expected)

    def test_num_buses_required_transport_more_50_and_not_multiple_50_people(self):
        """ Test the number of buses required to transport more than 50 and not multiple 50 people """
        actual = a1.num_buses(501)
        expected = 11
        self.assertEqual(actual, expected)

if __name__ == '__main__':
    unittest.main(exit=False)
