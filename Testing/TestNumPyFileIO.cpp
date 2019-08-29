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

#include <cstdint>
#include <complex>
#include <iostream>
#include <random>
#include <string>
#include <vector>

//
// Utility code
//

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
static std::vector<T> getRandomInputs(size_t numElements)
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

    // Execute the topology.
    {
        Pothos::Topology topology;
        topology.connect(
            vectorSource, 0,
            numpySave, 0);

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(5);
    }

    POTHOS_TEST_TRUE(Poco::File(filepath).exists());
    POTHOS_TEST_TRUE(
        Poco::File(filepath).getSize() >=
        (numElements * sizeof(T)));

    //
    // Read from the .NPY file and check the file contents.
    //

    auto numpyLoad = Pothos::BlockRegistry::make(
                         "/numpy/load",
                         dtype,
                         filepath);

    auto collectorSink = Pothos::BlockRegistry::make(
                             "/blocks/collector_sink",
                             dtype);

    // Execute the topology.
    {
        Pothos::Topology topology;
        topology.connect(
            numpyLoad, 0,
            collectorSink, 0);

        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(5);
    }

    testBufferChunk<T>(
        collectorSink.call("getBuffer"),
        randomInputs,
        getEpsilon<T>());
}

template <typename T>
static void testNPZIO()
{
    const Pothos::DType dtype(typeid(T));
    std::cout << "Testing " << dtype.toString() << std::endl;
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

    testNPZIO<std::int8_t>();
    testNPZIO<std::int16_t>();
    testNPZIO<std::int32_t>();
    testNPZIO<std::int64_t>();
    testNPZIO<std::uint8_t>();
    testNPZIO<std::uint16_t>();
    testNPZIO<std::uint32_t>();
    testNPZIO<std::uint64_t>();
    testNPZIO<float>();
    testNPZIO<double>();
    testNPZIO<std::complex<float>>();
    testNPZIO<std::complex<double>>();
}
