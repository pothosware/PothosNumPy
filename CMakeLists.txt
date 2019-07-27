########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosSciPy CXX)

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
    TARGET SciPyBlocks
    DESTINATION PothosSciPy
    SOURCES
        Python/__init__.py
        Python/NumPyLoad.py
        Python/NumPySave.py
        Python/NumPySaveZ.py
        Python/SimpleBlocks.py
        Python/SimpleSource.py
        Python/SplineBlocks.py
    CPP_SOURCES

        Cpp/Factory.cpp
        Cpp/SciPyConstants.cpp

        Testing/TestConstants.cpp
        Testing/TestSimpleBlocks.cpp
        Testing/TestSimpleSources.cpp
)
