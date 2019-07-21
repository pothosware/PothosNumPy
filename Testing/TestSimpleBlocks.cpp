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

template <typename T>
using TestFunc = T(*)(T);

template <typename T>
static std::vector<T> getArcTrigParams(size_t numInputs)
{
    return linspace<T>(0.0f, 1.0f, numInputs);
}

template <typename T>
static SimpleBlockTestParams<T> getTestParamsForFunc(
    const std::string& blockRegistryPath,
    TestFunc<T> func)
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

    std::transform(
        testParams.inputs.begin(),
        testParams.inputs.end(),
        std::back_inserter(testParams.expectedOutputs),
        func);

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

    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/sin",
        std::sin));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/cos",
        std::cos));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/tan",
        std::tan));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/arcsin",
        std::asin));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/arccos",
        std::acos));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/arctan",
        std::atan));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/floor",
        std::floor));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/ceil",
        std::ceil));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/trunc",
        std::trunc));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/exp",
        std::exp));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/expm1",
        std::expm1));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/exp2",
        std::exp2));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/log",
        std::log));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/log10",
        std::log10));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/log2",
        std::log2));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/log1p",
        std::log1p));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/reciprocal",
        reciprocal));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/positive",
        std::abs));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/negative",
        negative));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/sqrt",
        std::sqrt));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/cbrt",
        std::cbrt));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/square",
        square));
    simpleBlockTest(getTestParamsForFunc<T>(
        "/scipy/numpy/absolute",
        std::abs));
}

// TODO: test all other types
POTHOS_TEST_BLOCK("/scipy/tests", TestSimpleBlocks)
{
    testSimpleBlocksFloatingPoint<float>();
    testSimpleBlocksFloatingPoint<double>();
}
