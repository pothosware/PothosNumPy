// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SimpleBlockTest.hpp"
#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <algorithm>
#include <cmath>
#include <complex>
#include <string>
#include <vector>

//
// Templated functions to use for output comparison
//

// Convenience
template <typename T>
static constexpr T pi()
{
    return T(M_PI);
}

template <typename T>
static constexpr T testDegrees(T rad)
{
    return T(rad * T(180.0f) / pi<T>());
}

template <typename T>
static constexpr T testRadians(T deg)
{
    return T(deg * pi<T>() / T(180.0f));
}

template <typename T>
static T testAbs(T input)
{
    return T(std::abs(float(input)));
}

template <typename T>
static T positive(T input)
{
    return input;
}

template <typename T>
static T negative(T input)
{
    return T(input * T(-1));
}

template <typename T>
static EnableIfTypeMatches<T, float, T> square(T input)
{
    return T(std::pow(input, T(2)));
}

template <typename T>
static EnableIfTypeDoesNotMatch<T, float, T> square(T input)
{
    return T(std::pow(double(input), T(2)));
}

template <typename T>
static EnableIfComplex<T, T> complexExpM1(const T& input)
{
    static const T oneComplex{1.0f,0.0f};

    return std::exp(input) - oneComplex;
}

template <typename T>
static EnableIfComplex<T, T> complexLog2(const T& input)
{
    static const T twoComplex{2.0f,0.0f};

    return std::log(input) / std::log(twoComplex);
}

template <typename T>
std::vector<T> flip(const std::vector<T>& input)
{
    std::vector<T> output(input);
    std::reverse(output.begin(), output.end());
    return output;
}

//
// Test functions
//

template <typename T>
using TestFunc = T(*)(T);

template <typename T>
using ComplexTestFunc = std::complex<T>(*)(const std::complex<T>&);

template <typename T>
using VectorTestFunc = std::vector<T>(*)(const std::vector<T>&);

template <typename T>
static std::vector<T> getArcTrigParams(size_t numInputs)
{
    return linspace<T>(-0.5f, 0.5f, numInputs);
}

template <typename T>
static EnableIfInteger<T, SimpleBlockTestParams<T>> getBaseTestParams(const std::string& blockRegistryPath)
{
    static constexpr T minValue = std::is_same<T, std::int8_t>::value ? T(-5) : T(-25);
    static constexpr size_t numInputs = std::is_same<T, std::int8_t>::value ? 11 : 51;

    SimpleBlockTestParams<T> testParams =
    {
        blockRegistryPath,
        getIntTestParams<T>(minValue, T(1), numInputs),
        {},
        T(0)
    };
    testParams.expectedOutputs.reserve(numInputs);

    return testParams;
}

template <typename T>
static EnableIfUnsignedInt<T, SimpleBlockTestParams<T>> getBaseTestParams(const std::string& blockRegistryPath)
{
    static constexpr T minValue = std::is_same<T, std::uint8_t>::value ? T(5) : T(25);
    static constexpr size_t numInputs = std::is_same<T, std::uint8_t>::value ? 9 : 76;

    SimpleBlockTestParams<T> testParams =
    {
        blockRegistryPath,
        getIntTestParams<T>(minValue, T(1), numInputs),
        {},
        T(0)
    };
    testParams.expectedOutputs.reserve(numInputs);

    return testParams;
}

template <typename T>
static EnableIfFloat<T, SimpleBlockTestParams<T>> getBaseTestParams(
    const std::string& blockRegistryPath,
    T epsilon = 1e-6f)
{
    // To not have nice even numbers
    static const size_t numInputs = 123;

    const bool isArcTrig = (std::string::npos != blockRegistryPath.find("/arc"));

    SimpleBlockTestParams<T> testParams =
    {
        blockRegistryPath,
        isArcTrig ? getArcTrigParams<T>(numInputs) : linspace<T>(10.0f, 20.0f, numInputs),
        {},
        epsilon
    };
    testParams.expectedOutputs.reserve(numInputs);

    return testParams;
}

template <typename T>
static EnableIfComplex<T, SimpleBlockTestParams<T>> getBaseTestParams(const std::string& blockRegistryPath)
{
    using Scalar = typename T::value_type;
    static_assert(std::is_floating_point<Scalar>::value);

    auto scalarTestInputs = getBaseTestParams<Scalar>(blockRegistryPath).inputs;

    // To get an even size
    if(0 != (scalarTestInputs.size() % 2)) scalarTestInputs.pop_back();

    SimpleBlockTestParams<T> testParams =
    {
        blockRegistryPath,
        toComplexVector(scalarTestInputs),
        {},
        {1e-6f,1e-6f}
    };
    testParams.expectedOutputs.reserve(testParams.inputs.size());

    return testParams;
}

template <typename T>
static EnableIfAnyInt<T, SimpleBlockTestParams<T>> getTestParamsForFunc(
    const std::string& blockRegistryPath,
    TestFunc<T> func)
{
    auto testParams = getBaseTestParams<T>(blockRegistryPath);
    std::transform(
        testParams.inputs.begin(),
        testParams.inputs.end(),
        std::back_inserter(testParams.expectedOutputs),
        func);

    return testParams;
}

template <typename T>
static EnableIfFloat<T, SimpleBlockTestParams<T>> getTestParamsForFunc(
    const std::string& blockRegistryPath,
    TestFunc<T> func,
    T epsilon = 1e-6f)
{
    auto testParams = getBaseTestParams<T>(blockRegistryPath, epsilon);
    std::transform(
        testParams.inputs.begin(),
        testParams.inputs.end(),
        std::back_inserter(testParams.expectedOutputs),
        func);

    return testParams;
}

template <typename T>
static EnableIfComplex<T, SimpleBlockTestParams<T>> getTestParamsForFunc(
    const std::string& blockRegistryPath,
    ComplexTestFunc<typename T::value_type> func)
{
    auto testParams = getBaseTestParams<T>(blockRegistryPath);
    std::transform(
        testParams.inputs.begin(),
        testParams.inputs.end(),
        std::back_inserter(testParams.expectedOutputs),
        func);

    return testParams;
}

template <typename T>
static SimpleBlockTestParams<T> getTestParamsForVectorFunc(
    const std::string& blockRegistryPath,
    VectorTestFunc<T> func)
{
    auto testParams = getBaseTestParams<T>(blockRegistryPath);
    testParams.expectedOutputs = func(testParams.inputs);

    return testParams;
}

//
// Test code
//

template <typename T>
static void testSimpleBlocksInt()
{
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/absolute",
        testAbs));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/positive",
        positive));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/negative",
        negative));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/square",
        square));
}

template <typename T>
static void testSimpleBlocksUInt()
{
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/square",
        square));
}

template <typename T>
static void testSimpleBlocksFloat()
{
    // STL implementations where functions don't exist
    auto reciprocal = [](T input){return T(1.0f) / input;};

    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/sin",
        std::sin));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/cos",
        std::cos));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/tan",
        std::tan));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/arcsin",
        std::asin));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/arccos",
        std::acos));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/arctan",
        std::atan));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/sinh",
        std::sinh));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/cosh",
        std::cosh));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/tanh",
        std::tanh));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/arcsinh",
        std::asinh));

    // TODO: separate getTestValues func to get valid values
    /*simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/arccosh",
        std::acosh));*/

    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/arctanh",
        std::atanh));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/floor",
        std::floor));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/ceil",
        std::ceil));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/trunc",
        std::trunc));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/exp",
        std::exp));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/expm1",
        std::expm1));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/exp2",
        std::exp2));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/log",
        std::log));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/log10",
        std::log10));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/log2",
        std::log2));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/log1p",
        std::log1p));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/reciprocal",
        reciprocal));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/positive",
        positive));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/negative",
        negative));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/sqrt",
        std::sqrt));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/cbrt",
        std::cbrt));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/square",
        square));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/absolute",
        std::abs));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/degrees",
        testDegrees,
        T(1e-3f)));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/radians",
        testRadians,
        T(1e-3f)));
    /*simpleBlockTest(getTestParamsForVectorFunc<T>(
        "/numpy/flip",
        flip));*/
}

template <typename T>
static void testSimpleBlocksComplex()
{
    static_assert(IsComplex<T>::value);

    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/exp",
        std::exp));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/expm1",
        complexExpM1));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/numpy/log2",
        complexLog2));
}

POTHOS_TEST_BLOCK("/numpy/tests", test_simple_blocks)
{
    testSimpleBlocksInt<std::int8_t>();
    testSimpleBlocksInt<std::int16_t>();
    testSimpleBlocksInt<std::int32_t>();
    testSimpleBlocksInt<std::int64_t>();

    testSimpleBlocksUInt<std::uint8_t>();
    testSimpleBlocksUInt<std::uint16_t>();
    testSimpleBlocksUInt<std::uint32_t>();
    testSimpleBlocksUInt<std::uint64_t>();

    testSimpleBlocksFloat<float>();
    testSimpleBlocksFloat<double>();

    testSimpleBlocksComplex<std::complex<float>>();
    testSimpleBlocksComplex<std::complex<double>>();
}
