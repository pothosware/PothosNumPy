// Copyright (c) 2019 Nicholas Corgan
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

//
// Utility functions
//

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

#define TEST_BLOCK_EXECUTION_COMMON(T) \
    template <> \
    void testBlockExecutionCommon<T>( \
        const Pothos::Proxy& testBlock, \
        bool longTimeout) \
    { \
        std::unordered_map<std::string, Pothos::Proxy> feederSources; \
        auto inputPortInfo = getNonSigSlotPortInfo(testBlock, "inputPortInfo"); \
        const bool isSource = inputPortInfo.empty(); \
     \
        for(const auto& portInfo: inputPortInfo) \
        { \
            static const std::vector<T> testInputs = getTestInputs<T>(); \
     \
            auto feederSource = Pothos::BlockRegistry::make( \
                                    "/blocks/feeder_source", \
                                    portInfo.dtype); \
            feederSource.call( \
                "feedBuffer", \
                stdVectorToBufferChunk<T>( \
                    portInfo.dtype, \
                    testInputs)); \
            feederSources.emplace( \
                std::string(portInfo.name), \
                std::move(feederSource)); \
        } \
     \
        std::unordered_map<std::string, Pothos::Proxy> collectorSinks; \
        for(const auto& portInfo: getNonSigSlotPortInfo(testBlock, "outputPortInfo")) \
        { \
            collectorSinks.emplace( \
                portInfo.name, \
                Pothos::BlockRegistry::make( \
                    "/blocks/collector_sink", \
                    portInfo.dtype)); \
        } \
     \
        { \
            Pothos::Topology topology; \
            for(const auto& feederSourceMapPair: feederSources) \
            { \
                const auto& port = feederSourceMapPair.first; \
                const auto& feederSource = feederSourceMapPair.second; \
     \
                topology.connect( \
                    feederSource, \
                    "0", \
                    testBlock, \
                    port); \
            } \
            for(const auto& collectorSinkMapPair: collectorSinks) \
            { \
                const auto& port = collectorSinkMapPair.first; \
                const auto& collectorSink = collectorSinkMapPair.second; \
     \
                topology.connect( \
                    testBlock, \
                    port, \
                    collectorSink, \
                    "0"); \
            } \
     \
            topology.commit(); \
     \
            if(isSource) \
            { \
                Poco::Thread::sleep(longTimeout ? 10 : 5); \
            } \
            else \
            { \
                POTHOS_TEST_TRUE(topology.waitInactive( \
                                     0.01, \
                                     (longTimeout ? 0.0 : 1.0))); \
            } \
        } \
     \
        for(const auto& collectorSinkMapPair: collectorSinks) \
        { \
            const auto& collectorSink = collectorSinkMapPair.second; \
            POTHOS_TEST_TRUE(collectorSink.call("getBuffer").call<size_t>("elements") > 0); \
        } \
    }

TEST_BLOCK_EXECUTION_COMMON(std::int8_t)
TEST_BLOCK_EXECUTION_COMMON(std::int16_t)
TEST_BLOCK_EXECUTION_COMMON(std::int32_t)
TEST_BLOCK_EXECUTION_COMMON(std::int64_t)
TEST_BLOCK_EXECUTION_COMMON(std::uint8_t)
TEST_BLOCK_EXECUTION_COMMON(std::uint16_t)
TEST_BLOCK_EXECUTION_COMMON(std::uint32_t)
TEST_BLOCK_EXECUTION_COMMON(std::uint64_t)
TEST_BLOCK_EXECUTION_COMMON(float)
TEST_BLOCK_EXECUTION_COMMON(double)
TEST_BLOCK_EXECUTION_COMMON(std::complex<float>)
TEST_BLOCK_EXECUTION_COMMON(std::complex<double>)
