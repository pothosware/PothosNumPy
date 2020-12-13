// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Testing.hpp>

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

//
// Utility code
//

static constexpr size_t bufferLen = 256;

//
// Test code
//

template <typename T>
static void testNaNToNum()
{
    const auto dtype = Pothos::DType(typeid(T));

    std::cout << "Testing " << dtype.name() << "..." << std::endl;

    auto input = NPTests::getRandomInputs(dtype.name(), bufferLen);

    auto source = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    source.call("feedBuffer", input);

    auto sporadicNaN = Pothos::BlockRegistry::make("/blocks/sporadic_nan", dtype);
    sporadicNaN.call("setProbability", 1.0);
    sporadicNaN.call("setNumNaNs", 20);

    auto sporadicInf = Pothos::BlockRegistry::make("/blocks/sporadic_inf", dtype);
    sporadicInf.call("setProbability", 1.0);
    sporadicInf.call("setNumInfs", 20);

    auto nanToNum = Pothos::BlockRegistry::make("/numpy/nan_to_num", dtype);

    auto subnormalSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto nanToNumSink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    {
        Pothos::Topology topology;

        topology.connect(source, 0, sporadicNaN, 0);
        topology.connect(sporadicNaN, 0, sporadicInf, 0);

        topology.connect(sporadicInf, 0, subnormalSink, 0);

        topology.connect(sporadicInf, 0, nanToNum, 0);
        topology.connect(nanToNum, 0, nanToNumSink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    auto subnormalOutput = subnormalSink.call<Pothos::BufferChunk>("getBuffer");
    auto nanToNumOutput = nanToNumSink.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(subnormalOutput.dtype, nanToNumOutput.dtype);
    POTHOS_TEST_EQUAL(subnormalOutput.elements(), nanToNumOutput.elements());

    // 
    // Get indices of NaN and infinity.
    //

    std::vector<size_t> nanIndices;
    std::vector<size_t> infIndices;
    const T* subnormalOutputBuf = subnormalOutput;
    for(size_t elem = 0; elem < subnormalOutput.elements(); ++elem)
    {
        if(std::isnan(subnormalOutputBuf[elem])) nanIndices.emplace_back(elem);
        else if(std::isinf(subnormalOutputBuf[elem])) infIndices.emplace_back(elem);
    }
    POTHOS_TEST_FALSE(nanIndices.empty());
    POTHOS_TEST_FALSE(infIndices.empty());

    //
    // Make sure values were replaced as expected.
    //

    for(auto nanIndex: nanIndices)
    {
        POTHOS_TEST_EQUAL(
            T(0.0f),
            nanToNumOutput.as<const T*>()[nanIndex]);
    }
    for(auto infIndex: infIndices)
    {
        POTHOS_TEST_EQUAL(
            std::numeric_limits<T>::max(),
            nanToNumOutput.as<const T*>()[infIndex]);
    }
}

//
// Test block
//

POTHOS_TEST_BLOCK("/numpy/tests", test_nan_to_num)
{
    testNaNToNum<float>();
    testNaNToNum<double>();
}
