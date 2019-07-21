set(testSrcs
    TestNumPySqrt.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
POTHOS_MODULE_UTIL(
    TARGET SciPyTestBlocks
    SOURCES ${testSrcs}
    DESTINATION PothosSciPy
    ENABLE_DOCS
)
