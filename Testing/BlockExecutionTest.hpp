// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "Testing/TestUtility.hpp"

#include <Pothos/Callable.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/Thread.h>

#include <complex>
#include <cstdint>
#include <string>
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
// Test function
//

template <typename T>
static void testBlockExecutionCommon(const Pothos::Proxy& testBlock)
{
    std::unordered_map<std::string, Pothos::Proxy> feederSources;
    for(const auto& portInfo: testBlock.call<std::vector<Pothos::PortInfo>>("inputPortInfo"))
    {
        static const std::vector<T> testInputs = getTestInputs<T>();

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
