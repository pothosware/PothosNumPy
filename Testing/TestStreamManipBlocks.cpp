// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Testing.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

//
// Utility code
//

static constexpr size_t bufferLen = 64;

struct TestValuesInt
{
    Pothos::BufferChunk invert;
    Pothos::BufferChunk absolute;
    Pothos::BufferChunk positive;
    Pothos::BufferChunk negative;
};

struct TestValuesUInt
{
    Pothos::BufferChunk invert;
};

struct TestValuesFloat
{
    Pothos::BufferChunk absolute;
    Pothos::BufferChunk rint;
    Pothos::BufferChunk ceil;
    Pothos::BufferChunk floor;
    Pothos::BufferChunk trunc;
    Pothos::BufferChunk positive;
    Pothos::BufferChunk negative;
    Pothos::BufferChunk reciprocal;
};

template <typename T>
static NPTests::EnableIfInteger<T, void> getTestValues(
    Pothos::BufferChunk* pInputs,
    TestValuesInt* pTestValuesInt)
{
    const auto dtype = Pothos::DType(typeid(T));

    (*pInputs) = NPTests::getRandomInputs(dtype.name(), bufferLen);

    pTestValuesInt->invert = Pothos::BufferChunk(dtype, bufferLen);
    pTestValuesInt->absolute = Pothos::BufferChunk(dtype, bufferLen);
    pTestValuesInt->positive = Pothos::BufferChunk(dtype, bufferLen);
    pTestValuesInt->negative = Pothos::BufferChunk(dtype, bufferLen);

    for(size_t i = 0; i < bufferLen; ++i)
    {
        pTestValuesInt->invert.as<T*>()[i] = ~pInputs->as<const T*>()[i];
        pTestValuesInt->absolute.as<T*>()[i] = std::abs(pInputs->as<const T*>()[i]);
        pTestValuesInt->positive.as<T*>()[i] = pInputs->as<const T*>()[i];
        pTestValuesInt->negative.as<T*>()[i] = -pInputs->as<const T*>()[i];
    }
}

template <typename T>
static NPTests::EnableIfUnsignedInt<T, void> getTestValues(
    Pothos::BufferChunk* pInputs,
    TestValuesUInt* pTestValuesUInt)
{
    const auto dtype = Pothos::DType(typeid(T));

    (*pInputs) = NPTests::getRandomInputs(dtype.name(), bufferLen);

    pTestValuesUInt->invert = Pothos::BufferChunk(dtype, bufferLen);

    for(size_t i = 0; i < bufferLen; ++i)
    {
        pTestValuesUInt->invert.as<T*>()[i] = ~pInputs->as<const T*>()[i];
    }
}

template <typename T>
static NPTests::EnableIfFloat<T, void> getTestValues(
    Pothos::BufferChunk* pInputs,
    TestValuesFloat* pTestValuesFloat)
{
    const auto dtype = Pothos::DType(typeid(T));

    (*pInputs) = NPTests::getRandomInputs(dtype.name(), bufferLen);

    pTestValuesFloat->absolute = Pothos::BufferChunk(dtype, bufferLen);
    pTestValuesFloat->rint = Pothos::BufferChunk(dtype, bufferLen);
    pTestValuesFloat->ceil = Pothos::BufferChunk(dtype, bufferLen);
    pTestValuesFloat->floor = Pothos::BufferChunk(dtype, bufferLen);
    pTestValuesFloat->trunc = Pothos::BufferChunk(dtype, bufferLen);
    pTestValuesFloat->positive = Pothos::BufferChunk(dtype, bufferLen);
    pTestValuesFloat->negative = Pothos::BufferChunk(dtype, bufferLen);
    pTestValuesFloat->reciprocal = Pothos::BufferChunk(dtype, bufferLen);

    for(size_t i = 0; i < bufferLen; ++i)
    {
        pTestValuesFloat->absolute.as<T*>()[i] = std::abs(pInputs->as<const T*>()[i]);
        pTestValuesFloat->rint.as<T*>()[i] = std::round(pInputs->as<const T*>()[i]);
        pTestValuesFloat->ceil.as<T*>()[i] = std::ceil(pInputs->as<const T*>()[i]);
        pTestValuesFloat->floor.as<T*>()[i] = std::floor(pInputs->as<const T*>()[i]);
        pTestValuesFloat->trunc.as<T*>()[i] = std::trunc(pInputs->as<const T*>()[i]);
        pTestValuesFloat->positive.as<T*>()[i] = pInputs->as<const T*>()[i];
        pTestValuesFloat->negative.as<T*>()[i] = -pInputs->as<const T*>()[i];
        pTestValuesFloat->reciprocal.as<T*>()[i] = T(1.0f) / pInputs->as<const T*>()[i];
    }
}

//
// Test code
//

template <typename T>
static NPTests::EnableIfInteger<T, void> testStreamManipBlocks()
{
    const Pothos::DType dtype(typeid(T));

    std::cout << "Testing " << dtype.name() << "..." << std::endl;

    Pothos::BufferChunk input;
    TestValuesInt expectedOutput;
    getTestValues<T>(&input, &expectedOutput);

    auto source = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    source.call("feedBuffer", input);

    auto invert = Pothos::BlockRegistry::make("/numpy/invert", dtype);
    auto absolute = Pothos::BlockRegistry::make("/numpy/absolute", dtype);
    auto positive = Pothos::BlockRegistry::make("/numpy/positive", dtype);
    auto negative = Pothos::BlockRegistry::make("/numpy/negative", dtype);

    auto invertSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto absoluteSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto positiveSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto negativeSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    {
        Pothos::Topology topology;

        topology.connect(source, 0, invert, 0);
        topology.connect(source, 0, absolute, 0);
        topology.connect(source, 0, positive, 0);
        topology.connect(source, 0, negative, 0);

        topology.connect(invert, 0, invertSink, 0);
        topology.connect(absolute, 0, absoluteSink, 0);
        topology.connect(positive, 0, positiveSink, 0);
        topology.connect(negative, 0, negativeSink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    std::cout << " * Testing /numpy/invert..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.invert,
        invertSink.call<Pothos::BufferChunk>("getBuffer"));

    std::cout << " * Testing /numpy/absolute..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.absolute,
        absoluteSink.call<Pothos::BufferChunk>("getBuffer"));

    std::cout << " * Testing /numpy/positive..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.positive,
        positiveSink.call<Pothos::BufferChunk>("getBuffer"));

    std::cout << " * Testing /numpy/negative..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.negative,
        negativeSink.call<Pothos::BufferChunk>("getBuffer"));
}

template <typename T>
static NPTests::EnableIfUnsignedInt<T, void> testStreamManipBlocks()
{
    const Pothos::DType dtype(typeid(T));

    std::cout << "Testing " << dtype.name() << "..." << std::endl;

    Pothos::BufferChunk input;
    TestValuesUInt expectedOutput;
    getTestValues<T>(&input, &expectedOutput);

    auto source = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    source.call("feedBuffer", input);

    auto invert = Pothos::BlockRegistry::make("/numpy/invert", dtype);

    auto invertSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    {
        Pothos::Topology topology;

        topology.connect(source, 0, invert, 0);
        topology.connect(invert, 0, invertSink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    std::cout << " * Testing /numpy/invert..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.invert,
        invertSink.call<Pothos::BufferChunk>("getBuffer"));
}

template <typename T>
static NPTests::EnableIfFloat<T, void> testStreamManipBlocks()
{
    const Pothos::DType dtype(typeid(T));

    std::cout << "Testing " << dtype.name() << "..." << std::endl;

    Pothos::BufferChunk input;
    TestValuesFloat expectedOutput;
    getTestValues<T>(&input, &expectedOutput);

    auto source = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    source.call("feedBuffer", input);

    auto absolute = Pothos::BlockRegistry::make("/numpy/absolute", dtype);
    auto rint = Pothos::BlockRegistry::make("/numpy/rint", dtype);
    auto ceil = Pothos::BlockRegistry::make("/numpy/ceil", dtype);
    auto floor = Pothos::BlockRegistry::make("/numpy/floor", dtype);
    auto trunc = Pothos::BlockRegistry::make("/numpy/trunc", dtype);
    auto positive = Pothos::BlockRegistry::make("/numpy/positive", dtype);
    auto negative = Pothos::BlockRegistry::make("/numpy/negative", dtype);
    auto reciprocal = Pothos::BlockRegistry::make("/numpy/reciprocal", dtype);

    auto absoluteSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto rintSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto ceilSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto floorSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto truncSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto positiveSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto negativeSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto reciprocalSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    {
        Pothos::Topology topology;

        topology.connect(source, 0, absolute, 0);
        topology.connect(source, 0, rint, 0);
        topology.connect(source, 0, ceil, 0);
        topology.connect(source, 0, floor, 0);
        topology.connect(source, 0, trunc, 0);
        topology.connect(source, 0, positive, 0);
        topology.connect(source, 0, negative, 0);
        topology.connect(source, 0, reciprocal, 0);

        topology.connect(absolute, 0, absoluteSink, 0);
        topology.connect(rint, 0, rintSink, 0);
        topology.connect(ceil, 0, ceilSink, 0);
        topology.connect(floor, 0, floorSink, 0);
        topology.connect(trunc, 0, truncSink, 0);
        topology.connect(positive, 0, positiveSink, 0);
        topology.connect(negative, 0, negativeSink, 0);
        topology.connect(reciprocal, 0, reciprocalSink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    std::cout << " * Testing /numpy/absolute..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.absolute,
        absoluteSink.call<Pothos::BufferChunk>("getBuffer"));

    std::cout << " * Testing /numpy/rint..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.rint,
        rintSink.call<Pothos::BufferChunk>("getBuffer"));

    std::cout << " * Testing /numpy/ceil..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.ceil,
        ceilSink.call<Pothos::BufferChunk>("getBuffer"));

    std::cout << " * Testing /numpy/floor..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.floor,
        floorSink.call<Pothos::BufferChunk>("getBuffer"));

    std::cout << " * Testing /numpy/trunc..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.trunc,
        truncSink.call<Pothos::BufferChunk>("getBuffer"));

    std::cout << " * Testing /numpy/positive..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.positive,
        positiveSink.call<Pothos::BufferChunk>("getBuffer"));

    std::cout << " * Testing /numpy/negative..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.negative,
        negativeSink.call<Pothos::BufferChunk>("getBuffer"));

    std::cout << " * Testing /numpy/reciprocal..." << std::endl;
    NPTests::testBufferChunk(
        expectedOutput.reciprocal,
        reciprocalSink.call<Pothos::BufferChunk>("getBuffer"));
}

//
// Test block
//

POTHOS_TEST_BLOCK("/numpy/tests", test_stream_manip_blocks)
{
    testStreamManipBlocks<std::int8_t>();
    testStreamManipBlocks<std::int16_t>();
    testStreamManipBlocks<std::int32_t>();
    testStreamManipBlocks<std::int64_t>();
    testStreamManipBlocks<std::uint8_t>();
    testStreamManipBlocks<std::uint16_t>();
    testStreamManipBlocks<std::uint32_t>();
    testStreamManipBlocks<std::uint64_t>();
    testStreamManipBlocks<float>();
    testStreamManipBlocks<double>();
}
