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
// Square, Sqrt
//

template <typename T>
static constexpr T maxSquareInput()
{
    return T(100);
}

template <typename T>
static void testSquareSqrt()
{
    const auto dtype = Pothos::DType(typeid(T));

    std::cout << "Testing " << dtype.name() << "..." << std::endl;

    constexpr T minInput = T(0);
    constexpr T maxInput = maxSquareInput<T>();

    std::vector<T> testValuesVec;
    for(T val = minInput; val < maxInput; ++val) testValuesVec.emplace_back(val);

    auto testValues = NPTests::stdVectorToBufferChunk(testValuesVec);

    auto source = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    auto square = Pothos::BlockRegistry::make("/numpy/square", dtype);
    auto sqrt   = Pothos::BlockRegistry::make("/numpy/sqrt", dtype);
    auto sink   = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    source.call("feedBuffer", testValues);

    {
        Pothos::Topology topology;

        topology.connect(source, 0, square, 0);
        topology.connect(square, 0, sqrt, 0);
        topology.connect(sqrt, 0, sink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    NPTests::testBufferChunk(
        testValues,
        sink.call<Pothos::BufferChunk>("getBuffer"));
}

//
// Cbrt
//

template <typename T>
static constexpr T maxCbrtOutput()
{
    return T(100);
}

template <typename T>
static void testCbrt()
{
    const auto dtype = Pothos::DType(typeid(T));

    std::cout << "Testing " << dtype.name() << "..." << std::endl;

    constexpr T maxOutput = maxCbrtOutput<T>();

    std::vector<T> inputVec;
    std::vector<T> expectedOutputVec;

    for(size_t i = 0; i < size_t(maxOutput); ++i)
    {
        expectedOutputVec.emplace_back(T(i));
        inputVec.emplace_back(T(std::pow(T(i), 3.0f)));
    }

    auto input = NPTests::stdVectorToBufferChunk(inputVec);
    auto expectedOutput = NPTests::stdVectorToBufferChunk(expectedOutputVec);

    auto source = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    auto cbrt   = Pothos::BlockRegistry::make("/numpy/cbrt", dtype);
    auto sink   = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    source.call("feedBuffer", input);

    {
        Pothos::Topology topology;

        topology.connect(source, 0, cbrt, 0);
        topology.connect(cbrt, 0, sink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    NPTests::testBufferChunk(
        expectedOutput,
        sink.call<Pothos::BufferChunk>("getBuffer"));
}

//
// Test blocks
//

POTHOS_TEST_BLOCK("/numpy/tests", test_square_sqrt)
{
    testSquareSqrt<float>();
    testSquareSqrt<double>();
}

POTHOS_TEST_BLOCK("/numpy/tests", test_cbrt)
{
    testCbrt<float>();
    testCbrt<double>();
}
