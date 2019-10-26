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

    testBlockExecutionCommon<T>(testBlock);
}

//
// Type-specific functions
//

template <typename T>
static EnableIfInteger<T, void> _testManualBlockExecution()
{
}

template <typename T>
static EnableIfUnsignedInt<T, void> _testManualBlockExecution()
{
}

template <typename T>
static EnableIfFloat<T, void> _testManualBlockExecution()
{
}

template <typename T>
static EnableIfComplex<T, void> _testManualBlockExecution()
{
}

//
// Actual template specializations here
//

template <>
void testManualBlockExecution<std::int8_t>()
{_testManualBlockExecution<std::int8_t>();}

template <>
void testManualBlockExecution<std::int16_t>()
{_testManualBlockExecution<std::int16_t>();}

template <>
void testManualBlockExecution<std::int32_t>()
{_testManualBlockExecution<std::int32_t>();}

template <>
void testManualBlockExecution<std::int64_t>()
{_testManualBlockExecution<std::int64_t>();}

template <>
void testManualBlockExecution<std::uint8_t>()
{_testManualBlockExecution<std::uint8_t>();}

template <>
void testManualBlockExecution<std::uint16_t>()
{_testManualBlockExecution<std::uint16_t>();}

template <>
void testManualBlockExecution<std::uint32_t>()
{_testManualBlockExecution<std::uint32_t>();}

template <>
void testManualBlockExecution<std::uint64_t>()
{_testManualBlockExecution<std::uint64_t>();}

template <>
void testManualBlockExecution<float>()
{_testManualBlockExecution<float>();}

template <>
void testManualBlockExecution<double>()
{_testManualBlockExecution<double>();}

template <>
void testManualBlockExecution<std::complex<float>>()
{_testManualBlockExecution<std::complex<float>>();}

template <>
void testManualBlockExecution<std::complex<double>>()
{_testManualBlockExecution<std::complex<double>>();}
