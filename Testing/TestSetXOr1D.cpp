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
#include <set>
#include <unordered_map>
#include <vector>

static constexpr size_t numChannels = 3;
static constexpr size_t bufferLen = 64;

template <typename T>
static Pothos::BufferChunk setSymmetricDifference(const std::vector<Pothos::BufferChunk>& bufferChunks)
{
    POTHOS_TEST_GT(bufferChunks.size(), 1);

    std::vector<std::vector<T>> inputVectors;
    std::transform(
        bufferChunks.begin(),
        bufferChunks.end(),
        std::back_inserter(inputVectors),
        NPTests::bufferChunkToStdVector<T>);
    for(auto& input: inputVectors) input = NPTests::getUniqueValues(input);

    using ValueCountMap = std::unordered_map<T, size_t>;
    ValueCountMap valueCounts;

    for(const auto& inputVector: inputVectors)
    {
        for(T val: inputVector)
        {
            auto iter = valueCounts.find(val);
            if(iter != valueCounts.end()) ++iter->second;
            else valueCounts.emplace(val, 1);
        }
    }

    std::vector<T> outputVector;
    for(const auto& valueCountPair: valueCounts)
    {
        if(valueCountPair.second == 1)
            outputVector.emplace_back(valueCountPair.first);
    }
    std::sort(outputVector.begin(), outputVector.end());

    return NPTests::stdVectorToBufferChunk<T>(outputVector);
}

template <typename T>
static void getTestValues(
    std::vector<Pothos::BufferChunk>* pInputs,
    Pothos::BufferChunk* pExpectedOutput)
{
    const auto dtype = Pothos::DType(typeid(T));

    NPTests::getNonUniqueRandomTestInputs(
        dtype.name(),
        numChannels,
        bufferLen,
        pInputs);
    (*pExpectedOutput) = setSymmetricDifference<T>(*pInputs);
}

template <typename T>
static void testSetXOr1D()
{
    const auto dtype = Pothos::DType(typeid(T));

    std::cout << "Testing " << dtype.name() << "..." << std::endl;

    std::vector<Pothos::BufferChunk> inputs;
    Pothos::BufferChunk expectedOutput;
    getTestValues<T>(&inputs, &expectedOutput);

    std::vector<Pothos::Proxy> sources;

    for(size_t chan = 0; chan < numChannels; ++chan)
    {
        sources.emplace_back(Pothos::BlockRegistry::make("/blocks/feeder_source", dtype));
        sources.back().call("feedBuffer", inputs[chan]);
    }

    auto setXOr1D = Pothos::BlockRegistry::make("/numpy/setxor1d", dtype, numChannels);
    auto sink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    {
        Pothos::Topology topology;

        for(size_t chan = 0; chan < numChannels; ++chan)
        {
            topology.connect(sources[chan], 0, setXOr1D, chan);
        }
        topology.connect(setXOr1D, 0, sink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    NPTests::testBufferChunk(
        expectedOutput,
        sink.call<Pothos::BufferChunk>("getBuffer"));
}

POTHOS_TEST_BLOCK("/numpy/tests", test_setxor1d)
{
    testSetXOr1D<std::int8_t>();
    testSetXOr1D<std::int16_t>();
    testSetXOr1D<std::int32_t>();
    testSetXOr1D<std::int64_t>();
    testSetXOr1D<std::uint8_t>();
    testSetXOr1D<std::uint16_t>();
    testSetXOr1D<std::uint32_t>();
    testSetXOr1D<std::uint64_t>();
    testSetXOr1D<float>();
    testSetXOr1D<double>();
}
