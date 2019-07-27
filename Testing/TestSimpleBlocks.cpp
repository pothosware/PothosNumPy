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

template <typename T>
using TestFunc = T(*)(T);

template <typename T>
using VectorTestFunc = std::vector<T>(*)(const std::vector<T>&);

template <typename T>
static std::vector<T> getArcTrigParams(size_t numInputs)
{
    return linspace<T>(-0.5f, 0.5f, numInputs);
}

template <typename T>
static SimpleBlockTestParams<T> getBaseTestParams(const std::string& blockRegistryPath)
{
    static const size_t numInputs = 123;

    const bool isArcTrig = (std::string::npos != blockRegistryPath.find("/arc"));

    SimpleBlockTestParams<T> testParams =
    {
        blockRegistryPath,
        isArcTrig ? getArcTrigParams<T>(numInputs) : linspace<T>(10.0f, 20.0f, numInputs),
        {},
        1e-6f
    };
    testParams.expectedOutputs.reserve(numInputs);

    return testParams;
}

template <typename T>
static SimpleBlockTestParams<T> getTestParamsForFunc(
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
static SimpleBlockTestParams<T> getTestParamsForVectorFunc(
    const std::string& blockRegistryPath,
    VectorTestFunc<T> func)
{
    auto testParams = getBaseTestParams<T>(blockRegistryPath);
    testParams.expectedOutputs = func(testParams.inputs);

    return testParams;
}

// TODO: make sure all blocks are tested
template <typename T>
static void testSimpleBlocksFloatingPoint()
{
    // STL implementations where functions don't exist
    auto reciprocal = [](T input){return T(1.0f) / input;};
    auto negative = [](T input){return std::abs(input) * T(-1.0f);};
    auto square = [](T input){return std::pow(input, T(2.0f));};

    auto flip = [](const std::vector<T>& input)
    {
        std::vector<T> output(input);
        std::reverse(output.begin(), output.end());
        return output;
    };

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
        std::abs));
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
    simpleBlockTest(getTestParamsForVectorFunc<T>(
        "/numpy/flip",
        flip));
}

// TODO: test all other types
POTHOS_TEST_BLOCK("/numpy/tests", test_simple_blocks)
{
    testSimpleBlocksFloatingPoint<float>();
    testSimpleBlocksFloatingPoint<double>();
}
