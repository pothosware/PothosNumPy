// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "SimpleBlockTest.hpp"
#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/Logger.h>
#include <Poco/Thread.h>

#include <algorithm>
#include <cmath>
#include <complex>
#include <functional>
#include <string>
#include <typeinfo>

using SourceTestFunction = std::function<void(const Pothos::BufferChunk&)>;

//
// Common tests
//

template <typename T>
static void testAllValuesEqual(
    const Pothos::BufferChunk& bufferChunk,
    T testValue)
{
    const T* buffer = bufferChunk.as<const T*>();
    const size_t bufferLen = bufferChunk.elements();
    POTHOS_TEST_TRUE(bufferLen > 0);

    for(size_t i = 0; i < bufferLen; ++i)
    {
        testEqual(testValue, buffer[i]);
    }
}

static void testOutputNotEmpty(const Pothos::BufferChunk& bufferChunk)
{
    POTHOS_TEST_TRUE(bufferChunk.elements() > 0);

    Poco::Logger& logger = Poco::Logger::get(__FUNCTION__);
    poco_warning(logger, "This test should do some sort of output validation.");
}

//
// Test code
//

static void testSingleOutputSource(
    const Pothos::Proxy& testSource,
    const Pothos::DType& dtype,
    const SourceTestFunction& testFunction,
    bool forceEnd)
{
    auto collector = Pothos::BlockRegistry::make(
                         "/blocks/collector_sink",
                         dtype);

    // Run the topology
    {
        Pothos::Topology topology;
        topology.connect(testSource, 0, collector, 0);
        topology.commit();

        if(forceEnd)
        {
            // When this block exits, the flowgraph will stop.
            Poco::Thread::sleep(10);
        }
        else
        {
            POTHOS_TEST_TRUE(topology.waitInactive(0.01));
        }
    }

    testFunction(collector.call("getBuffer"));
}

template <typename T>
static void testOnes()
{
    using std::placeholders::_1;

    static const std::string path = "/numpy/ones";

    constexpr size_t dimension = 1;
    Pothos::DType dtype(typeid(T), dimension);
    std::cout << "Testing " << path << "(" << dtype.toString() << ")" << std::endl;

    auto ones = Pothos::BlockRegistry::make(
                    path,
                    dtype);

    testSingleOutputSource(
        ones,
        dtype,
        std::bind(testAllValuesEqual<T>, _1, T(1)),
        true);
}

template <typename T>
static void testZeros()
{
    using std::placeholders::_1;

    static const std::string path = "/numpy/zeros";

    constexpr size_t dimension = 1;
    Pothos::DType dtype(typeid(T), dimension);
    std::cout << "Testing " << path << "(" << dtype.toString() << ")" << std::endl;

    auto zeros = Pothos::BlockRegistry::make(
                     path,
                     dtype);
 
    testSingleOutputSource(
        zeros,
        dtype,
        std::bind(testAllValuesEqual<T>, _1, T(0)),
        true);
}

template <typename T>
static inline EnableIfComplex<T, T> getFullFillValue()
{
    using S = typename T::value_type;

    return T{S(10),S(20)};
}

template <typename T>
static inline EnableIfNotComplex<T, T> getFullFillValue()
{
    return T(5);
}

template <typename T>
static void testFull()
{
    using std::placeholders::_1;

    static const std::string path = "/numpy/full";

    constexpr size_t dimension = 1;
    Pothos::DType dtype(typeid(T), dimension);
    std::cout << "Testing " << path << "(" << dtype.toString() << ")" << std::endl;

    auto full = Pothos::BlockRegistry::make(
                    path,
                    dtype,
                    getFullFillValue<T>());

    const T testValue1 = getFullFillValue<T>();
    const T testValue2 = testValue1 + T(1);

    // Check the initial value.
    testEqual(
        testValue1,
        full.template call<T>("getFillValue"));

    testSingleOutputSource(
        full,
        dtype,
        std::bind(testAllValuesEqual<T>, _1, testValue1),
        true);

    // Set the value, test it, and try again with the new value.
    full.call("setFillValue", testValue2);
    testEqual(
        testValue2,
        full.template call<T>("getFillValue"));

    testSingleOutputSource(
        full,
        dtype,
        std::bind(testAllValuesEqual<T>, _1, testValue2),
        true);
}

template <typename T>
static void testARange()
{
    Pothos::DType dtype(typeid(T));
    std::cout << "Testing " << dtype.toString() << std::endl;

    auto arange = Pothos::BlockRegistry::make(
                     "/numpy/arange",
                     dtype,
                     T(0),
                     T(10),
                     T(1));

    // TODO: output validation
    testSingleOutputSource(
        arange,
        dtype,
        testOutputNotEmpty,
        true);
}

template <typename T>
static void testSpace(const std::string& path)
{
    Pothos::DType dtype(typeid(T));
    std::cout << "Testing " << path << "(" << dtype.toString() << ")" << std::endl;

    auto arange = Pothos::BlockRegistry::make(
                     path,
                     dtype,
                     T(1),
                     T(10),
                     T(5));

    // TODO: output validation
    testSingleOutputSource(
        arange,
        dtype,
        testOutputNotEmpty,
        true);
}

//
// Test code
//

// Called for all types
template <typename T>
static void testCommon()
{
    testOnes<T>();
    testZeros<T>();
    testFull<T>();
}

template <typename T>
static EnableIfInteger<T, void> testSingleOutputSources()
{
    testCommon<T>();
    testSpace<float>("/numpy/linspace");
    testSpace<float>("/numpy/logspace");
    testSpace<float>("/numpy/geomspace");
}

template <typename T>
static EnableIfUnsignedInt<T, void> testSingleOutputSources()
{
    testCommon<T>();
    testSpace<T>("/numpy/linspace");
    testSpace<T>("/numpy/logspace");
    testSpace<T>("/numpy/geomspace");
}

template <typename T>
static EnableIfFloat<T, void> testSingleOutputSources()
{
    testCommon<T>();
    testSpace<T>("/numpy/linspace");
    testSpace<T>("/numpy/logspace");
    testSpace<T>("/numpy/geomspace");
}

template <typename T>
static EnableIfComplex<T, void> testSingleOutputSources()
{
    testCommon<T>();
}

POTHOS_TEST_BLOCK("/numpy/tests", test_single_output_sources)
{
    // SFINAE will direct these calls to the functions that test
    // blocks applicable to the given type.
    testSingleOutputSources<std::int8_t>();
    testSingleOutputSources<std::int16_t>();
    testSingleOutputSources<std::int32_t>();
    testSingleOutputSources<std::int64_t>();
    testSingleOutputSources<std::uint8_t>();
    testSingleOutputSources<std::uint16_t>();
    testSingleOutputSources<std::uint32_t>();
    testSingleOutputSources<std::uint64_t>();
    testSingleOutputSources<float>();
    testSingleOutputSources<double>();
    testSingleOutputSources<std::complex<float>>();
    testSingleOutputSources<std::complex<double>>();
}
