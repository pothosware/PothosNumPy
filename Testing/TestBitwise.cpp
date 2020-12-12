// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Framework.hpp>
#include <Pothos/Testing.hpp>

#include <iostream>
#include <vector>

//
// Utility code
//

static constexpr size_t bufferLen = 64;

//
// Test implementations
//

template <typename T>
static void testInvert()
{
    const Pothos::DType dtype(typeid(T));

    auto input = NPTests::getRandomInputs(dtype.name(), bufferLen);
    Pothos::BufferChunk expectedOutput(typeid(T), input.elements());
    for (size_t elem = 0; elem < expectedOutput.elements(); ++elem)
    {
        expectedOutput.template as<T*>()[elem] = ~input.template as<const T*>()[elem];
    }

    auto source = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    auto notBlock = Pothos::BlockRegistry::make("/numpy/invert", dtype);
    auto sink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    source.call("feedBuffer", input);

    {
        Pothos::Topology topology;

        topology.connect(source, 0, notBlock, 0);
        topology.connect(notBlock, 0, sink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    std::cout << " * Testing Invert..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput,
        sink.call<Pothos::BufferChunk>("getBuffer"));
}

template <typename T>
static void testBinaryBlocks()
{
    const Pothos::DType dtype(typeid(T));
    constexpr size_t numInputs = 2;

    std::cout << "Testing " << dtype.name() << "..." << std::endl;

    testInvert<T>();

    std::vector<Pothos::BufferChunk> inputs;
    for (size_t i = 0; i < numInputs; ++i) inputs.emplace_back(NPTests::getRandomInputs(dtype.name(), bufferLen));

    Pothos::BufferChunk expectedAndOutput(dtype, bufferLen);
    Pothos::BufferChunk expectedOrOutput(dtype, bufferLen);
    Pothos::BufferChunk expectedXOrOutput(dtype, bufferLen);

    for (size_t elem = 0; elem < bufferLen; ++elem)
    {
        expectedAndOutput.template as<T*>()[elem] = inputs[0].template as<T*>()[elem] & inputs[1].template as<T*>()[elem];
        expectedOrOutput.template as<T*>()[elem] = inputs[0].template as<T*>()[elem] | inputs[1].template as<T*>()[elem];
        expectedXOrOutput.template as<T*>()[elem] = inputs[0].template as<T*>()[elem] ^ inputs[1].template as<T*>()[elem];
    }

    std::vector<Pothos::Proxy> sources(numInputs);
    for (size_t input = 0; input < numInputs; ++input)
    {
        sources[input] = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
        sources[input].call("feedBuffer", inputs[input]);
    }

    auto andBlock = Pothos::BlockRegistry::make("/numpy/bitwise_and", dtype);
    auto orBlock = Pothos::BlockRegistry::make("/numpy/bitwise_or", dtype);
    auto xorBlock = Pothos::BlockRegistry::make("/numpy/bitwise_xor", dtype);

    auto andSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto orSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto xorSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    {
        Pothos::Topology topology;

        for (size_t input = 0; input < numInputs; ++input)
        {
            topology.connect(sources[input], 0, andBlock, input);
            topology.connect(sources[input], 0, orBlock, input);
            topology.connect(sources[input], 0, xorBlock, input);
        }

        topology.connect(andBlock, 0, andSink, 0);
        topology.connect(orBlock, 0, orSink, 0);
        topology.connect(xorBlock, 0, xorSink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    std::cout << " * Testing And..." << std::endl;
    NPTests::testBufferChunk(
        expectedAndOutput,
        andSink.call<Pothos::BufferChunk>("getBuffer"));

    std::cout << " * Testing Or..." << std::endl;
    NPTests::testBufferChunk(
        expectedOrOutput,
        orSink.call<Pothos::BufferChunk>("getBuffer"));

    std::cout << " * Testing XOr..." << std::endl;
    NPTests::testBufferChunk(
        expectedXOrOutput,
        xorSink.call<Pothos::BufferChunk>("getBuffer"));
}

//
// Tests
//

POTHOS_TEST_BLOCK("/numpy/tests", test_bitwise_blocks)
{
    testBinaryBlocks<std::int8_t>();
    testBinaryBlocks<std::int16_t>();
    testBinaryBlocks<std::int32_t>();
    testBinaryBlocks<std::int64_t>();
    testBinaryBlocks<std::uint8_t>();
    testBinaryBlocks<std::uint16_t>();
    testBinaryBlocks<std::uint32_t>();
    testBinaryBlocks<std::uint64_t>();
}
