// Copyright (c) 2019-2020 Nicholas Corgan
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
static PothosNumPyTests::EnableIfFloat<T, std::vector<T>> getRandomInputs(size_t numElements)
{
    std::vector<T> randomInputs(numElements);

    for(size_t i = 0; i < numElements; ++i)
    {
        randomInputs[i] = static_cast<T>(std::rand());
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

using NpzContentsMap = std::unordered_map<std::string, Pothos::BufferChunk>;

static Pothos::ProxyMap npzTestInputsToProxyMap(const NpzContentsMap& testInputs)
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

static NpzContentsMap proxyMapToNpzTestInputs(const Pothos::ProxyMap& proxyMap)
{
    NpzContentsMap testValues;
    for(const auto& mapPair: proxyMap)
    {
        POTHOS_TEST_TRUE(
            typeid(std::string) ==
            mapPair.first.toObject().type());
        POTHOS_TEST_TRUE(
            typeid(Pothos::BufferChunk) ==
            mapPair.second.toObject().type());

        testValues.emplace(
            mapPair.first.toObject().extract<std::string>(),
            mapPair.second.toObject().extract<Pothos::BufferChunk>());
    }

    return testValues;
}

//
// Common test code
//

static void testLoadNPY(const std::string& type)
{
    const Pothos::DType dtype(type);
    std::cout << "Testing " << dtype.toString() << std::endl;

    const std::string filepath = getTemporaryTestFile(dtype, ".npy");

    //
    // Generate our test file in NumPy and save our expected values.
    //

    auto env = Pothos::ProxyEnvironment::make("python");
    auto testFuncs = env->findProxy("PothosNumPy.TestFuncs");

    auto expectedOutputs = testFuncs.call(
                               "generateNpyFile",
                               filepath,
                               dtype);
    POTHOS_TEST_TRUE(
        typeid(Pothos::BufferChunk) ==
        expectedOutputs.toObject().type());
    POTHOS_TEST_TRUE(Poco::File(filepath).exists());

    //
    // Read from the .NPY file and check the file contents.
    //

    auto numpyLoadNpy = Pothos::BlockRegistry::make(
                             "/numpy/load_npy",
                             filepath,
                             false /*repeat*/);
    POTHOS_TEST_EQUAL(
        filepath,
        numpyLoadNpy.call<std::string>("getFilepath"));
    POTHOS_TEST_TRUE(!numpyLoadNpy.call<bool>("getRepeat"));

    // Note: we need to get the Python class's internal port because the Python
    // class's dtype() function returns the NumPy dtype.
    POTHOS_TEST_EQUAL(
        dtype.name(),
        numpyLoadNpy.call("output", 0)
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
        PothosNumPyTests::testBufferChunk(
            collectorSink.call("getBuffer"),
            expectedOutputs.toObject().extract<Pothos::BufferChunk>());
    }
}

static void testSaveNPY(const std::string& type)
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
                         "/numpy/save_npy",
                         filepath,
                         dtype,
                         false /*append*/);
    POTHOS_TEST_EQUAL(
        filepath,
        numpySave.call<std::string>("getFilepath"));
    POTHOS_TEST_TRUE(!numpySave.call<bool>("getAppend"));

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
    // Use NumPy directly to test our values.
    //
    auto env = Pothos::ProxyEnvironment::make("python");
    auto testFuncs = env->findProxy("PothosNumPy.TestFuncs");

    testFuncs.call("checkNpyContents", filepath, randomInputs);
}

static void testLoadNPZ(bool compressed)
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

    auto testValues = proxyMapToNpzTestInputs(expectedOutputs.toObject().extract<Pothos::ProxyMap>());
    std::vector<std::string> keys;
    std::transform(
        testValues.begin(),
        testValues.end(),
        std::back_inserter(keys),
        [](const NpzContentsMap::value_type& mapPair){return mapPair.first;});
    POTHOS_TEST_EQUAL(keys.size(), testValues.size());

    for(const auto& key: keys)
    {
        auto numpyLoadNpz = Pothos::BlockRegistry::make(
                                "/numpy/load_npz",
                                filepath,
                                key,
                                false /*repeat*/);

        // Note: we need to get the Python object's internal port proxy because
        // the Python class returns a NumPy DType.
        auto dtype = numpyLoadNpz.call("output", "0")
                                 .get("_port")
                                 .template call<Pothos::DType>("dtype");

        std::cout << " * Testing " << dtype.name() << std::endl;

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

        PothosNumPyTests::testBufferChunk(
            collectorSink.call<Pothos::BufferChunk>("getBuffer"),
            testValues[key]);
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
                            "/numpy/save_npz",
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

    if(append)
    {
        const auto allKeys = numpySaveNpz.call<std::vector<std::string>>("getAllKeys");
        POTHOS_TEST_TRUE(allKeys.end() != std::find(allKeys.begin(), allKeys.end(), key));
    }

    // Note: we need to get the Python class's internal port because the Python
    // class's dtype() function returns the NumPy dtype.
    POTHOS_TEST_EQUAL(
        dtype.name(),
        numpySaveNpz.call("input", 0)
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
            numpySaveNpz, 0);

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }
}

static void testSaveNPZ(bool compressed)
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

    const std::string blockName = "/numpy/save_npz";
    std::cout << "Testing " << blockName << " (" << (compressed ? "compressed" : "uncompressed") << ")" << std::endl;
    const std::string filepath = getTemporaryTestFile(".npz");

    NpzContentsMap testInputs;

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

    for(size_t i = 0; i < 2; ++i)
    {
        //
        // Save generated files to the .NPZ file.
        //
        for(const auto& mapPair: testInputs)
        {
            const auto& typeName = mapPair.first;
            const auto& inputs = mapPair.second;

            testSaveNPZ(
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
            POTHOS_TEST_TRUE(Poco::File(filepath).getSize() >= minSize);
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
}

//
// Registered tests
//

// TODO: generate file in raw NumPy, test with block

POTHOS_TEST_BLOCK("/numpy/tests", test_load_npy)
{
    testLoadNPY("int8");
    testLoadNPY("int16");
    testLoadNPY("int32");
    testLoadNPY("int64");
    testLoadNPY("uint8");
    testLoadNPY("uint16");
    testLoadNPY("uint32");
    testLoadNPY("uint64");
    testLoadNPY("float32");
    testLoadNPY("float64");
    testLoadNPY("complex_float32");
    testLoadNPY("complex_float64");
}

POTHOS_TEST_BLOCK("/numpy/tests", test_save_npy)
{
    std::srand(std::time(0ULL));

    testSaveNPY("int8");
    testSaveNPY("int16");
    testSaveNPY("int32");
    testSaveNPY("int64");
    testSaveNPY("uint8");
    testSaveNPY("uint16");
    testSaveNPY("uint32");
    testSaveNPY("uint64");
    testSaveNPY("float32");
    testSaveNPY("float64");
    testSaveNPY("complex_float32");
    testSaveNPY("complex_float64");
}

POTHOS_TEST_BLOCK("/numpy/tests", test_load_npz)
{
    testLoadNPZ(false /*compressed*/);
    testLoadNPZ(true /*compressed*/);
}

POTHOS_TEST_BLOCK("/numpy/tests", test_save_npz)
{
    std::srand(std::time(0ULL));

    testSaveNPZ(false /*compressed*/);
    testSaveNPZ(true /*compressed*/);
}
