// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "Testing/TestUtility.hpp"

#include <Pothos/Callable.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/Thread.h>

#include <complex>
#include <cstdint>
#include <iostream>
#include <string>
#include <typeinfo>

//
// Main test function
//

static void testRandomSourcesFunc(
    const Pothos::Proxy& randomSource,
    const std::string& blockRegistryPath,
    const Pothos::DType& dtype)
{
    std::cout << "Testing " << blockRegistryPath << "(" << dtype.toString() << ")" << std::endl;

    auto collectorSink = Pothos::BlockRegistry::make(
                             "/blocks/collector_sink",
                             dtype);

    // Execute the topology.
    {
        Pothos::Topology topology;
        topology.connect(
            randomSource,
            0,
            collectorSink,
            0);
        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }

    POTHOS_TEST_TRUE(collectorSink.call("getBuffer").call<size_t>("elements") > 0);
}

//
// Per-block functions
//

// TODO: move to standard test, since this isn't a source
template <typename T>
static void testPermutation()
{
    /*const Pothos::DType dtype(typeid(T));
    const std::string blockRegistryPath = "/numpy/random/permutation";

    auto permutation = Pothos::BlockRegistry::make(
                           blockRegistryPath,
                           dtype);
    testRandomSourcesFunc(
        permutation,
        blockRegistryPath,
        dtype);*/
}

template <typename T>
static void testBinomial()
{
    const Pothos::DType dtype(typeid(T));

    const std::string binomialPath = "/numpy/random/binomial";
    constexpr T N = T(10);
    constexpr double P = 0.5;

    auto binomial = Pothos::BlockRegistry::make(
                         binomialPath,
                         dtype,
                         N,
                         P);

    // Test initial values.
    testEqual(
        N,
        binomial.template call<T>("getN"));
    testEqual(
        P,
        binomial.template call<double>("getP"));

    testRandomSourcesFunc(
        binomial,
        binomialPath,
        dtype);

    // Test the setters by changing values.
    constexpr T N2 = T(10);
    constexpr float P2 = 0.25;

    binomial.call("setN", N2);
    binomial.call("setP", P2);
    testEqual(
        N2,
        binomial.template call<T>("getN"));
    testEqual(
        P2,
        binomial.template call<float>("getP"));

    // Check input validation.
    POTHOS_TEST_THROWS(
        binomial.template call("setP", T(-0.01)),
        Pothos::ProxyExceptionMessage);
}

//
// Test code
//

template <typename T>
static EnableIfInteger<T, void> testRandomSources()
{
    const Pothos::DType dtype(typeid(T));

    testPermutation<T>();
    testBinomial<T>();

    //
    // Integers
    //

    const std::string integersPath = "/numpy/random/integers";
    auto integers = Pothos::BlockRegistry::make(
                        integersPath,
                        dtype);
    testRandomSourcesFunc(
        integers,
        integersPath,
        dtype);
}

template <typename T>
static EnableIfUnsignedInt<T, void> testRandomSources()
{
    testPermutation<T>();
    testBinomial<T>();
}

template <typename T>
static EnableIfFloat<T, void> testRandomSources()
{
    const Pothos::DType dtype(typeid(T));

    testPermutation<T>();

    //
    // Beta
    //
    {
        const std::string betaPath = "/numpy/random/beta";
        constexpr T param1 = T(2.0);
        constexpr T param2 = T(5.0);

        auto beta = Pothos::BlockRegistry::make(
                             betaPath,
                             dtype,
                             param1,
                             param2);

        // Test initial values.
        testEqual(
            param1,
            beta.template call<T>("getAlpha"));
        testEqual(
            param2,
            beta.template call<T>("getBeta"));

        testRandomSourcesFunc(
            beta,
            betaPath,
            dtype);

        // Test the setters by switching values.
        beta.call("setAlpha", param2);
        beta.call("setBeta", param1);
        testEqual(
            param2,
            beta.template call<T>("getAlpha"));
        testEqual(
            param1,
            beta.template call<T>("getBeta"));

        // Check input validation.
        POTHOS_TEST_THROWS(
            beta.template call("setAlpha", T(0.0)),
            Pothos::ProxyExceptionMessage);
        POTHOS_TEST_THROWS(
            beta.template call("setBeta", T(0.0)),
            Pothos::ProxyExceptionMessage);
    }

    //
    // ChiSquare
    //
    {
        const std::string chisquarePath = "/numpy/random/chisquare";
        constexpr T df1 = T(2.5);
        constexpr T df2 = T(5.2);

        auto chisquare = Pothos::BlockRegistry::make(
                             chisquarePath,
                             dtype,
                             df1);

        // Test initial values.
        testEqual(
            df1,
            chisquare.template call<T>("getDegreesOfFreedom"));

        testRandomSourcesFunc(
            chisquare,
            chisquarePath,
            dtype);

        // Test the setters by switching values.
        chisquare.call("setDegreesOfFreedom", df2);
        testEqual(
            df2,
            chisquare.template call<T>("getDegreesOfFreedom"));

        // Check input validation.
        POTHOS_TEST_THROWS(
            chisquare.template call("setDegreesOfFreedom", T(0.0)),
            Pothos::ProxyExceptionMessage);
    }

    //
    // Exponential
    //
    {
        const std::string exponentialPath = "/numpy/random/exponential";
        constexpr T scale1 = T(2.5);
        constexpr T scale2 = T(5.2);

        auto exponential = Pothos::BlockRegistry::make(
                             exponentialPath,
                             dtype,
                             scale1);

        // Test initial values.
        testEqual(
            scale1,
            exponential.template call<T>("getScale"));

        testRandomSourcesFunc(
            exponential,
            exponentialPath,
            dtype);

        // Test the setters by switching values.
        exponential.call("setScale", scale2);
        testEqual(
            scale2,
            exponential.template call<T>("getScale"));

        // Check input validation.
        POTHOS_TEST_THROWS(
            exponential.template call("setScale", T(-0.01)),
            Pothos::ProxyExceptionMessage);
    }
}

template <typename T>
static EnableIfComplex<T, void> testRandomSources()
{
    testPermutation<T>();
}

POTHOS_TEST_BLOCK("/numpy/tests", test_random_sources)
{
    // SFINAE will make these call the functions with the
    // applicable blocks.
    testRandomSources<std::int8_t>();
    testRandomSources<std::int16_t>();
    testRandomSources<std::int32_t>();
    testRandomSources<std::int64_t>();
    testRandomSources<std::uint8_t>();
    testRandomSources<std::uint16_t>();
    testRandomSources<std::uint32_t>();
    testRandomSources<std::uint64_t>();
    testRandomSources<float>();
    testRandomSources<double>();
    testRandomSources<std::complex<float>>();
    testRandomSources<std::complex<double>>();
}
