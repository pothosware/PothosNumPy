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
    message(WARNING "Pothos NumPy toolkit requires json.hpp, skipping...")
endif (NOT JSON_HPP_INCLUDE_DIR)

########################################################################
# Find Python modules used to implement blocks
########################################################################
include(FindPythonModule)
find_python_module(numpy REQUIRED)
find_python_module(mako REQUIRED)
find_python_module(yaml REQUIRED)

########################################################################
# Auto-generate the majority of blocks
########################################################################

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/BlockGen)
set(autogenDeps
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Blocks/Arithmetic.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Blocks/Binary.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Blocks/Complex.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Blocks/CppOnly.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Blocks/Exponential.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Blocks/Random.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Blocks/Rounding.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Blocks/Sources.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Blocks/Special.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Blocks/Stats.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Blocks/Stream.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Blocks/Trig.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/Factory.mako.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/BlockExecutionTest.mako.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/PothosDoc.mako.py
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/PythonFactoryFunction.mako.py
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/PythonSubclass.mako.py
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/GenBlocks.py)
set(autogenOutputs
    ${CMAKE_CURRENT_BINARY_DIR}/BlockGen/BlockEntryPoints.py
    ${CMAKE_CURRENT_BINARY_DIR}/BlockGen/BlockExecutionTestAuto.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/BlockGen/Factory.cpp)
add_custom_command(
    OUTPUT ${autogenOutputs}
    DEPENDS ${autogenDeps}
    COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/BlockGen/GenBlocks.py" "${CMAKE_CURRENT_BINARY_DIR}/BlockGen"
    COMMENT "Auto-generating C++ factories and Python entry points")
add_custom_target(
    autogen_files ALL
    DEPENDS ${autogenOutputs})

########################################################################
# Build the module
########################################################################

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${JSON_HPP_INCLUDE_DIR})

include(BuildPythonModule)
BUILD_PYTHON_MODULE(
    TARGET NumPyBlocks
    DESTINATION PothosNumPy
    SOURCES
        Python/__init__.py
        Python/BaseBlock.py
        Python/FFT.py
        Python/ForwardAndPostLabelBlock.py
        Python/Load.py
        Python/NToOneBlock.py
        Python/OneToOneBlock.py
        Python/Random.py
        Python/RegisteredCallHelpers.py
        Python/Save.py
        Python/Source.py
        Python/TestFuncs.py
        Python/TwoToOneBlock.py
        Python/Utility.py
        Python/Window.py
        ${CMAKE_CURRENT_BINARY_DIR}/BlockGen/BlockEntryPoints.py
    CPP_SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/BlockGen/Factory.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/BlockGen/BlockExecutionTestAuto.cpp
        Cpp/NumericInfo.cpp
        Cpp/RegisteredCalls.cpp

        Testing/BlockExecutionTest.cpp
        Testing/BlockExecutionTestManual.cpp
        Testing/TestFFT.cpp
        Testing/TestLabels.cpp
        Testing/TestNumPyFileIO.cpp
        Testing/TestRegisteredCalls.cpp
        Testing/TestUtility.cpp
        Testing/TestWindow.cpp
    DOC_SOURCES
        ${autogenOutputs}
        Python/FFT.py
        Python/Load.py
        Python/Save.py
        Python/Window.py
)
add_dependencies(NumPyBlocks autogen_files)
