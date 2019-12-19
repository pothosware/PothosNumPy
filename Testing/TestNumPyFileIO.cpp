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

static Pothos::BufferChunk getRandomInputs(
    const std::string& type,
    size_t numElements)
{
    #define IfTypeGetRandomInputs(typeStr, ctype) \
        if(type == typeStr) \
            return stdVectorToBufferChunk<ctype>( \
                Pothos::DType(typeStr), \
                getRandomInputs<ctype>(numElements));

    IfTypeGetRandomInputs("int8", std::int8_t)
    IfTypeGetRandomInputs("int16", std::int16_t)
    IfTypeGetRandomInputs("int32", std::int32_t)
    IfTypeGetRandomInputs("int64", std::int64_t)
    IfTypeGetRandomInputs("uint8", std::uint8_t)
    IfTypeGetRandomInputs("uint16", std::uint16_t)
    IfTypeGetRandomInputs("uint32", std::uint32_t)
    IfTypeGetRandomInputs("uint64", std::uint64_t)
    IfTypeGetRandomInputs("float32", float)
    IfTypeGetRandomInputs("float64", double)
    IfTypeGetRandomInputs("complex_float32", std::complex<float>)
    IfTypeGetRandomInputs("complex_float64", std::complex<double>)

    // Should never get here
    return Pothos::BufferChunk();
}

//
// Common test code
//

static void testNPYIO(const std::string& type)
{
    static constexpr size_t numElements = 256;

    const Pothos::DType dtype(type);
    std::cout << "Testing " << dtype.toString() << std::endl;

    const std::string filepath = getTemporaryTestFile(dtype, ".npy");
    const auto randomInputs = getRandomInputs(type, numElements);

    //
    // Write known values to the .NPY file.
    //

    auto feederSource = Pothos::BlockRegistry::make(
                            "/blocks/feeder_source",
                            dtype);
    feederSource.call("feedBuffer", randomInputs);

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
            feederSource, 0,
            numpySave, 0);

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }

    POTHOS_TEST_TRUE(Poco::File(filepath).exists());
    POTHOS_TEST_TRUE(
        Poco::File(filepath).getSize() >=
        (numElements * dtype.elemSize()));

    //
    // Read from the .NPY file and check the file contents.
    //

    auto numpyLoadNpy = Pothos::BlockRegistry::make(
                             "/numpy/load_npy",
                             filepath);
    POTHOS_TEST_EQUAL(
        filepath,
        numpyLoadNpy.call<std::string>("getFilepath"));

    // Note: we need to get the Python class's internal port because the Python
    // class's dtype() function returns the NumPy dtype.
    POTHOS_TEST_EQUAL(
        dtype.name(),
        numpyLoadNpy.call("output", 0).get("_port").call("dtype").call<std::string>("name"));

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
    if(std::string::npos == type.find("int64"))
    {
        testBufferChunk(
            collectorSink.call("getBuffer"),
            randomInputs);
    }
}

static void testSaveNPZ(
    const std::string& filepath,
    const std::string& key,
    bool compressed,
    bool append,
    const Pothos::BufferChunk& values)
{
    const auto& dtype = values.dtype;

    auto numpySaveNpz = Pothos::BlockRegistry::make(
                            "/numpy/savez",
                            filepath,
                            key,
                            dtype,
                            compressed,
                            append);
    POTHOS_TEST_EQUAL(
        filepath,
        numpySaveNpz.call<std::string>("getFilepath"));
    POTHOS_TEST_EQUAL(
        key,
        numpySaveNpz.call<std::string>("getKey"));
    POTHOS_TEST_EQUAL(
        compressed,
        numpySaveNpz.call<bool>("getCompressed"));
    POTHOS_TEST_EQUAL(
        append,
        numpySaveNpz.call<bool>("getAppend"));

    // Note: we need to get the Python class's internal port because the Python
    // class's dtype() function returns the NumPy dtype.
    POTHOS_TEST_EQUAL(
        dtype.name(),
        numpySaveNpz.call("input", 0).get("_port").call("dtype").call<std::string>("name"));

    auto feederSource = Pothos::BlockRegistry::make(
                            "/blocks/feeder_source",
                            dtype);
    feederSource.call("feedBuffer", values);

    // Execute the topology.
    {
        Pothos::Topology topology;

        topology.connect(
            feederSource, 0,
            numpySaveNpz, 0);

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }
}

static void testLoadNPZ(
    const std::string& filepath,
    const std::string& key,
    const Pothos::BufferChunk& values)
{
    const auto& dtype = values.dtype;

    auto numpyLoadNpz = Pothos::BlockRegistry::make(
                            "/numpy/load_npz",
                            filepath,
                            key);
    POTHOS_TEST_EQUAL(
        filepath,
        numpyLoadNpz.call<std::string>("getFilepath"));
    POTHOS_TEST_EQUAL(
        key,
        numpyLoadNpz.call<std::string>("getKey"));

    // Note: we need to get the Python class's internal port because the Python
    // class's dtype() function returns the NumPy dtype.
    POTHOS_TEST_EQUAL(
        dtype.name(),
        numpyLoadNpz.call("output", 0).get("_port").call("dtype").call<std::string>("name"));

    auto collectorSink = Pothos::BlockRegistry::make(
                             "/blocks/collector_sink",
                             dtype);

    // Execute the topology.
    {
        Pothos::Topology topology;

        topology.connect(
            numpyLoadNpz, 0,
            collectorSink, 0);

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }

    testBufferChunk(
        collectorSink.call<Pothos::BufferChunk>("getBuffer"),
        values);
}

static void testNPZIO(bool compressed)
{
    static constexpr size_t numElements = 256;

    const std::string blockName = "/numpy/savez";
    std::cout << "Testing " << blockName << " (" << (compressed ? "compressed" : "uncompressed") << ")" << std::endl;
    const std::string filepath = getTemporaryTestFile(".npz");

    auto int8Input = getRandomInputs("int8", numElements);
    auto int16Input = getRandomInputs("int16", numElements);
    auto int32Input = getRandomInputs("int32", numElements);
    auto int64Input = getRandomInputs("int64", numElements);
    auto uint8Input = getRandomInputs("uint8", numElements);
    auto uint16Input = getRandomInputs("uint16", numElements);
    auto uint32Input = getRandomInputs("uint32", numElements);
    auto uint64Input = getRandomInputs("uint64", numElements);
    auto floatInput = getRandomInputs("float32", numElements);
    auto doubleInput = getRandomInputs("float64", numElements);
    auto complexFloatInput = getRandomInputs("complex_float32", numElements);
    auto complexDoubleInput = getRandomInputs("complex_float64", numElements);

    //
    // Save generated files to the .NPZ file.
    //
    testSaveNPZ(filepath, "port_int8", compressed, false /*append*/, int8Input);
    testSaveNPZ(filepath, "port_int16", compressed, false /*append*/, int16Input);
    testSaveNPZ(filepath, "port_int32", compressed, false /*append*/, int32Input);
    testSaveNPZ(filepath, "port_int64", compressed, false /*append*/, int64Input);
    testSaveNPZ(filepath, "port_uint8", compressed, false /*append*/, uint8Input);
    testSaveNPZ(filepath, "port_uint16", compressed, false /*append*/, uint16Input);
    testSaveNPZ(filepath, "port_uint32", compressed, false /*append*/, uint32Input);
    testSaveNPZ(filepath, "port_uint64", compressed, false /*append*/, uint64Input);
    testSaveNPZ(filepath, "port_float32", compressed, false /*append*/, floatInput);
    testSaveNPZ(filepath, "port_float64", compressed, false /*append*/, doubleInput);
    testSaveNPZ(filepath, "port_complex_float32", compressed, false /*append*/, complexFloatInput);
    testSaveNPZ(filepath, "port_complex_float64", compressed, false /*append*/, complexDoubleInput);

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
    testLoadNPZ(filepath, "port_int8", int8Input);
    testLoadNPZ(filepath, "port_int16", int16Input);
    testLoadNPZ(filepath, "port_int32", int32Input);
    testLoadNPZ(filepath, "port_int64", int64Input);
    testLoadNPZ(filepath, "port_uint8", uint8Input);
    testLoadNPZ(filepath, "port_uint16", uint16Input);
    testLoadNPZ(filepath, "port_uint32", uint32Input);
    testLoadNPZ(filepath, "port_uint64", uint64Input);
    testLoadNPZ(filepath, "port_float32", floatInput);
    testLoadNPZ(filepath, "port_float64", doubleInput);
    testLoadNPZ(filepath, "port_complex_float32", complexFloatInput);
    testLoadNPZ(filepath, "port_complex_float64", complexDoubleInput);
}

//
// Registered tests
//

POTHOS_TEST_BLOCK("/numpy/tests", test_npy_io)
{
    std::srand(std::time(0ULL));

    testNPYIO("int8");
    testNPYIO("int16");
    testNPYIO("int32");
    testNPYIO("int64");
    testNPYIO("uint8");
    testNPYIO("uint16");
    testNPYIO("uint32");
    testNPYIO("uint64");
    testNPYIO("float32");
    testNPYIO("float64");
    testNPYIO("complex_float32");
    testNPYIO("complex_float64");
}

POTHOS_TEST_BLOCK("/numpy/tests", test_npz_io)
{
    std::srand(std::time(0ULL));

    testNPZIO(false /*compressed*/);
    testNPZIO(true /*compressed*/);
}
