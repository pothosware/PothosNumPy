// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <iostream>
#include <string>
#include <vector>

template <typename T>
struct SimpleBlockTestParams
{
    std::string blockRegistryPath;
    std::vector<T> inputs;
    std::vector<T> expectedOutputs;
    T epsilon;
};

template <typename T>
static void simpleBlockTest(const SimpleBlockTestParams<T>& testParams)
{
    POTHOS_TEST_EQUAL(
        testParams.inputs.size(),
        testParams.expectedOutputs.size());

    static constexpr size_t dimension = 2;
    auto dtype = Pothos::DType(typeid(T), dimension);

    std::cout << "Testing " << dtype.toString() << std::endl;

    auto feeder = Pothos::BlockRegistry::make(
                      "/blocks/feeder_source",
                      dtype);
    auto testBlock = Pothos::BlockRegistry::make(
                         testParams.blockRegistryPath,
                         dtype);
    auto collector = Pothos::BlockRegistry::make(
                         "/blocks/collector_sink",
                         dtype);

    // Load the feeder
    feeder.call(
        "feedBuffer",
        stdVectorToBufferChunk(
            dtype,
            testParams.inputs));

    // Run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, testBlock, 0);
        topology.connect(testBlock, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    // Check the outputs.
    testBufferChunk(
        collector.call("getBuffer"),
        testParams.expectedOutputs,
        testParams.epsilon);
}
