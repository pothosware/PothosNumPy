// Copyright (c) 2019-2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>
#include <Pothos/Exception.hpp>
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
#include <limits>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

constexpr size_t kNumChannels = 4;

//
// Utility code
//

static std::string getTemporaryTestFile(const std::string& extension)
{
    Poco::Path tempTestFile(Poco::Path::forDirectory(Poco::Path::temp()));
    tempTestFile.setBaseName(std::to_string(Poco::Timestamp().utcTime()) + extension);
    Poco::TemporaryFile::registerForDeletion(tempTestFile.toString());

    return tempTestFile.toString();
}

static std::string getTemporaryTestFile(
    const Pothos::DType& dtype,
    const std::string& extension)
{
    Poco::Path tempTestFile(Poco::Path::forDirectory(Poco::Path::temp()));
    tempTestFile.setBaseName(dtype.toString() + "_" + std::to_string(Poco::Timestamp().utcTime()) + extension);
    Poco::TemporaryFile::registerForDeletion(tempTestFile.toString());

    return tempTestFile.toString();
}

// This is random enough for our use case.
template <typename T>
static PothosNumPyTests::EnableIfAnyInt<T, std::vector<T>> getRandomInputs(size_t numElements)
{
    static std::random_device rd;
    static std::mt19937 g(rd());
    static std::uniform_int_distribution<T> dist(
               std::numeric_limits<T>::min(),
               std::numeric_limits<T>::max());

    std::vector<T> randomInputs;
    for(size_t i = 0; i < numElements; ++i)
    {
        randomInputs.emplace_back(dist(g));
    }

    return randomInputs;
}

template <typename T>
static PothosNumPyTests::EnableIfFloat<T, std::vector<T>> getRandomInputs(size_t numElements)
{
    static std::random_device rd;
    static std::mt19937 g(rd());
    static std::uniform_real_distribution<T> dist(
               std::numeric_limits<T>::min(),
               std::numeric_limits<T>::max());

    std::vector<T> randomInputs;
    for(size_t i = 0; i < numElements; ++i)
    {
        randomInputs.emplace_back(dist(g));
    }

    return randomInputs;
}

template <typename T>
static PothosNumPyTests::EnableIfComplex<T, std::vector<T>> getRandomInputs(size_t numElements)
{
    using Scalar = typename T::value_type;

    return PothosNumPyTests::toComplexVector(getRandomInputs<Scalar>(numElements * 2));
}

template <typename T>
static inline PothosNumPyTests::EnableIfNotComplex<T, T> getEpsilon()
{
    return T(1e-4);
}

template <typename T>
static inline PothosNumPyTests::EnableIfComplex<T, T> getEpsilon()
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
            return PothosNumPyTests::stdVectorToBufferChunk<ctype>( \
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

static std::vector<Pothos::BufferChunk> convert2DNumPyArrayToBufferChunks(const Pothos::Proxy& numpyArray)
{
    POTHOS_TEST_EQUAL("numpy.ndarray", numpyArray.getClassName());

    std::vector<Pothos::BufferChunk> ret;

    const auto arrLength = numpyArray.call<size_t>("__len__");
    for(size_t i = 0; i < arrLength; ++i)
    {
        ret.emplace_back(numpyArray.call<Pothos::BufferChunk>("__getitem__", i));
    }

    return ret;
}

using Npz1DContentsMap = std::unordered_map<std::string, Pothos::BufferChunk>;
using Npz2DContentsMap = std::unordered_map<std::string, std::vector<Pothos::BufferChunk>>;

template <typename T, typename U, typename V>
static Pothos::ProxyMap npzTestInputsToProxyMap(const typename std::unordered_map<T,U,V>& testInputs)
{
    auto env = Pothos::ProxyEnvironment::make("python");

    Pothos::ProxyMap ret;
    for(const auto& mapPair: testInputs)
    {
        ret.emplace(
            env->makeProxy(mapPair.first),
            env->makeProxy(mapPair.second));
    }

    return ret;
}

static void proxyMapToNpzTestInputs(
    const Pothos::ProxyMap& proxyMap,
    Npz1DContentsMap* p1DTestInputs,
    Npz2DContentsMap* p2DTestInputs)
{
    p1DTestInputs->clear();
    p2DTestInputs->clear();

    for(const auto& mapPair: proxyMap)
    {
        POTHOS_TEST_EQUAL("str", mapPair.first.getClassName());
        POTHOS_TEST_EQUAL("numpy.ndarray", mapPair.second.getClassName());

        const auto npArrayDims = mapPair.second.call("shape").call<size_t>("__len__");
        POTHOS_TEST_TRUE((1 == npArrayDims) || (2 == npArrayDims));

        if(1 == npArrayDims)
        {
            p1DTestInputs->emplace(
                mapPair.first.toObject().extract<std::string>(),
                mapPair.second.toObject().extract<Pothos::BufferChunk>());
        }
        else
        {
            p2DTestInputs->emplace(
                mapPair.first.toObject().extract<std::string>(),
                convert2DNumPyArrayToBufferChunks(mapPair.second));
        }
    }
}

//
// Common test code
//

static void testNpySource1D(const std::string& type)
{
    const Pothos::DType dtype(type);
    std::cout << "Testing " << dtype.toString() << " (1D)..." << std::endl;

    const std::string filepath = getTemporaryTestFile(dtype, ".npy");

    //
    // Generate our test file in NumPy and save our expected values.
    //

    auto env = Pothos::ProxyEnvironment::make("python");
    auto testFuncs = env->findProxy("PothosNumPy.TestFuncs");

    auto expectedOutputs = testFuncs.call(
                               "generate1DNpyFile",
                               filepath,
                               dtype);
    POTHOS_TEST_TRUE(
        typeid(Pothos::BufferChunk) ==
        expectedOutputs.toObject().type());
    POTHOS_TEST_TRUE(Poco::File(filepath).exists());

    //
    // Read from the .NPY file and check the file contents.
    //

    auto numpyNpySource = Pothos::BlockRegistry::make(
                              "/numpy/npy_source",
                              filepath,
                              false /*repeat*/);
    POTHOS_TEST_EQUAL(
        filepath,
        numpyNpySource.call<std::string>("filepath"));
    POTHOS_TEST_FALSE(numpyNpySource.call<bool>("repeat"));

    // Note: we need to get the Python class's internal port because the Python
    // class's dtype() function returns the NumPy dtype.
    POTHOS_TEST_EQUAL(
        dtype.name(),
        numpyNpySource.call("output", 0)
                      .get("_port")
                      .call("dtype")
                      .call<std::string>("name"));

    auto collectorSink = Pothos::BlockRegistry::make(
                             "/blocks/collector_sink",
                             dtype);

    // Execute the topology.
    {
        Pothos::Topology topology;
        topology.connect(
            numpyNpySource, 0,
            collectorSink, 0);

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }
    POTHOS_TEST_GT(collectorSink.call<Pothos::BufferChunk>("getBuffer").elements(), 0);

    PothosNumPyTests::testBufferChunk(
        collectorSink.call("getBuffer"),
        expectedOutputs.toObject().extract<Pothos::BufferChunk>());
}

static void testNpySource2D(const std::string& type)
{
    const Pothos::DType dtype(type);
    std::cout << "Testing " << dtype.toString() << " (2D)..." << std::endl;

    const std::string filepath = getTemporaryTestFile(dtype, ".npy");

    //
    // Generate our test file in NumPy and save our expected values.
    //

    auto env = Pothos::ProxyEnvironment::make("python");
    auto testFuncs = env->findProxy("PothosNumPy.TestFuncs");

    auto expectedOutputs = convert2DNumPyArrayToBufferChunks(testFuncs.call(
                               "generate2DNpyFile",
                               filepath,
                               dtype));
    POTHOS_TEST_TRUE(Poco::File(filepath).exists());

    auto numpyNpySource = Pothos::BlockRegistry::make(
                              "/numpy/npy_source",
                              filepath,
                              false /*repeat*/);
    POTHOS_TEST_EQUAL(
        filepath,
        numpyNpySource.call<std::string>("filepath"));
    POTHOS_TEST_FALSE(numpyNpySource.call<bool>("repeat"));

    // Note: we need to get the Python class's internal port because the Python
    // class's dtype() function returns the NumPy dtype.
    for(size_t chan = 0; chan < kNumChannels; ++chan)
    {
        POTHOS_TEST_EQUAL(
            dtype.name(),
            numpyNpySource.call("output", chan)
                          .get("_port")
                          .call("dtype")
                          .call<std::string>("name"));
    }

    std::vector<Pothos::Proxy> collectorSinks;
    for(size_t chan = 0; chan < kNumChannels; ++chan)
    {
        collectorSinks.emplace_back(Pothos::BlockRegistry::make(
                                        "/blocks/collector_sink",
                                        dtype));
    }

    // Execute the topology.
    {
        Pothos::Topology topology;
        for(size_t chan = 0; chan < kNumChannels; ++chan)
        {
            topology.connect(
                numpyNpySource, chan,
                collectorSinks[chan], 0);
        }

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }

    for(size_t chan = 0; chan < expectedOutputs.size(); ++chan)
    {
        POTHOS_TEST_GT(collectorSinks[chan].call<Pothos::BufferChunk>("getBuffer").elements(), 0);

        PothosNumPyTests::testBufferChunk(
            collectorSinks[chan].call("getBuffer"),
            expectedOutputs[chan]);
    }
}

static void testNpySource(const std::string& type)
{
    testNpySource1D(type);
    testNpySource2D(type);
}

static void testNpySink(const std::string& type)
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
                         "/numpy/npy_sink",
                         filepath,
                         dtype,
                         false /*append*/);
    POTHOS_TEST_EQUAL(
        filepath,
        numpySave.call<std::string>("filepath"));
    POTHOS_TEST_FALSE(numpySave.call<bool>("append"));

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
    POTHOS_TEST_GE(Poco::File(filepath).getSize(),(numElements * dtype.elemSize()));

    //
    // Use NumPy directly to test our values.
    //
    auto env = Pothos::ProxyEnvironment::make("python");
    auto testFuncs = env->findProxy("PothosNumPy.TestFuncs");

    testFuncs.call("checkNpyContents", filepath, randomInputs);
}

static void testNpzSource1D(
    const std::string& filepath,
    const std::string& key,
    const Pothos::BufferChunk& expectedOutputs)
{
    auto numpyNpzSource = Pothos::BlockRegistry::make(
                              "/numpy/npz_source",
                              filepath,
                              key,
                              false /*repeat*/);

    // Note: we need to get the Python object's internal port proxy because
    // the Python class returns a NumPy DType.
    auto dtype = numpyNpzSource.call("output", "0")
                               .get("_port")
                               .call<Pothos::DType>("dtype");
    POTHOS_TEST_EQUAL(expectedOutputs.dtype.name(), dtype.name());

    std::cout << " * Testing " << dtype.name() << " (1D)..." << std::endl;

    auto collectorSink = Pothos::BlockRegistry::make(
                             "/blocks/collector_sink",
                             dtype);

    // Execute the topology.
    {
        Pothos::Topology topology;

        topology.connect(
            numpyNpzSource, 0,
            collectorSink, 0);

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }

    PothosNumPyTests::testBufferChunk(
        collectorSink.call("getBuffer"),
        expectedOutputs);
}

static void testNpzSource(bool compressed)
{
    if(compressed)
    {
        std::cout << "Testing loading compressed .npz file." << std::endl;
    }
    else
    {
        std::cout << "Testing loading uncompressed .npz file." << std::endl;
    }

    const std::string filepath = getTemporaryTestFile(".npz");

    //
    // Generate our test file in NumPy and save our expected values.
    //

    auto env = Pothos::ProxyEnvironment::make("python");
    auto testFuncs = env->findProxy("PothosNumPy.TestFuncs");

    auto expectedOutputs = testFuncs.call(
                               "generateNpzFile",
                               filepath,
                               compressed);
    POTHOS_TEST_TRUE(
        typeid(Pothos::ProxyMap) ==
        expectedOutputs.toObject().type());
    POTHOS_TEST_TRUE(Poco::File(filepath).exists());

    Npz1DContentsMap testValues1D;
    Npz2DContentsMap testValues2D;
    proxyMapToNpzTestInputs(
        expectedOutputs,
        &testValues1D,
        &testValues2D);

    for(const auto& testValues: testValues1D)
    {
        const auto& key = testValues.first;
        const auto& expectedOutputs = testValues.second;
        testNpzSource1D(filepath, key, expectedOutputs);
    }
}

static void testNpzSink(
    const std::string& filepath,
    const std::string& key,
    bool compressed,
    bool append,
    const Pothos::BufferChunk& values)
{
    const auto& dtype = values.dtype;

    auto numpyNpzSink = Pothos::BlockRegistry::make(
                            "/numpy/npz_sink",
                            filepath,
                            key,
                            dtype,
                            compressed,
                            append);
    POTHOS_TEST_EQUAL(
        filepath,
        numpyNpzSink.call<std::string>("filepath"));
    POTHOS_TEST_EQUAL(
        key,
        numpyNpzSink.call<std::string>("key"));
    POTHOS_TEST_EQUAL(
        compressed,
        numpyNpzSink.call<bool>("compressed"));
    POTHOS_TEST_EQUAL(
        append,
        numpyNpzSink.call<bool>("append"));

    if(append)
    {
        const auto allKeys = numpyNpzSink.call<std::vector<std::string>>("allKeys");
        POTHOS_TEST_TRUE(allKeys.end() != std::find(allKeys.begin(), allKeys.end(), key));
    }

    // Note: we need to get the Python class's internal port because the Python
    // class's dtype() function returns the NumPy dtype.
    POTHOS_TEST_EQUAL(
        dtype.name(),
        numpyNpzSink.call("input", 0)
                    .get("_port")
                    .call("dtype")
                    .call<std::string>("name"));

    auto feederSource = Pothos::BlockRegistry::make(
                            "/blocks/feeder_source",
                            dtype);
    feederSource.call("feedBuffer", values);

    // Execute the topology.
    {
        Pothos::Topology topology;

        topology.connect(
            feederSource, 0,
            numpyNpzSink, 0);

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }
}

static void testNpzSink(bool compressed)
{
    static constexpr size_t numElements = 256;

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

    const std::string blockName = "/numpy/npz_sink";
    std::cout << "Testing " << blockName << " (" << (compressed ? "compressed" : "uncompressed") << ")" << std::endl;
    const std::string filepath = getTemporaryTestFile(".npz");

    Npz1DContentsMap testInputs;

    //
    // Generate our inputs.
    //
    const std::vector<std::string> typeNames =
    {
        "int8", "int16", "int32", "int64",
        "uint8", "uint16", "uint32", "uint64",
        "float32", "float64", "complex_float32", "complex_float64"
    };
    for(const std::string& type: typeNames)
    {
        testInputs.emplace(
            type,
            getRandomInputs(type, numElements));
    }

    //
    // Save generated files to the .NPZ file.
    //
    for(const auto& mapPair: testInputs)
    {
        const auto& typeName = mapPair.first;
        const auto& inputs = mapPair.second;

        testNpzSink(
            filepath,
            typeName,
            compressed,
            false /*append*/,
            inputs);
    }

    POTHOS_TEST_TRUE(Poco::File(filepath).exists());

    // Since every channel should have written
    if(!compressed)
    {
        POTHOS_TEST_GE(Poco::File(filepath).getSize(), minSize);
    }

    //
    // Use NumPy directly to test our values.
    //
    auto env = Pothos::ProxyEnvironment::make("python");
    auto testFuncs = env->findProxy("PothosNumPy.TestFuncs");

    testFuncs.call(
        "checkNpzContents",
        filepath,
        npzTestInputsToProxyMap(testInputs));
}

//
// Registered tests
//

POTHOS_TEST_BLOCK("/numpy/tests", test_npy_source)
{
    testNpySource("int8");
    testNpySource("int16");
    testNpySource("int32");
    testNpySource("int64");
    testNpySource("uint8");
    testNpySource("uint16");
    testNpySource("uint32");
    testNpySource("uint64");
    testNpySource("float32");
    testNpySource("float64");
    testNpySource("complex_float32");
    testNpySource("complex_float64");
}

POTHOS_TEST_BLOCK("/numpy/tests", test_npy_sink)
{
    testNpySink("int8");
    testNpySink("int16");
    testNpySink("int32");
    testNpySink("int64");
    testNpySink("uint8");
    testNpySink("uint16");
    testNpySink("uint32");
    testNpySink("uint64");
    testNpySink("float32");
    testNpySink("float64");
    testNpySink("complex_float32");
    testNpySink("complex_float64");
}

POTHOS_TEST_BLOCK("/numpy/tests", test_npz_source)
{
    testNpzSource(false /*compressed*/);
    testNpzSource(true /*compressed*/);
}

POTHOS_TEST_BLOCK("/numpy/tests", test_npz_sink)
{
    testNpzSink(false /*compressed*/);
    testNpzSink(true /*compressed*/);
}
