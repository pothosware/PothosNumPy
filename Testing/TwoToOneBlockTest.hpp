// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/Thread.h>

#include <iostream>
#include <string>
#include <vector>

template <typename T>
struct TwoToOneBlockTestParams
{
    std::string blockRegistryPath;
    std::vector<T> inputs0;
    std::vector<T> inputs1;
    std::vector<T> expectedOutputs;
    T epsilon;
};

template <typename T>
static void twoToOneBlockTest(const TwoToOneBlockTestParams<T>& testParams)
{
    POTHOS_TEST_EQUAL(
        testParams.inputs0.size(),
        testParams.expectedOutputs.size());
    POTHOS_TEST_EQUAL(
        testParams.inputs1.size(),
        testParams.expectedOutputs.size());

    static constexpr size_t dimension = 1;
    auto dtype = Pothos::DType(typeid(T), dimension);

    std::cout << "Testing " << testParams.blockRegistryPath << "(" << dtype.toString() << ")" << std::endl;

    auto feeder0 = Pothos::BlockRegistry::make(
                       "/blocks/feeder_source",
                       dtype);
    auto feeder1 = Pothos::BlockRegistry::make(
                       "/blocks/feeder_source",
                       dtype);
    auto testBlock = Pothos::BlockRegistry::make(
                         testParams.blockRegistryPath,
                         dtype);
    auto collector = Pothos::BlockRegistry::make(
                         "/blocks/collector_sink",
                         dtype);

    // Load the feeders
    feeder0.call(
        "feedBuffer",
        stdVectorToBufferChunk(
            dtype,
            testParams.inputs0));
    feeder1.call(
        "feedBuffer",
        stdVectorToBufferChunk(
            dtype,
            testParams.inputs1));

    // Run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder0, 0, testBlock, 0);
        topology.connect(feeder1, 0, testBlock, 1);
        topology.connect(testBlock, 0, collector, 0);
        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }

    // Check the outputs.
    testBufferChunk(
        collector.call("getBuffer"),
        testParams.expectedOutputs,
        testParams.epsilon);
}
