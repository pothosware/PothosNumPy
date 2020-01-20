// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "Testing/BlockExecutionTest.hpp"
#include "Testing/TestUtility.hpp"

#include <Pothos/Callable.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/Thread.h>

#include <complex>
#include <cstdint>
#include <iostream>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

//
// Block-specific functions
//

template <typename T>
static void testBlockExecutionFunc(
    const std::string& blockRegistryPath,
    bool hasNChans)
{
    static constexpr size_t nchans = 3;

    static const Pothos::DType dtype(typeid(T));
    std::cout << blockRegistryPath << "(" << dtype.toString() << ")" << std::endl;

    Pothos::Proxy testBlock;
    if(hasNChans)
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype,
                        nchans);
        POTHOS_TEST_EQUAL(
            nchans,
            testBlock.call<size_t>("getNumChannels"));
    }
    else
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype);
    }
    POTHOS_TEST_EQUAL(
        blockRegistryPath,
        testBlock.call<std::string>("getName"));

    PothosNumPyTests::testBlockExecutionCommon(testBlock);
}

template <typename T1, typename T2, typename T3>
static void test3ParamBlockExecution(
    const std::string& blockRegistryPath,
    const T1& param1,
    const T2& param2,
    const T3& param3)
{
    static const Pothos::DType dtype(typeid(T1));

    auto testBlock = Pothos::BlockRegistry::make(
                         blockRegistryPath,
                         dtype,
                         param1,
                         param2,
                         param3,
                         false /*repeat*/);
    POTHOS_TEST_TRUE(!testBlock.template call<bool>("getRepeat"));
    POTHOS_TEST_EQUAL(
        blockRegistryPath,
        testBlock.template call<std::string>("getName"));

    std::cout << blockRegistryPath << "(" << dtype.toString() << ", not repeating)" << std::endl;
    PothosNumPyTests::testBlockExecutionCommon(testBlock);

    testBlock.template call("setRepeat", true);
    POTHOS_TEST_TRUE(testBlock.template call<bool>("getRepeat"));

    std::cout << blockRegistryPath << "(" << dtype.toString() << ", repeating)" << std::endl;
    PothosNumPyTests::testBlockExecutionCommon(testBlock);
}

template <typename T>
static void testSpaceBlockExecution(
    const std::string& blockRegistryPath,
    const T& start,
    const T& stop)
{
    static const Pothos::DType dtype(typeid(T));

    static constexpr size_t numValues = std::is_floating_point<T>::value ? 25 : 10;
    test3ParamBlockExecution(
        blockRegistryPath,
        start,
        stop,
        numValues);
}

template <typename T>
static void testWindow()
{
    const Pothos::DType dtype(typeid(T));
    const std::string blockRegistryPath = "/numpy/window";

    static const double DefaultKaiserBeta = 0.0;
    static const double TestKaiserBeta = 8.6; // Similar to Blackman

    static const std::vector<std::string> Windows =
    {
        "BARTLETT",
        "BLACKMAN",
        "HAMMING",
        "HANNING",
        "KAISER"
    };
    for(const auto& window: Windows)
    {
        std::cout << blockRegistryPath << "(" << dtype.toString() << "," << window << ")" << std::endl;
        auto windowBlock = Pothos::BlockRegistry::make(
                               blockRegistryPath,
                               dtype,
                               window);
        POTHOS_TEST_EQUAL(
            blockRegistryPath,
            windowBlock.call<std::string>("getName"));
        PothosNumPyTests::testEqual(
            window,
            windowBlock.call<std::string>("getWindowType"));
        PothosNumPyTests::testEqual(
            DefaultKaiserBeta,
            windowBlock.call<double>("getKaiserBeta"));

        if(window == "KAISER")
        {
            windowBlock.call("setKaiserBeta", TestKaiserBeta);
            PothosNumPyTests::testEqual(
                TestKaiserBeta,
                windowBlock.call<double>("getKaiserBeta"));
        }

        PothosNumPyTests::testBlockExecutionCommon(windowBlock);
    }
}

//
// Type-specific functions
//

template <typename T>
static void testManualBlockExecutionScalarCommon()
{
    testSpaceBlockExecution<T>(
        "/numpy/linspace",
        T(1),
        T(10));

    constexpr T rangeStep = std::is_floating_point<T>::value ? 0.25 : 1;
    test3ParamBlockExecution(
        "/numpy/arange",
        T(1),
        T(10),
        rangeStep);
}

template <typename T>
static PothosNumPyTests::EnableIfInteger<T, void> _testManualBlockExecution()
{
    testManualBlockExecutionScalarCommon<T>();
}

template <typename T>
static PothosNumPyTests::EnableIfUnsignedInt<T, void> _testManualBlockExecution()
{
    testManualBlockExecutionScalarCommon<T>();
}

template <typename T>
static PothosNumPyTests::EnableIfFloat<T, void> _testManualBlockExecution()
{
    testManualBlockExecutionScalarCommon<T>();

    /*testSpaceBlockExecution<T>(
        "/numpy/logspace",
        T(1.0),
        T(3.0));*/
    testSpaceBlockExecution<T>(
        "/numpy/geomspace",
        T(1.0),
        T(3.0));
    testWindow<T>();
}

template <typename T>
static PothosNumPyTests::EnableIfComplex<T, void> _testManualBlockExecution()
{
    testWindow<T>();
}

//
// Actual template specializations here
//

namespace PothosNumPyTests
{

#define SPECIALIZE_MANUAL_BLOCK_EXECUTION(T) \
    template <> \
    void testManualBlockExecution<T>() \
    {_testManualBlockExecution<T>();}

SPECIALIZE_MANUAL_BLOCK_EXECUTION(std::int8_t)
SPECIALIZE_MANUAL_BLOCK_EXECUTION(std::int16_t)
SPECIALIZE_MANUAL_BLOCK_EXECUTION(std::int32_t)
SPECIALIZE_MANUAL_BLOCK_EXECUTION(std::int64_t)
SPECIALIZE_MANUAL_BLOCK_EXECUTION(std::uint8_t)
SPECIALIZE_MANUAL_BLOCK_EXECUTION(std::uint16_t)
SPECIALIZE_MANUAL_BLOCK_EXECUTION(std::uint32_t)
SPECIALIZE_MANUAL_BLOCK_EXECUTION(std::uint64_t)
SPECIALIZE_MANUAL_BLOCK_EXECUTION(float)
SPECIALIZE_MANUAL_BLOCK_EXECUTION(double)
SPECIALIZE_MANUAL_BLOCK_EXECUTION(std::complex<float>)
SPECIALIZE_MANUAL_BLOCK_EXECUTION(std::complex<double>)

}
