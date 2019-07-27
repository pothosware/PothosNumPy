if(DEFINED INCLUDED_BUILD_PYTHON_MODULE_CMAKE)
    return()
endif()
set(INCLUDED_BUILD_PYTHON_MODULE_CMAKE TRUE)

# the directory which contains this CMake module
set(BUILD_PYTHON_MODULE_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}")

########################################################################
# Find python interp
########################################################################
find_package(PythonInterp)
if (NOT PYTHONINTERP_FOUND)
    message(WARNING "Python bindings require python exe, skipping...")
endif ()
message(STATUS "PYTHON_EXECUTABLE: ${PYTHON_EXECUTABLE}")

########################################################################
# Determine install directory
########################################################################
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c
    "from distutils.sysconfig import get_python_lib; print(get_python_lib(plat_specific=True, prefix=''))"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE POTHOS_PYTHON_DIR_SYSCONF
)
set(POTHOS_PYTHON_DIR "${POTHOS_PYTHON_DIR_SYSCONF}" CACHE STRING "python install prefix")

file(TO_CMAKE_PATH "${POTHOS_PYTHON_DIR}" POTHOS_PYTHON_DIR)
message(STATUS "POTHOS_PYTHON_DIR: \${prefix}/${POTHOS_PYTHON_DIR}")

if(NOT POTHOS_PYTHON_DIR)
    message(WARNING "Python: get_python_lib() extraction failed, skipping...")
endif(NOT POTHOS_PYTHON_DIR)

########################################################################
## BUILD_PYTHON_MODULE - build and install python modules for Pothos
##
## TARGET - the name of the module to build
## This module will contain the block factory registrations
## and block description markup parsed out of the sources.
##
## SOURCES - the list of python sources to install
##
## CPP_SOURCES - other C++ files to include in module
##
## DESTINATION - relative destination path
## This is the destination for the python sources and the module.
##
## DOC_SOURCES - an alternative list of sources to scan for docs
##
## ENABLE_DOCS - enable scanning of SOURCES for documentation markup.
##
## Most arguments are passed directly to the POTHOS_MODULE_UTIL()
## See documentation for POTHOS_MODULE_UTIL() in PothosUtil.cmake
########################################################################
function(BUILD_PYTHON_MODULE)

    include(CMakeParseArguments)
    CMAKE_PARSE_ARGUMENTS(BUILD_PYTHON_MODULE "ENABLE_DOCS" "TARGET;DESTINATION" "SOURCES;CPP_SOURCES;DOC_SOURCES" ${ARGN})

    #install python sources
    if (BUILD_PYTHON_MODULE_SOURCES)
        install(
            FILES ${BUILD_PYTHON_MODULE_SOURCES}
            DESTINATION ${POTHOS_PYTHON_DIR}/${BUILD_PYTHON_MODULE_DESTINATION}
        )
    endif()

    #build the module
    if (BUILD_PYTHON_MODULE_ENABLE_DOCS)
        set(BUILD_PYTHON_MODULE_ENABLE_DOCS "ENABLE_DOCS")
    else()
        unset(BUILD_PYTHON_MODULE_ENABLE_DOCS)
    endif()
    POTHOS_MODULE_UTIL(
        TARGET ${BUILD_PYTHON_MODULE_TARGET}
        DESTINATION ${BUILD_PYTHON_MODULE_DESTINATION}
        SOURCES ${BUILD_PYTHON_MODULE_CPP_SOURCES}
        DOC_SOURCES ${BUILD_PYTHON_MODULE_DOC_SOURCES}
        ${BUILD_PYTHON_MODULE_ENABLE_DOCS}
    )

endfunction(BUILD_PYTHON_MODULE)
