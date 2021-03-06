#!/usr/bin/env py.test
# Copyright (C) 2011-2014 Anders Logg
#
# This file is part of DOLFIN.
#
# DOLFIN is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# DOLFIN is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.

from __future__ import print_function
import os, re
import pytest

def pytest_generate_tests(metafunc):
    # List of C++ tests
    cpp_tests = [dolfin_error, uint]

    # List of Python tests
    python_tests = [dolfin_error, raise_exception]

    # Set up paths
    pwd = os.path.dirname(os.path.abspath(__file__))
    topdir = os.path.join(pwd, "..", "..")

    # Check C++ files
    cpp_call = {'topdir': topdir,
                'subdir': "dolfin",
                'suffixes':[".cpp", ".h"]}

    python_call = {'topdir': topdir,
                   'subdir': "site-packages",
                   'suffixes':[".py"]}

    for test in python_tests:
        python_call['test'] = test
        metafunc.addcall(funcargs=python_call)

    for test in cpp_tests:
        cpp_call['test'] = test
        metafunc.addcall(funcargs=cpp_call)


def test_codingstyle(topdir, subdir, suffixes, test):
    "Main function for performing tests"

    # Iterate over all files
    for subdir, dirs, files in os.walk(os.path.join(topdir, subdir)):
        for filename in files:

            # Only consider files with given suffix
            if not len([1 for suffix in suffixes if filename.endswith(suffix)]) > 0:
                continue

            # Read file
            f = open(os.path.join(subdir, filename), "r")
            code = f.read()
            f.close()

            # Perform all tests
            result = test(code, filename)

def dolfin_error(code, filename):
    "Test for use of dolfin_error vs error"

    # Skip exceptions
    exceptions = ["log.h", "log.cpp", "Logger.h", "Logger.cpp",
                  "pugixml.cpp", "meshconvert.py"]
    if filename in exceptions:
        return True

    # Check for error(...)
    if re.search(r"\berror\(", code) is None:
        return True

    # Write an error message
    assert False, "*** error() used in %s when dolfin_error() should be used" % filename
    return False

def raise_exception(code, filename):
    "Test for use of dolfin_error vs raising exception"

    # Skip exceptions
    exceptions = ["meshconvert.py"]
    if filename in exceptions:
        return True

    # Check for raising of exception
    if re.search(r"\braise\b", code) is None:
        return True

    # Write an error message
    assert False, "* Warning: exception raised in %s when dolfin_error() should be used" % filename
    return False

def uint(code, filename):
    "Test for use of uint"

    # Skip exceptions
    exceptions = []
    if filename in exceptions:
        return True

    # Check for raising of exception
    if re.search(r"\buint\b", code) is None:
        return True

    # Write an error message
    assert False, "* Warning: uint is used in %s when std::size_t should be used" % filename
    return False

if __name__ == "__main__":
    pytest.main()
