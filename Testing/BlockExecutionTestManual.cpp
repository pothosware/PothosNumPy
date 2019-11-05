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
