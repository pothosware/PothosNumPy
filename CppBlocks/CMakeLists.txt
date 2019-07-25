set(cppSrcs
    SciPyConstants.cpp
    TestConstants.cpp
    TestSimpleBlocks.cpp
    TestSimpleSources.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
POTHOS_MODULE_UTIL(
    TARGET SciPyCppBlocks
    SOURCES ${cppSrcs}
    DESTINATION PothosSciPy
    ENABLE_DOCS
)
