// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TwoToOneBlockTest.hpp"
#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <algorithm>
#include <cmath>
#include <complex>
#include <functional>
#include <string>
#include <tuple>

#include <tr1/cmath>

//
// Test functions
//

template <typename T>
using BinaryFunc = std::binary_function<T, T, T>;

template <typename T>
static std::vector<T> removeZero(const std::vector<T>& inputs)
{
    static const auto NotEqualTo = std::not_equal_to<T>();

    std::vector<T> filtered;
    filtered.reserve(inputs.size());
    std::copy_if(
        inputs.begin(),
        inputs.end(),
        std::back_inserter(filtered),
        [](T val){return NotEqualTo(T(0), val);});

    return filtered;
}

template <typename T>
static EnableIfInteger<T, TwoToOneBlockTestParams<T>> getBaseTestParams(const std::string& blockRegistryPath)
{
    static constexpr T minValue = std::is_same<T, std::int8_t>::value ? T(-5) : T(-25);
    static constexpr size_t numInputs = std::is_same<T, std::int8_t>::value ? 11 : 51;

    // Add an extra one because we're removing 0.
    TwoToOneBlockTestParams<T> testParams =
    {
        blockRegistryPath,
        removeZero(getIntTestParams<T>(minValue, T(1), numInputs+1)),
        removeZero(flip(getIntTestParams<T>(minValue, T(1), numInputs+1))),
        {},
        T(0)
    };
    testParams.expectedOutputs.reserve(numInputs);

    return testParams;
}

template <typename T>
static EnableIfUnsignedInt<T, TwoToOneBlockTestParams<T>> getBaseTestParams(const std::string& blockRegistryPath)
{
    static constexpr T minValue = std::is_same<T, std::uint8_t>::value ? T(5) : T(25);
    static constexpr size_t numInputs = std::is_same<T, std::uint8_t>::value ? 9 : 76;

    // Add an extra one because we're removing 0.
    TwoToOneBlockTestParams<T> testParams =
    {
        blockRegistryPath,
        removeZero(getIntTestParams<T>(minValue, T(1), numInputs+1)),
        removeZero(flip(getIntTestParams<T>(minValue, T(1), numInputs+1))),
        {},
        T(0)
    };
    testParams.expectedOutputs.reserve(numInputs);

    return testParams;
}

template <typename T>
static EnableIfFloat<T, TwoToOneBlockTestParams<T>> getBaseTestParams(
    const std::string& blockRegistryPath,
    T epsilon = 1e-6f)
{
    // To not have nice even numbers
    static const size_t numInputs = 123;

    TwoToOneBlockTestParams<T> testParams =
    {
        blockRegistryPath,
        linspace<T>(10.0f, 20.0f, numInputs),
        flip(linspace<T>(10.0f, 20.0f, numInputs)),
        {},
        epsilon
    };
    testParams.expectedOutputs.reserve(numInputs);

    return testParams;
}

template <typename T>
static EnableIfComplex<T, TwoToOneBlockTestParams<T>> getBaseTestParams(const std::string& blockRegistryPath)
{
    using Scalar = typename T::value_type;
    static_assert(std::is_floating_point<Scalar>::value);

    auto scalarTestInputs = getBaseTestParams<Scalar>(blockRegistryPath).inputs0;

    // To get an even size
    if(0 != (scalarTestInputs.size() % 2)) scalarTestInputs.pop_back();

    TwoToOneBlockTestParams<T> testParams =
    {
        blockRegistryPath,
        toComplexVector(scalarTestInputs),
        flip(toComplexVector(scalarTestInputs)),
        {},
        {1e-6f,1e-6f}
    };
    testParams.expectedOutputs.reserve(testParams.inputs0.size());

    return testParams;
}

template <typename T, typename Func>
static TwoToOneBlockTestParams<T> getTestParamsForFunc(
    const std::string& blockRegistryPath,
    const Func& func)
{
    auto testParams = getBaseTestParams<T>(blockRegistryPath);
    testParams.expectedOutputs.resize(testParams.inputs0.size());

    // This can probably be done with std::transform hackiness, but this works.
    for(size_t i = 0; i < testParams.inputs0.size(); ++i)
    {
        testParams.expectedOutputs[i] = func(testParams.inputs0[i], testParams.inputs1[i]);
    }

    return testParams;
}

//
// Test code
//

template <typename T>
static void testTwoToOneBlocksCommon()
{
    twoToOneBlockTest(getTestParamsForFunc<T>(
        "/numpy/subtract",
        std::minus<T>()));
}

template <typename T>
static EnableIfInteger<T, void> testTwoToOneBlocks()
{
    testTwoToOneBlocksCommon<T>();

    twoToOneBlockTest(getTestParamsForFunc<T>(
        "/numpy/bitwise_and",
        std::bit_and<T>()));
    twoToOneBlockTest(getTestParamsForFunc<T>(
        "/numpy/bitwise_or",
        std::bit_or<T>()));
}

template <typename T>
static EnableIfUnsignedInt<T, void> testTwoToOneBlocks()
{
    testTwoToOneBlocksCommon<T>();

    twoToOneBlockTest(getTestParamsForFunc<T>(
        "/numpy/bitwise_and",
        std::bit_and<T>()));
    twoToOneBlockTest(getTestParamsForFunc<T>(
        "/numpy/bitwise_or",
        std::bit_or<T>()));
}

template <typename T>
static EnableIfFloat<T, void> testTwoToOneBlocks()
{
    testTwoToOneBlocksCommon<T>();

    twoToOneBlockTest(getTestParamsForFunc<T>(
        "/numpy/divide",
        std::divides<T>()));
}

template <typename T>
static EnableIfComplex<T, void> testTwoToOneBlocks()
{
    testTwoToOneBlocksCommon<T>();

    twoToOneBlockTest(getTestParamsForFunc<T>(
        "/numpy/divide",
        std::divides<T>()));
}

POTHOS_TEST_BLOCK("/numpy/tests", test_two_to_one_blocks)
{
    // SFINAE will make these call the functions with the
    // applicable blocks.
    testTwoToOneBlocks<std::int8_t>();
    testTwoToOneBlocks<std::int16_t>();
    testTwoToOneBlocks<std::int32_t>();
    testTwoToOneBlocks<std::int64_t>();
    testTwoToOneBlocks<std::uint8_t>();
    testTwoToOneBlocks<std::uint16_t>();
    testTwoToOneBlocks<std::uint32_t>();
    testTwoToOneBlocks<std::uint64_t>();
    testTwoToOneBlocks<float>();
    testTwoToOneBlocks<double>();
    testTwoToOneBlocks<std::complex<float>>();
    testTwoToOneBlocks<std::complex<double>>();
}
