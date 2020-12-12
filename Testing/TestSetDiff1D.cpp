// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Framework.hpp>
#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/Random.h>
#include <Poco/Timestamp.h>

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>

static constexpr size_t numChannels = 2;
static constexpr size_t bufferLen = 64;

template <typename T>
static void getTestValues(
    Pothos::BufferChunk* pInput1,
    Pothos::BufferChunk* pInput2,
    Pothos::BufferChunk* pExpectedOutput)
{
    const auto dtype = Pothos::DType(typeid(T));

    auto rng = Poco::Random();
    rng.seed(Poco::Timestamp().utcTime());

    std::vector<T> inputVec1 = NPTests::bufferChunkToStdVector<T>(NPTests::getRandomInputs(dtype.name(), bufferLen));
    std::vector<T> inputVec2;
    std::vector<T> outputVector;

    inputVec1 = NPTests::getUniqueValues(inputVec1);
    inputVec2 = NPTests::getUniqueValues(inputVec2);

    // Second vector is a subset of first
    const auto inputVec2Size = inputVec1.size() / 2;
    std::unordered_set<size_t> inputVec1Indices;
    do
    {
        inputVec1Indices.emplace(rng.next(inputVec1.size()));
    }
    while(inputVec1Indices.size() < inputVec2Size);

    for(size_t i = 0; i < inputVec1.size(); ++i)
    {
        if(inputVec1Indices.find(i) == inputVec1Indices.end())
        {
            outputVector.emplace_back(inputVec1[i]);
        }
        else
        {
            inputVec2.emplace_back(inputVec1[i]);
        }
    }
    POTHOS_TEST_FALSE(outputVector.empty());

    (*pInput1) = NPTests::stdVectorToBufferChunk(inputVec1);
    (*pInput2) = NPTests::stdVectorToBufferChunk(inputVec2);
    (*pExpectedOutput) = NPTests::stdVectorToBufferChunk(outputVector);
}

template <typename T>
static void testSetDiff1D()
{
    const auto dtype = Pothos::DType(typeid(T));

    std::cout << "Testing " << dtype.name() << "..." << std::endl;

    Pothos::BufferChunk input1;
    Pothos::BufferChunk input2;
    Pothos::BufferChunk expectedOutput;
    getTestValues<T>(&input1, &input2, &expectedOutput);

    auto source1 = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    source1.call("feedBuffer", input1);

    auto source2 = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    source2.call("feedBuffer", input2);

    auto setDiff1D = Pothos::BlockRegistry::make("/numpy/setdiff1d", dtype);

    auto sink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    {
        Pothos::Topology topology;

        topology.connect(source1, 0, setDiff1D, 0);
        topology.connect(source2, 0, setDiff1D, 1);
        topology.connect(setDiff1D, 0, sink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    auto env = Pothos::ProxyEnvironment::make("python");
    std::cout << env->makeProxy(input1).toString() << std::endl;
    std::cout << env->makeProxy(input2).toString() << std::endl;
    std::cout << std::endl;
    std::cout << env->makeProxy(expectedOutput).toString() << std::endl;
    std::cout << env->makeProxy(sink.call<Pothos::BufferChunk>("getBuffer")).toString() << std::endl;

    NPTests::testBufferChunk(
        expectedOutput,
        sink.call<Pothos::BufferChunk>("getBuffer"));
}

POTHOS_TEST_BLOCK("/numpy/tests", test_setdiff1d)
{
    testSetDiff1D<std::int8_t>();
    /*
    testSetDiff1D<std::int16_t>();
    testSetDiff1D<std::int32_t>();
    testSetDiff1D<std::int64_t>();
    testSetDiff1D<std::uint8_t>();
    testSetDiff1D<std::uint16_t>();
    testSetDiff1D<std::uint32_t>();
    testSetDiff1D<std::uint64_t>();
    testSetDiff1D<float>();
    testSetDiff1D<double>();
    */
}
