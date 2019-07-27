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

#add_subdirectory(PothosSciPy)
#add_subdirectory(CppBlocks)

include(BuildPythonModule)
BUILD_PYTHON_MODULE(
    TARGET SciPyBlocks
    DESTINATION PothosSciPy
    SOURCES
        PothosSciPy/__init__.py
        PothosSciPy/NumPyLoad.py
        PothosSciPy/NumPySave.py
        PothosSciPy/NumPySaveZ.py
        PothosSciPy/SimpleBlocks.py
        PothosSciPy/SimpleSource.py
        PothosSciPy/SplineBlocks.py
    CPP_SOURCES
        Factory.cpp
)
