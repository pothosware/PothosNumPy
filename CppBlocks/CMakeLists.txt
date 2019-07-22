set(cppSrcs
    TestSimpleBlocks.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
POTHOS_MODULE_UTIL(
    TARGET SciPyCppBlocks
    SOURCES ${cppSrcs}
    DESTINATION PothosSciPy
    ENABLE_DOCS
)
