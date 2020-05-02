// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Testing.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <typeinfo>
#include <vector>

template <typename T>
static NPTests::EnableIfAnyInt<T, Pothos::BufferChunk> _getRandomInputs(size_t numInputs)
{
    static std::random_device rd;
    static std::mt19937 g(rd());
    static std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

    Pothos::BufferChunk randomInputs(Pothos::DType(typeid(T)), numInputs);
    for(size_t elem = 0; elem < numInputs; ++elem)
    {
        randomInputs.as<T*>()[elem] = dist(g);
    }

    return randomInputs;
}

template <typename T>
static NPTests::EnableIfFloat<T, Pothos::BufferChunk> _getRandomInputs(size_t numInputs)
{
    static std::random_device rd;
    static std::mt19937 g(rd());
    static std::uniform_real_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

    Pothos::BufferChunk randomInputs(Pothos::DType(typeid(T)), numInputs);
    for(size_t elem = 0; elem < numInputs; ++elem)
    {
        randomInputs.as<T*>()[elem] = dist(g);
    }

    return randomInputs;
}

template <typename T>
static NPTests::EnableIfComplex<T, Pothos::BufferChunk> _getRandomInputs(size_t numInputs)
{
    using ScalarType = typename T::value_type;

    auto randomInputs = _getRandomInputs<ScalarType>(numInputs*2);
    randomInputs.dtype = Pothos::DType(typeid(T));

    return randomInputs;
}

static Pothos::BufferChunk getRandomInputs(const std::string& type, size_t numInputs)
{
#define GET_RANDOM_INPUTS(typeStr, cType) \
    if(type == typeStr) return _getRandomInputs<cType>(numInputs);

    GET_RANDOM_INPUTS("int8", std::int8_t)
    GET_RANDOM_INPUTS("int16", std::int16_t)
    GET_RANDOM_INPUTS("int32", std::int32_t)
    GET_RANDOM_INPUTS("int64", std::int64_t)
    GET_RANDOM_INPUTS("uint8", std::uint8_t)
    GET_RANDOM_INPUTS("uint16", std::uint16_t)
    GET_RANDOM_INPUTS("uint32", std::uint32_t)
    GET_RANDOM_INPUTS("uint64", std::uint64_t)
    GET_RANDOM_INPUTS("float32", float)
    GET_RANDOM_INPUTS("float64", double)
    GET_RANDOM_INPUTS("complex_float32", std::complex<float>)
    GET_RANDOM_INPUTS("complex_float64", std::complex<double>)

    // Should never happen
    return Pothos::BufferChunk();
}

static void addCornerCases(
    const std::string& type1,
    const std::string& type2,
    Pothos::BufferChunk& inputBuffer)
{
    POTHOS_TEST_GE(inputBuffer.elements(), 9);

    if((("float32" == type1) || ("complex_float32" == type1)) && Pothos::DType(type2).isFloat())
    {
        inputBuffer.as<float*>()[0] = std::numeric_limits<float>::min();
        inputBuffer.as<float*>()[1] = std::numeric_limits<float>::max();
        inputBuffer.as<float*>()[2] = std::numeric_limits<float>::infinity();
        inputBuffer.as<float*>()[3] = -std::numeric_limits<float>::infinity();
        inputBuffer.as<float*>()[4] = std::numeric_limits<float>::lowest();
        inputBuffer.as<float*>()[5] = std::numeric_limits<float>::epsilon();
        inputBuffer.as<float*>()[6] = std::nexttoward(std::numeric_limits<float>::min(), 0.0f);
        inputBuffer.as<float*>()[7] = std::nexttoward(std::numeric_limits<float>::max(), 0.0f);
        inputBuffer.as<float*>()[8] = std::numeric_limits<float>::denorm_min();
    }
    else if(("float64" == type1) && ("complex_float64" == type2))
    {
        inputBuffer.as<double*>()[0] = std::numeric_limits<double>::min();
        inputBuffer.as<double*>()[1] = std::numeric_limits<double>::max();
        inputBuffer.as<double*>()[2] = std::numeric_limits<double>::infinity();
        inputBuffer.as<double*>()[3] = -std::numeric_limits<double>::infinity();
        inputBuffer.as<double*>()[4] = std::numeric_limits<double>::lowest();
        inputBuffer.as<double*>()[5] = std::numeric_limits<double>::epsilon();
        inputBuffer.as<double*>()[6] = std::nexttoward(std::numeric_limits<double>::min(), 0.0f);
        inputBuffer.as<double*>()[7] = std::nexttoward(std::numeric_limits<double>::max(), 0.0f);
        inputBuffer.as<double*>()[8] = std::numeric_limits<double>::denorm_min();
    }
}

static void testAsType(
    const std::string& type1,
    const std::string& type2)
{
    static constexpr size_t numInputs = 256;
    static constexpr const char* numpyAsTypeRegistryPath = "/numpy/astype";

    std::cout << "Testing " << numpyAsTypeRegistryPath
              << " (types: " << type1 << " -> " << type2 << ")" << std::endl;

    Pothos::DType inputDType(type1);
    Pothos::DType outputDType(type2);

    auto npAsType = Pothos::BlockRegistry::make(
                        numpyAsTypeRegistryPath,
                        outputDType);

    auto pothosConverter = Pothos::BlockRegistry::make(
                               "/blocks/converter",
                               outputDType);

    auto testInputs = getRandomInputs(inputDType.name(), numInputs);
    addCornerCases(type1, type2, testInputs);

    auto feederSource = Pothos::BlockRegistry::make(
                            "/blocks/feeder_source",
                            inputDType);
    feederSource.call("feedBuffer", testInputs);

    auto npCollectorSink = Pothos::BlockRegistry::make(
                               "/blocks/collector_sink",
                               outputDType);
    auto pothosCollectorSink = Pothos::BlockRegistry::make(
                                   "/blocks/collector_sink",
                                   outputDType);

    // Execute the topology.
    {
        Pothos::Topology topology;
        topology.connect(feederSource, 0, npAsType, 0);
        topology.connect(npAsType, 0, npCollectorSink, 0);

        topology.connect(feederSource, 0, pothosConverter, 0);
        topology.connect(pothosConverter, 0, pothosCollectorSink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.05));
    }

    // This block is meant to be a faster version of Pothos's converter
    // block, so we need to make sure the outputs match.
    auto npOutput = npCollectorSink.call<Pothos::BufferChunk>("getBuffer");
    auto pothosOutput = pothosCollectorSink.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(
        testInputs.elements(),
        npOutput.elements());
    NPTests::testBufferChunk(
        pothosOutput,
        npOutput);
}

POTHOS_TEST_BLOCK("/numpy/tests", test_astype)
{
    const std::vector<std::string> dtypeNames =
    {
        "uint8","uint16","uint32","uint64",
        "int8","int16","int32","int64",
        "float32","float64","complex_float32","complex_float64"
    };

    for(const auto& inputType: dtypeNames)
    {
        for(const auto& outputType: dtypeNames)
        {
            testAsType(inputType, outputType);
        }
    }
}
