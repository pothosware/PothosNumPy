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
#include <vector>

static constexpr size_t bufferLen = 64;

template <typename T>
static Pothos::BufferChunk getBufferChunkUnion1D(const std::vector<Pothos::BufferChunk>& bufferChunks)
{
    POTHOS_TEST_FALSE(bufferChunks.empty());

    std::set<T> union1D;
    for(const auto& bufferChunk: bufferChunks)
    {
        std::copy(
            bufferChunk.as<T*>(),
            bufferChunk.as<T*>()+bufferChunk.elements(),
            std::inserter(union1D, union1D.end()));
    }

    std::vector<T> union1DVec;
    std::copy(
        union1D.begin(),
        union1D.end(),
        std::back_inserter(union1DVec));

    return NPTests::stdVectorToBufferChunk(union1DVec);
}

template <typename T>
static void testUnion1D()
{
    const auto dtype = Pothos::DType(typeid(T));

    std::cout << "Testing " << dtype.name() << "..." << std::endl;

    constexpr size_t numChannels = 3;
    std::vector<Pothos::BufferChunk> inputs;
    std::vector<Pothos::Proxy> sources;
    for(size_t i = 0; i < numChannels; ++i)
    {
        inputs.emplace_back(NPTests::getRandomInputs(dtype.name(), bufferLen));

        sources.emplace_back(Pothos::BlockRegistry::make("/blocks/feeder_source", dtype));
        sources.back().call("feedBuffer", inputs.back());
    }

    // Make sure we're actually testing the block behavior.
    Pothos::BufferChunk expectedOutput;
    do
    {
        expectedOutput = getBufferChunkUnion1D<T>(inputs);
    }
    while(expectedOutput.elements() >= (inputs.size() * inputs[0].elements()));

    auto union1D = Pothos::BlockRegistry::make("/numpy/union1d", "Auto", dtype, numChannels);
    auto sink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    {
        Pothos::Topology topology;

        for(size_t chan = 0; chan < numChannels; ++chan)
        {
            topology.connect(sources[chan], 0, union1D, chan);
        }
        topology.connect(union1D, 0, sink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    NPTests::testBufferChunk(
        expectedOutput,
        sink.call<Pothos::BufferChunk>("getBuffer"));
}

POTHOS_TEST_BLOCK("/numpy/tests", test_union1d)
{
    testUnion1D<std::int8_t>();
    testUnion1D<std::int16_t>();
    testUnion1D<std::int32_t>();
    testUnion1D<std::int64_t>();
    testUnion1D<std::uint8_t>();
    testUnion1D<std::uint16_t>();
    testUnion1D<std::uint32_t>();
    testUnion1D<std::uint64_t>();
    testUnion1D<float>();
    testUnion1D<double>();
}
