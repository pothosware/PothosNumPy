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

template <typename BlockType, typename ParamType>
static void testOneParamRandomSource(
    const std::string& blockRegistryPath,
    const std::string& paramName,
    ParamType paramValue1,
    ParamType paramValue2,
    Pothos::Proxy& rBlock)
{
    const Pothos::DType dtype(typeid(BlockType));

    std::cout << "Testing " << blockRegistryPath << "(" << dtype.toString() << ")" << std::endl;

    const std::string getParam = "get" + paramName;
    const std::string setParam = "set" + paramName;

    rBlock = Pothos::BlockRegistry::make(
                 blockRegistryPath,
                 dtype,
                 paramValue1);

    // Test initial values.
    testEqual(
        paramValue1,
        rBlock.template call<ParamType>(getParam));

    testRandomSourcesFunc(
        rBlock,
        blockRegistryPath,
        dtype);

    rBlock.call(setParam, paramValue2);
    testEqual(
        paramValue2,
        rBlock.template call<ParamType>(getParam));
}

template <typename BlockType, typename Param1Type, typename Param2Type>
static void testTwoParamRandomSource(
    const std::string& blockRegistryPath,
    const std::string& param1Name,
    Param1Type param1Value,
    const std::string& param2Name,
    Param2Type param2Value,
    Pothos::Proxy& rBlock)
{
    const Pothos::DType dtype(typeid(BlockType));

    std::cout << "Testing " << blockRegistryPath << "(" << dtype.toString() << ")" << std::endl;

    const std::string getParam1 = "get" + param1Name;
    const std::string setParam1 = "set" + param1Name;
    const std::string getParam2 = "get" + param2Name;
    const std::string setParam2 = "set" + param2Name;

    rBlock = Pothos::BlockRegistry::make(
                 blockRegistryPath,
                 dtype,
                 param1Value,
                 param2Value);

    // Test initial values.
    testEqual(
        param1Value,
        rBlock.template call<Param1Type>(getParam1));
    testEqual(
        param2Value,
        rBlock.template call<Param2Type>(getParam2));

    testRandomSourcesFunc(
        rBlock,
        blockRegistryPath,
        dtype);

    rBlock.call(setParam1, param2Value);
    rBlock.call(setParam2, param1Value);
    testEqual(
        param2Value,
        rBlock.template call<Param2Type>(getParam1));
    testEqual(
        param1Value,
        rBlock.template call<Param1Type>(getParam2));
}

template <typename BlockType, typename Param1Type, typename Param2Type, typename Param3Type>
static void testThreeParamRandomSource(
    const std::string& blockRegistryPath,
    const std::string& param1Name,
    Param1Type param1Value,
    const std::string& param2Name,
    Param2Type param2Value,
    const std::string& param3Name,
    Param3Type param3Value,
    Pothos::Proxy& rBlock)
{
    const Pothos::DType dtype(typeid(BlockType));

    std::cout << "Testing " << blockRegistryPath << "(" << dtype.toString() << ")" << std::endl;

    const std::string getParam1 = "get" + param1Name;
    const std::string setParam1 = "set" + param1Name;
    const std::string getParam2 = "get" + param2Name;
    const std::string setParam2 = "set" + param2Name;
    const std::string getParam3 = "get" + param3Name;
    const std::string setParam3 = "set" + param3Name;

    rBlock = Pothos::BlockRegistry::make(
                 blockRegistryPath,
                 dtype,
                 param1Value,
                 param2Value,
                 param3Value);

    // Test initial values.
    testEqual(
        param1Value,
        rBlock.template call<Param1Type>(getParam1));
    testEqual(
        param2Value,
        rBlock.template call<Param2Type>(getParam2));
    testEqual(
        param3Value,
        rBlock.template call<Param3Type>(getParam3));

    testRandomSourcesFunc(
        rBlock,
        blockRegistryPath,
        dtype);
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
static void testIntegers()
{
    const Pothos::DType dtype(typeid(T));

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
static void testBinomial()
{
    const Pothos::DType dtype(typeid(T));

    const std::string binomialPath = "/numpy/random/binomial";
    constexpr T N = T(10);
    constexpr double P = 0.5;

    std::cout << "Testing " << binomialPath << "(" << dtype.toString() << ")" << std::endl;

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
        binomial.template call("setP", -0.01),
        Pothos::ProxyExceptionMessage);
}

template <typename T>
static EnableIfUnsignedInt<T, void> testHypergeometricInvalidMin(const Pothos::Proxy&)
{
}

template <typename T>
static EnableIfInteger<T, void> testHypergeometricInvalidMin(const Pothos::Proxy& hypergeometric)
{
    POTHOS_TEST_THROWS(
        hypergeometric.template call("setNumGood", T(-1)),
        Pothos::ProxyExceptionMessage);
    POTHOS_TEST_THROWS(
        hypergeometric.template call("setNumBad", T(-1)),
        Pothos::ProxyExceptionMessage);
    POTHOS_TEST_THROWS(
        hypergeometric.template call("setNumSampled", T(-1)),
        Pothos::ProxyExceptionMessage);
}

template <typename T>
static EnableIfTypeSizeIsLT<T, void, 8> testHypergeometricInvalidMax(const Pothos::Proxy&)
{
}

template <typename T>
static EnableIfTypeSizeIsGE<T, void, 8> testHypergeometricInvalidMax(const Pothos::Proxy& hypergeometric)
{
    constexpr T max = T(10e9);

    POTHOS_TEST_THROWS(
        hypergeometric.template call("setNumGood", (max+1)),
        Pothos::ProxyExceptionMessage);
    POTHOS_TEST_THROWS(
        hypergeometric.template call("setNumBad", (max+1)),
        Pothos::ProxyExceptionMessage);
}

//
// Test code
//

template <typename T>
static EnableIfAnyInt<T, void> testRandomSources()
{
    testPermutation<T>();
    testIntegers<T>();
    testBinomial<T>();

    //
    // Geometric
    //
    {
        const std::string geometricPath = "/numpy/random/geometric";
        Pothos::Proxy geometric;
        constexpr double P1 = 0.05;
        constexpr double P2 = 0.95;

        testOneParamRandomSource<T, double>(
            geometricPath,
            "P",
            P1,
            P2,
            geometric);

        // Check input validation.
        POTHOS_TEST_THROWS(
            geometric.template call("setP", -0.01),
            Pothos::ProxyExceptionMessage);
        POTHOS_TEST_THROWS(
            geometric.template call("setP", 1.01),
            Pothos::ProxyExceptionMessage);
    }

    //
    // Hypergeometric
    //
    {
        const std::string hypergeometricPath = "/numpy/random/hypergeometric";
        Pothos::Proxy hypergeometric;
        constexpr T numGood = 5;
        constexpr T numBad = 10;

        testThreeParamRandomSource<T, T, T, T>(
            hypergeometricPath,
            "NumGood",
            numGood,
            "NumBad",
            numBad,
            "NumSampled",
            (numGood + numBad),
            hypergeometric);

        // Check input validation.
        testHypergeometricInvalidMin<T>(hypergeometric);
        testHypergeometricInvalidMax<T>(hypergeometric);

        /*constexpr T max = T(10e9);

        POTHOS_TEST_THROWS(
            hypergeometric.template call("setNumGood", (max+1)),
            Pothos::ProxyExceptionMessage);
        POTHOS_TEST_THROWS(
            hypergeometric.template call("setNumBad", (max+1)),
            Pothos::ProxyExceptionMessage);*/
    }

    //
    // LogSeries
    //
    {
        const std::string logseriesPath = "/numpy/random/logseries";
        Pothos::Proxy logseries;
        constexpr double P1 = 0.05;
        constexpr double P2 = 0.95;

        testOneParamRandomSource<T, double>(
            logseriesPath,
            "P",
            P1,
            P2,
            logseries);

        // Check input validation.
        POTHOS_TEST_THROWS(
            logseries.template call("setP", 0.0),
            Pothos::ProxyExceptionMessage);
        POTHOS_TEST_THROWS(
            logseries.template call("setP", 1.0),
            Pothos::ProxyExceptionMessage);
    }
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
        Pothos::Proxy beta;
        constexpr T param1 = T(2.0);
        constexpr T param2 = T(5.0);

        testTwoParamRandomSource<T, T, T>(
            betaPath,
            "Alpha",
            param1,
            "Beta",
            param2,
            beta);

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
        Pothos::Proxy chisquare;
        constexpr T df1 = T(2.5);
        constexpr T df2 = T(5.2);

        testOneParamRandomSource<T, T>(
            chisquarePath,
            "DegreesOfFreedom",
            df1,
            df2,
            chisquare);

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
        Pothos::Proxy exponential;
        constexpr T scale1 = T(2.5);
        constexpr T scale2 = T(5.2);

        testOneParamRandomSource<T, T>(
            exponentialPath,
            "Scale",
            scale1,
            scale2,
            exponential);

        // Check input validation.
        POTHOS_TEST_THROWS(
            exponential.template call("setScale", T(-0.01)),
            Pothos::ProxyExceptionMessage);
    }

    //
    // F
    //
    {
        const std::string fPath = "/numpy/random/f";
        Pothos::Proxy f;
        constexpr T param1 = T(2.5);
        constexpr T param2 = T(5.2);

        testTwoParamRandomSource<T, T, T>(
            fPath,
            "Numerator",
            param1,
            "Denominator",
            param2,
            f);

        // Check input validation.
        POTHOS_TEST_THROWS(
            f.template call("setNumerator", T(-0.01)),
            Pothos::ProxyExceptionMessage);
        POTHOS_TEST_THROWS(
            f.template call("setDenominator", T(0.0)),
            Pothos::ProxyExceptionMessage);
    }

    //
    // Gamma
    //
    {
        const std::string gammaPath = "/numpy/random/gamma";
        Pothos::Proxy gamma;
        constexpr T param1 = T(2.5);
        constexpr T param2 = T(5.2);

        testTwoParamRandomSource<T, T, T>(
            gammaPath,
            "Shape",
            param1,
            "Scale",
            param2,
            gamma);

        // Check input validation.
        POTHOS_TEST_THROWS(
            gamma.template call("setShape", T(-0.01)),
            Pothos::ProxyExceptionMessage);
        POTHOS_TEST_THROWS(
            gamma.template call("setScale", T(-0.01)),
            Pothos::ProxyExceptionMessage);
    }

    //
    // Gumbel
    //
    {
        const std::string gumbelPath = "/numpy/random/gumbel";
        Pothos::Proxy gumbel;
        constexpr T param1 = T(2.5);
        constexpr T param2 = T(5.2);

        testTwoParamRandomSource<T, T, T>(
            gumbelPath,
            "Location",
            param1,
            "Scale",
            param2,
            gumbel);

        // Check input validation.
        POTHOS_TEST_THROWS(
            gumbel.template call("setScale", T(-0.01)),
            Pothos::ProxyExceptionMessage);
    }

    //
    // Laplace
    //
    {
        const std::string laplacePath = "/numpy/random/laplace";
        Pothos::Proxy laplace;
        constexpr T param1 = T(2.5);
        constexpr T param2 = T(5.2);

        testTwoParamRandomSource<T, T, T>(
            laplacePath,
            "Position",
            param1,
            "Scale",
            param2,
            laplace);

        // Check input validation.
        POTHOS_TEST_THROWS(
            laplace.template call("setScale", T(-0.01)),
            Pothos::ProxyExceptionMessage);
    }

    //
    // Logistic
    //
    {
        const std::string logisticPath = "/numpy/random/logistic";
        Pothos::Proxy logistic;
        constexpr T param1 = T(2.5);
        constexpr T param2 = T(5.2);

        testTwoParamRandomSource<T, T, T>(
            logisticPath,
            "Position",
            param1,
            "Scale",
            param2,
            logistic);

        // Check input validation.
        POTHOS_TEST_THROWS(
            logistic.template call("setScale", T(-0.01)),
            Pothos::ProxyExceptionMessage);
    }

    //
    // LogNormal
    //
    {
        const std::string lognormalPath = "/numpy/random/lognormal";
        Pothos::Proxy lognormal;
        constexpr T param1 = T(2.5);
        constexpr T param2 = T(5.2);

        testTwoParamRandomSource<T, T, T>(
            lognormalPath,
            "Mean",
            param1,
            "Sigma",
            param2,
            lognormal);

        // Check input validation.
        POTHOS_TEST_THROWS(
            lognormal.template call("setSigma", T(-0.01)),
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
