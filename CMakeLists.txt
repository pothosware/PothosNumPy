########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosNumPy CXX)

find_package(Pothos CONFIG REQUIRED)
include(PothosPythonUtil)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

########################################################################
# json.hpp header
########################################################################
find_path(JSON_HPP_INCLUDE_DIR NAMES json.hpp PATH_SUFFIXES nlohmann)

if (NOT JSON_HPP_INCLUDE_DIR)
    message(WARNING "Pothos SDR toolkit requires json.hpp, skipping...")
endif (NOT JSON_HPP_INCLUDE_DIR)

########################################################################
# Find Python modules used to implement blocks
########################################################################
include(FindPythonModule)
find_python_module(numpy REQUIRED)
find_python_module(scipy REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${JSON_HPP_INCLUDE_DIR})

include(BuildPythonModule)
BUILD_PYTHON_MODULE(
    TARGET NumPyBlocks
    DESTINATION PothosNumPy
    SOURCES
        Python/__init__.py
        Python/FFT.py
        Python/Load.py
        Python/OneToOneBlock.py
        Python/PostBufferBlock.py
        Python/RegisteredCallHelpers.py
        Python/Save.py
        Python/SaveZ.py
        Python/Source.py
        Python/TwoToOneBlock.py
        Python/Utility.py
    CPP_SOURCES
        Cpp/Factory.cpp
        Cpp/RegisteredCalls.cpp

        Testing/TestFFT.cpp
        Testing/TestRegisteredCalls.cpp
        Testing/TestSimpleBlocks.cpp
        Testing/TestSimpleSources.cpp
)
