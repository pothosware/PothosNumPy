// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Framework.hpp>
#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/Random.h>
#include <Poco/Timestamp.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <set>
#include <vector>

static constexpr size_t bufferLen = 64;

template <typename T>
static void getTestValues(
    Pothos::BufferChunk* pInput,
    Pothos::BufferChunk* pExpectedOutput)
{
    const auto dtype = Pothos::DType(typeid(T));

    NPTests::getNonUniqueRandomTestInputs(
        dtype.name(),
        bufferLen,
        pInput);

    const T* pInputBuffer = (*pInput);

    std::set<T> uniqueSet;
    std::copy(
        pInputBuffer,
        pInputBuffer + bufferLen,
        std::inserter(uniqueSet, uniqueSet.end()));

    std::vector<T> uniqueSetVector;
    std::copy(
        uniqueSet.begin(),
        uniqueSet.end(),
        std::back_inserter(uniqueSetVector));

    (*pExpectedOutput) = Pothos::BufferChunk(dtype, uniqueSetVector.size());
    void* pExpectedOutputBuffer = (*pExpectedOutput);
    std::memcpy(
        pExpectedOutputBuffer,
        uniqueSetVector.data(),
        pExpectedOutput->elements() * sizeof(T));
}

template <typename T>
static void testUnique()
{
    const auto dtype = Pothos::DType(typeid(T));

    std::cout << "Testing " << dtype.name() << "..." << std::endl;

    Pothos::BufferChunk input;
    Pothos::BufferChunk expectedOutput;
    getTestValues<T>(
        &input,
        &expectedOutput);

    auto source = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    source.call("feedBuffer", input);

    auto unique = Pothos::BlockRegistry::make(
                         "/numpy/unique",
                         dtype);

    auto sink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    {
        Pothos::Topology topology;

        topology.connect(source, 0, unique, 0);
        topology.connect(unique, 0, sink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    NPTests::testBufferChunk(
        expectedOutput,
        sink.call<Pothos::BufferChunk>("getBuffer"));
}

// Note: not testing floats due to differences in precision
// between different ArrayFire backends. Assumption is that
// integral types are enough to test.
//
POTHOS_TEST_BLOCK("/numpy/tests", test_unique)
{
    testUnique<std::int8_t>();
    testUnique<std::int16_t>();
    testUnique<std::int32_t>();
    testUnique<std::int64_t>();
    testUnique<std::uint8_t>();
    testUnique<std::uint16_t>();
    testUnique<std::uint32_t>();
    testUnique<std::uint64_t>();
    testUnique<float>();
    testUnique<double>();
}
