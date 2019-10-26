// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <nlohmann/json.hpp>

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>

//
// Helper/commmon code
//

template <typename T>
static void testIntInfo()
{
    Pothos::DType dtype(typeid(T));
    std::cout << "Testing " << dtype.toString() << std::endl;

    auto iinfo = getAndCallPlugin<Pothos::Proxy>("/numpy/info/iinfo", dtype);

    // This should be enough to confirm that this is the type we were hoping
    // for. Past this, we run into limitations of types coming through Pothos,
    // so just make sure the fields exist.
    testEqual<int>(
        (sizeof(T) * 8),
        iinfo.get<int>("bits"));
    (void)iinfo.get<T>("min");

    // Pothos's converters have an issue getting this value out of Python,
    // so we're going with the assumption that if this value exists elsewhere,
    // then it exists here.
    if(!std::is_same<T, std::uint64_t>::value)
    {
        (void)iinfo.get<T>("max");
    }
}

template <typename T>
static EnableIfNotComplex<T, void> testFloatInfo()
{
    Pothos::DType dtype(typeid(T));
    std::cout << "Testing " << dtype.toString() << std::endl;

    auto finfo = getAndCallPlugin<Pothos::Proxy>("/numpy/info/finfo", dtype);

    // This should be enough to confirm that this is the type we were hoping
    // for. Past this, we run into limitations of types coming through Pothos,
    // so just make sure the fields exist.
    testEqual<int>(
        (sizeof(T) * 8),
        finfo.get<int>("bits"));
    (void)finfo.get<T>("eps");
    (void)finfo.get<T>("epsneg");
    (void)finfo.get<int>("iexp");
    (void)finfo.get<int>("machep");
    (void)finfo.get<T>("max");
    (void)finfo.get<int>("maxexp");
    (void)finfo.get<T>("min");
    (void)finfo.get<int>("minexp");
    (void)finfo.get<int>("negep");
    (void)finfo.get<int>("nexp");
    (void)finfo.get<int>("nmant");
    (void)finfo.get<int>("precision");
    (void)finfo.get<T>("resolution");
    (void)finfo.get<T>("tiny");
}

template <typename T>
static EnableIfComplex<T, void> testFloatInfo()
{
    Pothos::DType dtype(typeid(T));
    std::cout << "Testing " << dtype.toString() << std::endl;

    // For complex, just make sure the finfo returned corresponds to the scalar
    // type.
    auto finfo = getAndCallPlugin<Pothos::Proxy>("/numpy/info/finfo", dtype);
    auto scalarFInfo = getAndCallPlugin<Pothos::Proxy>(
                           "/numpy/info/finfo",
                           Pothos::DType(typeid(typename T::value_type)));

    testEqual<std::string>(
        finfo.get<Pothos::Proxy>("dtype").get<std::string>("name"),
        scalarFInfo.get<Pothos::Proxy>("dtype").get<std::string>("name"));
}

//
// Actual tests
//

POTHOS_TEST_BLOCK("/numpy/tests", test_iinfo)
{
    // This tests that the NumPy type proxy works and that the fields can
    // be queried as expected.
    testIntInfo<std::int8_t>();
    testIntInfo<std::int16_t>();
    testIntInfo<std::int32_t>();
    testIntInfo<std::int64_t>();
    testIntInfo<std::uint8_t>();
    testIntInfo<std::uint16_t>();
    testIntInfo<std::uint32_t>();
    testIntInfo<std::uint64_t>();
}

POTHOS_TEST_BLOCK("/numpy/tests", test_finfo)
{
    // This tests that the NumPy type proxy works and that the fields can
    // be queried as expected.
    testFloatInfo<float>();
    testFloatInfo<double>();
    testFloatInfo<std::complex<float>>();
    testFloatInfo<std::complex<double>>();
}

POTHOS_TEST_BLOCK("/numpy/tests", test_registered_string_calls)
{
    // Make sure this is valid JSON. This will throw if the JSON is invalid.
    std::string numpyConfigInfo = getAndCallPlugin<std::string>("/devices/numpy/info");
    POTHOS_TEST_TRUE(!numpyConfigInfo.empty());
    auto json = nlohmann::json::parse(numpyConfigInfo);

    std::cout << json.dump() << std::endl;
}

POTHOS_TEST_BLOCK("/numpy/tests", test_windows)
{
    //
    // Make sure the Pothos proxy calls to NumPy go back and forth correctly.
    // All expected values are from the NumPy documentation.
    //

    //
    // Bartlett
    //

    const std::vector<double> expectedBartlett =
    {
        0.0       ,  0.18181818,  0.36363636,  0.54545455,  0.72727273,
        0.90909091,  0.90909091,  0.72727273,  0.54545455,  0.36363636,
        0.18181818,  0.0
    };
    const auto calculatedBartlett = getAndCallPlugin<Pothos::BufferChunk>(
                                        "/numpy/window/bartlett",
                                        expectedBartlett.size());

    std::cout << "Testing Bartlett." << std::endl;
    testBufferChunk(
        calculatedBartlett,
        expectedBartlett);

    //
    // Blackman
    //

    const std::vector<double> expectedBlackman =
    {
        -1.38777878e-17,  3.26064346e-02,   1.59903635e-01,
        4.14397981e-01,   7.36045180e-01,   9.67046769e-01,
        9.67046769e-01,   7.36045180e-01,   4.14397981e-01,
        1.59903635e-01,   3.26064346e-02,  -1.38777878e-17
    };
    const auto calculatedBlackman = getAndCallPlugin<Pothos::BufferChunk>(
                                        "/numpy/window/blackman",
                                        expectedBlackman.size());

    std::cout << "Testing Blackman." << std::endl;
    testBufferChunk(
        calculatedBlackman,
        expectedBlackman);

    //
    // Hamming
    //

    const std::vector<double> expectedHamming =
    {
        0.08      ,  0.15302337,  0.34890909,  0.60546483,  0.84123594,
        0.98136677,  0.98136677,  0.84123594,  0.60546483,  0.34890909,
        0.15302337,  0.08
    };
    const auto calculatedHamming = getAndCallPlugin<Pothos::BufferChunk>(
                                       "/numpy/window/hamming",
                                       expectedHamming.size());

    std::cout << "Testing Hamming." << std::endl;
    testBufferChunk(
        calculatedHamming,
        expectedHamming);

    //
    // Hanning
    //

    const std::vector<double> expectedHanning =
    {
        0.0       , 0.07937323, 0.29229249, 0.57115742, 0.82743037,
        0.97974649, 0.97974649, 0.82743037, 0.57115742, 0.29229249,
        0.07937323, 0.0
    };
    const auto calculatedHanning = getAndCallPlugin<Pothos::BufferChunk>(
                                       "/numpy/window/hanning",
                                       expectedHanning.size());

    std::cout << "Testing Hanning." << std::endl;
    testBufferChunk(
        calculatedHanning,
        expectedHanning);

    //
    // Kaiser
    //

    constexpr double kaiserBeta = 14.0;
    const std::vector<double> expectedKaiser =
    {
        7.72686684e-06, 3.46009194e-03, 4.65200189e-02,
        2.29737120e-01, 5.99885316e-01, 9.45674898e-01,
        9.45674898e-01, 5.99885316e-01, 2.29737120e-01,
        4.65200189e-02, 3.46009194e-03, 7.72686684e-06
    };
    const auto calculatedKaiser = getAndCallPlugin<Pothos::BufferChunk>(
                                      "/numpy/window/kaiser",
                                      expectedKaiser.size(),
                                      kaiserBeta);

    std::cout << "Testing Kaiser." << std::endl;
    testBufferChunk(
        calculatedKaiser,
        expectedKaiser);
}
