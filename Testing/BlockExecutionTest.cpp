// Copyright (c) 2019-2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "Testing/BlockExecutionTest.hpp"
#include "Testing/TestUtility.hpp"

#include <Pothos/Callable.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/Thread.h>

#include <algorithm>
#include <complex>
#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <iostream>

namespace PothosNumPyTests
{

//
// Utility functions
//

template <typename T>
static std::vector<T> getIntTestParams(T a, T step, size_t N)
{
    std::vector<T> ret;
    ret.reserve(N);

    for(size_t i = 0; i < N; ++i)
    {
        ret.emplace_back(a + (T(i)*step));
    }

    return ret;
}

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

Pothos::BufferChunk getTestInputs(const std::string& type)
{
    #define IfTypeThenGetInputs(typeStr, ctype) \
        if(type == typeStr) \
            return stdVectorToBufferChunk<ctype>(getTestInputs<ctype>());

    IfTypeThenGetInputs("int8", std::int8_t)
    IfTypeThenGetInputs("int16", std::int16_t)
    IfTypeThenGetInputs("int32", std::int32_t)
    IfTypeThenGetInputs("int64", std::int64_t)
    IfTypeThenGetInputs("uint8", std::uint8_t)
    IfTypeThenGetInputs("uint16", std::uint16_t)
    IfTypeThenGetInputs("uint32", std::uint32_t)
    IfTypeThenGetInputs("uint64", std::uint64_t)
    IfTypeThenGetInputs("float32", float)
    IfTypeThenGetInputs("float64", double)
    IfTypeThenGetInputs("complex_float32", std::complex<float>)
    IfTypeThenGetInputs("complex_float64", std::complex<double>)

    // Should never get here
    return Pothos::BufferChunk();
}

static std::vector<Pothos::PortInfo> getNonSigSlotPortInfo(
    const Pothos::Proxy& block,
    const std::string& portInfoCall)
{
    const auto allPortInfo = block.call<std::vector<Pothos::PortInfo>>(portInfoCall);
    if(allPortInfo.empty())
    {
        return allPortInfo;
    }

    std::vector<Pothos::PortInfo> filteredPortInfo;
    filteredPortInfo.reserve(allPortInfo.size());

    std::copy_if(
        std::begin(allPortInfo),
        std::end(allPortInfo),
        std::back_inserter(filteredPortInfo),
        [](const Pothos::PortInfo& portInfo){return !portInfo.isSigSlot;});

    return filteredPortInfo;
}

//
// Test function
//

void testBlockExecutionCommon(
    const Pothos::Proxy& testBlock,
    bool longTimeout)
{
    std::unordered_map<std::string, Pothos::Proxy> feederSources;
    auto inputPortInfo = getNonSigSlotPortInfo(testBlock, "inputPortInfo");
    const bool isSource = inputPortInfo.empty();

    for(const auto& portInfo: inputPortInfo)
    {
        auto feederSource = Pothos::BlockRegistry::make(
                                "/blocks/feeder_source",
                                portInfo.dtype);
        feederSource.call(
            "feedBuffer",
            getTestInputs(portInfo.dtype.name()));
        feederSources.emplace(
            std::string(portInfo.name),
            std::move(feederSource));
    }

    std::unordered_map<std::string, Pothos::Proxy> collectorSinks;
    for(const auto& portInfo: getNonSigSlotPortInfo(testBlock, "outputPortInfo"))
    {
        collectorSinks.emplace(
            portInfo.name,
            Pothos::BlockRegistry::make(
                "/blocks/collector_sink",
                portInfo.dtype));
    }

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

        if(isSource)
        {
            Poco::Thread::sleep(longTimeout ? 10 : 5);
        }
        else
        {
            POTHOS_TEST_TRUE(topology.waitInactive(
                                 0.01,
                                 (longTimeout ? 0.0 : 1.0)));
        }
    }

    for(const auto& collectorSinkMapPair: collectorSinks)
    {
        const auto& collectorSink = collectorSinkMapPair.second;
        POTHOS_TEST_GT(collectorSink.call("getBuffer").call<size_t>("elements"), 0);
    }
}

}
