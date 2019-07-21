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
