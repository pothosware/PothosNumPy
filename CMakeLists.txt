########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosNumPy CXX)

find_package(Pothos CONFIG REQUIRED)
include(PothosPythonUtil)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

########################################################################
# Find Python modules used to implement blocks
########################################################################
include(FindPythonModule)
find_python_module(numpy REQUIRED)
find_python_module(scipy REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(BuildPythonModule)
BUILD_PYTHON_MODULE(
    TARGET NumPyBlocks
    DESTINATION PothosNumPy
    SOURCES
        Python/__init__.py
        Python/Load.py
        Python/Save.py
        Python/SaveZ.py
        Python/SimpleBlocks.py
        Python/Source.py
    CPP_SOURCES
        Cpp/Factory.cpp

        Testing/TestSimpleBlocks.cpp
        Testing/TestSimpleSources.cpp
)
