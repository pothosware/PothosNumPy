// Copyright (c) 2019 Nick Foster
//               2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Testing.hpp>

#include <cmath>
#include <cstdint>
#include <iostream>

static constexpr size_t NUM_POINTS = 12;

//
// Utility code
//

// Make it easier to templatize
template <typename Type>
static double logTmpl(Type input)
{
    return std::log(input);
}


// Make it easier to templatize
template <typename Type>
static double log2Tmpl(Type input)
{
    return std::log2(input);
}

// Make it easier to templatize
template <typename Type>
static double log10Tmpl(Type input)
{
    return std::log10(input);
}

// Make it easier to templatize
template <typename Type>
static double log1pTmpl(Type input)
{
    return std::log1p(input);
}

//
// Test implementations
//

template <typename Type>
using LogTmplFcn = double(*)(Type);

template <typename Type>
static void testLogImpl(const std::string& blockPath, LogTmplFcn<Type> logFcn)
{
    static const auto dtype = Pothos::DType(typeid(Type));
    std::cout << "Testing " << blockPath << " with type " << dtype.toString() << std::endl;

    auto feeder = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    auto log = Pothos::BlockRegistry::make(blockPath, dtype);
    auto collector = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    Pothos::BufferChunk abuffOut = collector.call("getBuffer");
    //load the feeder
    auto buffIn = Pothos::BufferChunk(typeid(Type), NUM_POINTS);
    auto pIn = buffIn.as<Type *>();
    for (size_t i = 0; i < buffIn.elements(); i++)
    {
        pIn[i] = Type(10*(i+1));
    }
    feeder.call("feedBuffer", buffIn);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, log, 0);
        topology.connect(log   , 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    //check the collector
    Pothos::BufferChunk buffOut = collector.call("getBuffer");
    POTHOS_TEST_EQUAL(buffOut.elements(), buffIn.elements());
    auto pOut = buffOut.as<const Type *>();
    for (size_t i = 0; i < buffOut.elements(); i++)
    {
        const auto expected = logFcn(pIn[i]);
        //allow up to an error of 1 because of fixed point truncation rounding
        POTHOS_TEST_CLOSE(pOut[i], expected, 1);
    }
}

POTHOS_TEST_BLOCK("/numpy/tests", test_log_blocks)
{
    testLogImpl<float>("/numpy/log", &logTmpl<float>);
    testLogImpl<float>("/numpy/log2", &log2Tmpl<float>);
    testLogImpl<float>("/numpy/log10", &log10Tmpl<float>);
    testLogImpl<float>("/numpy/log1p", &log1pTmpl<float>);
    testLogImpl<double>("/numpy/log", &logTmpl<double>);
    testLogImpl<double>("/numpy/log2", &log2Tmpl<double>);
    testLogImpl<double>("/numpy/log10", &log10Tmpl<double>);
    testLogImpl<double>("/numpy/log1p", &log1pTmpl<double>);
}
