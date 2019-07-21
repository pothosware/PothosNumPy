POTHOS_PYTHON_UTIL(
    TARGET SciPyBlocks
    SOURCES
        __init__.py
        NumPySqrt.py
    FACTORIES
        "/scipy/numpy/sqrt:NumPySqrt"
    DESTINATION PothosSciPy
    ENABLE_DOCS
)
