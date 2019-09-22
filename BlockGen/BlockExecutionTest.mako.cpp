// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

// This file was generated on ${Now}.

#include "Testing/TestUtility.hpp"

#include <Pothos/Callable.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/Thread.h>

#include <complex>
#include <cstdint>
#include <iostream>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

//
// Utility
//

template <typename T>
static EnableIfInteger<T, std::vector<T>> getTestInputs()
{
    static constexpr T minValue = std::is_same<T, std::int8_t>::value ? T(-5) : T(-25);
    static constexpr size_t numInputs = std::is_same<T, std::int8_t>::value ? 11 : 51;

    return getIntTestParams<T>(minValue, T(1), numInputs);
}

template <typename T>
static EnableIfUnsignedInt<T, std::vector<T>> getTestInputs()
{
    static constexpr T minValue = std::is_same<T, std::uint8_t>::value ? T(5) : T(25);
    static constexpr size_t numInputs = std::is_same<T, std::uint8_t>::value ? 9 : 76;

    return getIntTestParams<T>(minValue, T(1), numInputs);
}

template <typename T>
static EnableIfFloat<T, std::vector<T>> getTestInputs()
{
    // To not have nice even numbers
    static constexpr size_t numInputs = 123;

    return linspace<T>(10.0f, 20.0f, numInputs);
}

template <typename T>
static EnableIfComplex<T, std::vector<T>> getTestInputs()
{
    using Scalar = typename T::value_type;

    // To not have nice even numbers
    static constexpr size_t numInputs = 246;

    return toComplexVector(linspace<Scalar>(10.0f, 20.0f, numInputs));
}

//
// Test function (TODO: different input and output types)
//

template <typename T>
static void testBlockExecutionFunc(
    const std::string& blockRegistryPath,
    bool hasNChans)
{
    static constexpr size_t nchans = 3;

    static const Pothos::DType dtype(typeid(T));
    std::cout << blockRegistryPath << "(" << dtype.toString() << ")" << std::endl;

    static const std::vector<T> testInputs = getTestInputs<T>();

    Pothos::Proxy testBlock;
    if(hasNChans)
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype,
                        nchans);
        POTHOS_TEST_EQUAL(
            nchans,
            testBlock.call<size_t>("getNumChannels"));
    }
    else
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype);
    }

    std::unordered_map<std::string, Pothos::Proxy> feederSources;
    for(const auto& portInfo: testBlock.call<std::vector<Pothos::PortInfo>>("inputPortInfo"))
    {
        auto feederSource = Pothos::BlockRegistry::make(
                                "/blocks/feeder_source",
                                portInfo.dtype);
        feederSource.call(
            "feedBuffer",
            stdVectorToBufferChunk<T>(
                portInfo.dtype,
                testInputs));
        feederSources.emplace(
            std::string(portInfo.name),
            std::move(feederSource));
    }

    std::unordered_map<std::string, Pothos::Proxy> collectorSinks;
    for(const auto& portInfo: testBlock.call<std::vector<Pothos::PortInfo>>("outputPortInfo"))
    {
        collectorSinks.emplace(
            portInfo.name,
            Pothos::BlockRegistry::make(
                "/blocks/collector_sink",
                portInfo.dtype));
    }

    // Execute the topology.
    {
        Pothos::Topology topology;
        for(const auto& feederSourceMapPair: feederSources)
        {
            const auto& port = feederSourceMapPair.first;
            const auto& feederSource = feederSourceMapPair.second;

            topology.connect(
                feederSource,
                "0",
                testBlock,
                port);
        }
        for(const auto& collectorSinkMapPair: collectorSinks)
        {
            const auto& port = collectorSinkMapPair.first;
            const auto& collectorSink = collectorSinkMapPair.second;

            topology.connect(
                testBlock,
                port,
                collectorSink,
                "0");
        }

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(5);
    }

    // Make sure the blocks output data.
    for(const auto& collectorSinkMapPair: collectorSinks)
    {
        const auto& collectorSink = collectorSinkMapPair.second;
        POTHOS_TEST_TRUE(collectorSink.call("getBuffer").call<size_t>("elements") > 0);
    }
}

//
// Test code
//
%for typedefName,typeName in sfinaeMap.items():

template <typename T>
static EnableIf${typedefName}<T, void> testBlockExecution()
{
%for blockName,blockInfo in blockYAML.items():
    %if (not blockInfo.get("skipExecTest", False)):
        %if "blockType" in blockInfo:
            %if (typeName in blockInfo["blockType"]) or ("all" in blockInfo["blockType"]):
    testBlockExecutionFunc<T>("/numpy/${blockName}", ${"true" if blockInfo["class"] == "NToOneBlock" else "false"});
            %endif
        %endif
    %endif
%endfor
}
%endfor

POTHOS_TEST_BLOCK("/numpy/tests", test_block_execution)
{
    // SFINAE will make these call the functions with the
    // applicable blocks.
    testBlockExecution<std::int8_t>();
    testBlockExecution<std::int16_t>();
    testBlockExecution<std::int32_t>();
    testBlockExecution<std::int64_t>();
    testBlockExecution<std::uint8_t>();
    testBlockExecution<std::uint16_t>();
    testBlockExecution<std::uint32_t>();
    testBlockExecution<std::uint64_t>();
    testBlockExecution<float>();
    testBlockExecution<double>();
    testBlockExecution<std::complex<float>>();
    testBlockExecution<std::complex<double>>();
}
