// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/Path.h>
#include <Poco/TemporaryFile.h>
#include <Poco/Timestamp.h>
#include <Poco/Thread.h>

#include <algorithm>
#include <cstdint>
#include <complex>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

//
// Utility code
//

static std::string getTemporaryTestFile(const std::string& extension)
{
    Poco::Path tempTestFile(Poco::Path::forDirectory(Poco::Path::temp()));
    tempTestFile.setBaseName(std::to_string(Poco::Timestamp().utcTime()) + extension);

    return tempTestFile.toString();
}

static std::string getTemporaryTestFile(
    const Pothos::DType& dtype,
    const std::string& extension)
{
    Poco::Path tempTestFile(Poco::Path::forDirectory(Poco::Path::temp()));
    tempTestFile.setBaseName(dtype.toString() + "_" + std::to_string(Poco::Timestamp().utcTime()) + extension);

    return tempTestFile.toString();
}

// This is random enough for our use case.
template <typename T>
static EnableIfAnyInt<T, std::vector<T>> getRandomInputs(size_t numElements)
{
    std::vector<T> randomInputs(numElements);
    int* intBuffer = (int*)randomInputs.data();

    const size_t numIntElements = (numElements * sizeof(T)) / sizeof(int);

    for(size_t intI = 0; intI < numIntElements; ++intI)
    {
        intBuffer[intI] = std::rand();
    }

    return randomInputs;
}

// The reinterpret_cast method above potentially results in NaN for floating-point
// types, which ruins comparisons.
template <typename T>
static EnableIfFloat<T, std::vector<T>> getRandomInputs(size_t numElements)
{
    std::vector<T> randomInputs(numElements);

    for(size_t i = 0; i < numElements; ++i)
    {
        randomInputs[i] = static_cast<T>(std::rand());
    }

    return randomInputs;
}

template <typename T>
static EnableIfComplex<T, std::vector<T>> getRandomInputs(size_t numElements)
{
    using Scalar = typename T::value_type;

    return toComplexVector(getRandomInputs<Scalar>(numElements * 2));
}

template <typename T>
static inline EnableIfNotComplex<T, T> getEpsilon()
{
    return T(1e-4);
}

template <typename T>
static inline EnableIfComplex<T, T> getEpsilon()
{
    using U = typename T::value_type;

    return T{getEpsilon<U>(), getEpsilon<U>()};
}

//
// Common test code
//

template <typename T>
static void testNPYIO()
{
    static constexpr size_t numElements = 256;

    const Pothos::DType dtype(typeid(T));
    std::cout << "Testing " << dtype.toString() << std::endl;

    const std::string filepath = getTemporaryTestFile(dtype, ".npy");
    const auto randomInputs = getRandomInputs<T>(numElements);

    //
    // Write known values to the .NPY file.
    //

    auto vectorSource = Pothos::BlockRegistry::make(
                            "/blocks/vector_source",
                            dtype);
    vectorSource.call("setElements", randomInputs);

    auto numpySave = Pothos::BlockRegistry::make(
                         "/numpy/save",
                         dtype,
                         filepath);
    POTHOS_TEST_EQUAL(
        filepath,
        numpySave.call<std::string>("getFilepath"));

    // Execute the topology.
    {
        Pothos::Topology topology;
        topology.connect(
            vectorSource, 0,
            numpySave, 0);

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }

    POTHOS_TEST_TRUE(Poco::File(filepath).exists());
    POTHOS_TEST_TRUE(
        Poco::File(filepath).getSize() >=
        (numElements * sizeof(T)));

    //
    // Read from the .NPY file and check the file contents.
    //

    auto numpyLoadNpy = Pothos::BlockRegistry::make(
                             "/numpy/load_npy",
                             filepath,
                             dtype);
    POTHOS_TEST_EQUAL(
        filepath,
        numpyLoadNpy.call<std::string>("getFilepath"));

    auto collectorSink = Pothos::BlockRegistry::make(
                             "/blocks/collector_sink",
                             dtype);

    // Execute the topology.
    {
        Pothos::Topology topology;
        topology.connect(
            numpyLoadNpy, 0,
            collectorSink, 0);

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }
    POTHOS_TEST_TRUE(collectorSink.call<Pothos::BufferChunk>("getBuffer").elements() > 0);

    // Equality is not guaranteed with 64-bit integral types, so just
    // make sure it executes.
    if(!std::is_same<T, std::int64_t>::value && !std::is_same<T, std::uint64_t>::value)
    {
        testBufferChunk<T>(
            collectorSink.call("getBuffer"),
            randomInputs,
            getEpsilon<T>());
    }
}

static void testNPZIO(bool compressed)
{
    static constexpr size_t numElements = 256;

    static const std::vector<std::string> dtypeStrings =
    {
        "int8", "int16", "int32", "int64",
        "uint8", "uint16", "uint32", "uint64",
        "float32", "float64",
        "complex_float32", "complex_float64"
    };

    std::unordered_map<std::string, Pothos::DType> dtypeMap;
    std::unordered_map<std::string, std::string> portNames;
    std::unordered_map<std::string, Pothos::Proxy> vectorSourceMap;
    std::unordered_map<std::string, Pothos::Proxy> collectorSinkMap;

    const std::string blockName = "/numpy/savez";
    std::cout << "Testing " << blockName << " (" << (compressed ? "compressed" : "uncompressed") << ")" << std::endl;

    //
    // Write known values to the .NPZ file.
    //

    const std::string filepath = getTemporaryTestFile(".npz");
    auto numpySaveZ = Pothos::BlockRegistry::make(
                          blockName,
                          filepath,
                          compressed);
    POTHOS_TEST_EQUAL(
        filepath,
        numpySaveZ.call<std::string>("getFilepath"));

    auto int8Input = getRandomInputs<std::int8_t>(numElements);
    auto int16Input = getRandomInputs<std::int16_t>(numElements);
    auto int32Input = getRandomInputs<std::int32_t>(numElements);
    auto uint8Input = getRandomInputs<std::uint8_t>(numElements);
    auto uint16Input = getRandomInputs<std::uint16_t>(numElements);
    auto uint32Input = getRandomInputs<std::uint32_t>(numElements);
    auto floatInput = getRandomInputs<float>(numElements);
    auto doubleInput = getRandomInputs<double>(numElements);
    auto complexFloatInput = getRandomInputs<std::complex<float>>(numElements);
    auto complexDoubleInput = getRandomInputs<std::complex<double>>(numElements);

    for(const std::string& dtypeString: dtypeStrings)
    {
        dtypeMap.emplace(
            dtypeString,
            Pothos::DType(dtypeString));
        portNames.emplace(
            dtypeString,
            "port_"+dtypeString);
        vectorSourceMap.emplace(
            dtypeString,
            Pothos::BlockRegistry::make(
                "/blocks/vector_source",
                dtypeMap[dtypeString]));
        collectorSinkMap.emplace(
            dtypeString,
            Pothos::BlockRegistry::make(
                "/blocks/collector_sink",
                dtypeMap[dtypeString]));

        numpySaveZ.call(
            "addChannel",
            dtypeMap[dtypeString],
            portNames[dtypeString]);
    }

    vectorSourceMap["int8"].call("setElements", int8Input);
    vectorSourceMap["int16"].call("setElements", int16Input);
    vectorSourceMap["int32"].call("setElements", int32Input);
    vectorSourceMap["uint8"].call("setElements", uint8Input);
    vectorSourceMap["uint16"].call("setElements", uint16Input);
    vectorSourceMap["uint32"].call("setElements", uint32Input);
    vectorSourceMap["float32"].call("setElements", floatInput);
    vectorSourceMap["float64"].call("setElements", doubleInput);
    vectorSourceMap["complex_float32"].call("setElements", complexFloatInput);
    vectorSourceMap["complex_float64"].call("setElements", complexDoubleInput);

    // Execute the topology.
    {
        Pothos::Topology topology;

        for(const std::string& dtypeString: dtypeStrings)
        {
            topology.connect(
                vectorSourceMap[dtypeString], 0,
                numpySaveZ, portNames[dtypeString]);
        }

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }

    POTHOS_TEST_TRUE(Poco::File(filepath).exists());

    // Since every channel should have written
    if(!compressed)
    {
        static constexpr size_t minSize = (numElements * sizeof(std::int8_t))
                                        + (numElements * sizeof(std::int16_t))
                                        + (numElements * sizeof(std::int32_t))
                                        + (numElements * sizeof(std::uint8_t))
                                        + (numElements * sizeof(std::uint16_t))
                                        + (numElements * sizeof(std::uint32_t))
                                        + (numElements * sizeof(float))
                                        + (numElements * sizeof(double))
                                        + (numElements * sizeof(std::complex<float>))
                                        + (numElements * sizeof(std::complex<double>));
        POTHOS_TEST_TRUE(Poco::File(filepath).getSize() >= minSize);
    }

    //
    // Read from the .NPZ file and check the file contents.
    //

    auto numpyLoadNpz = Pothos::BlockRegistry::make(
                            "/numpy/load_npz",
                            filepath);
    POTHOS_TEST_EQUAL(
        filepath,
        numpyLoadNpz.call<std::string>("getFilepath"));

    // Make sure the block populated its output ports based on the file.
    auto outputPortInfo = numpyLoadNpz.call<std::vector<Pothos::PortInfo>>("outputPortInfo");
    for(const std::string& dtypeString: dtypeStrings)
    {
        auto outputPortIter = std::find_if(
            outputPortInfo.begin(),
            outputPortInfo.end(),
            [&](const Pothos::PortInfo& portInfo)
            {
                return (portInfo.name == portNames[dtypeString]);
            });
        POTHOS_TEST_TRUE(outputPortInfo.end() != outputPortIter);
        POTHOS_TEST_TRUE(outputPortIter->dtype == dtypeMap[dtypeString]);
    }

    // Execute the topology.
    {
        Pothos::Topology topology;

        for(const std::string& dtypeString: dtypeStrings)
        {
            topology.connect(
                numpyLoadNpz, portNames[dtypeString],
                collectorSinkMap[dtypeString], 0);
        }

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }

    testBufferChunk<std::int8_t>(
        collectorSinkMap["int8"].call("getBuffer"),
        int8Input,
        getEpsilon<std::int8_t>());
    testBufferChunk<std::int16_t>(
        collectorSinkMap["int16"].call("getBuffer"),
        int16Input,
        getEpsilon<std::int16_t>());
    testBufferChunk<std::int32_t>(
        collectorSinkMap["int32"].call("getBuffer"),
        int32Input,
        getEpsilon<std::int32_t>());
    testBufferChunk<std::uint8_t>(
        collectorSinkMap["uint8"].call("getBuffer"),
        uint8Input,
        getEpsilon<std::uint8_t>());
    testBufferChunk<std::uint16_t>(
        collectorSinkMap["uint16"].call("getBuffer"),
        uint16Input,
        getEpsilon<std::uint16_t>());
    testBufferChunk<std::uint32_t>(
        collectorSinkMap["uint32"].call("getBuffer"),
        uint32Input,
        getEpsilon<std::uint32_t>());
    testBufferChunk<float>(
        collectorSinkMap["float32"].call("getBuffer"),
        floatInput,
        getEpsilon<float>());
    testBufferChunk<double>(
        collectorSinkMap["float64"].call("getBuffer"),
        doubleInput,
        getEpsilon<double>());
    testBufferChunk<std::complex<float>>(
        collectorSinkMap["complex_float32"].call("getBuffer"),
        complexFloatInput,
        getEpsilon<std::complex<float>>());
    testBufferChunk<std::complex<double>>(
        collectorSinkMap["complex_float64"].call("getBuffer"),
        complexDoubleInput,
        getEpsilon<std::complex<double>>());
}

//
// Registered tests
//

POTHOS_TEST_BLOCK("/numpy/tests", test_npy_io)
{
    std::srand(std::time(0ULL));

    testNPYIO<std::int8_t>();
    testNPYIO<std::int16_t>();
    testNPYIO<std::int32_t>();
    testNPYIO<std::int64_t>();
    testNPYIO<std::uint8_t>();
    testNPYIO<std::uint16_t>();
    testNPYIO<std::uint32_t>();
    testNPYIO<std::uint64_t>();
    testNPYIO<float>();
    testNPYIO<double>();
    testNPYIO<std::complex<float>>();
    testNPYIO<std::complex<double>>();
}

POTHOS_TEST_BLOCK("/numpy/tests", test_npz_io)
{
    std::srand(std::time(0ULL));

    testNPZIO(false /*compressed*/);
    testNPZIO(true /*compressed*/);
}
