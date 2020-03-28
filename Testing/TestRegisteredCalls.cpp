// Copyright (c) 2019-2020 Nicholas Corgan
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

    auto iinfo = PothosNumPyTests::getAndCallPlugin<Pothos::Proxy>("/numpy/typeinfo/iinfo", dtype);

    POTHOS_TEST_EQUAL(
        dtype.name(),
        iinfo.call("dtype").call<std::string>("name"));
    POTHOS_TEST_EQUAL(
        (sizeof(T) * 8),
        iinfo.call<size_t>("getBits"));
    POTHOS_TEST_EQUAL(
        std::numeric_limits<T>::min(),
        iinfo.call<T>("getMinValue"));
    POTHOS_TEST_EQUAL(
        std::numeric_limits<T>::max(),
        iinfo.call<T>("getMaxValue"));
}

template <typename T>
static PothosNumPyTests::EnableIfNotComplex<T, void> testFloatInfo()
{
    Pothos::DType dtype(typeid(T));
    std::cout << "Testing " << dtype.toString() << std::endl;

    auto finfo = PothosNumPyTests::getAndCallPlugin<Pothos::Proxy>("/numpy/typeinfo/finfo", dtype);

    POTHOS_TEST_EQUAL(
        dtype.name(),
        finfo.call("dtype").call<std::string>("name"));
    POTHOS_TEST_EQUAL(
        (sizeof(T) * 8),
        finfo.call<size_t>("getBits"));
    POTHOS_TEST_EQUAL(
        std::numeric_limits<T>::epsilon(),
        finfo.call<T>("getEpsilon"));
    POTHOS_TEST_EQUAL(
        std::numeric_limits<T>::lowest(),
        finfo.call<T>("getMinValue"));
    POTHOS_TEST_EQUAL(
        (std::numeric_limits<T>::min_exponent - 1),
        finfo.call<ssize_t>("getMinExponent"));
    POTHOS_TEST_EQUAL(
        std::numeric_limits<T>::max(),
        finfo.call<T>("getMaxValue"));
    POTHOS_TEST_EQUAL(
        std::numeric_limits<T>::max_exponent,
        finfo.call<size_t>("getMaxExponent"));
    POTHOS_TEST_EQUAL(
        std::numeric_limits<T>::digits10,
        finfo.call<size_t>("getPrecision"));
    POTHOS_TEST_EQUAL(
        // 1e-precision
        (T(1.0) * std::pow<T>(T(10.0), (std::numeric_limits<T>::digits10 * T(-1.0)))),
        finfo.call<T>("getResolution"));
    POTHOS_TEST_EQUAL(
        std::numeric_limits<T>::min(),
        finfo.call<T>("getMinPositiveValue"));

    // These calls don't have C++ analogues we can test against, so just make
    // sure the call works.
    (void)finfo.call<T>("getNegativeEpsilon");
    (void)finfo.call<size_t>("getExponentBits");
    (void)finfo.call<ssize_t>("getEpsilonExponent");
    (void)finfo.call<ssize_t>("getNegativeEpsilonExponent");
    (void)finfo.call<size_t>("getMantissaBits");
}

template <typename T>
static PothosNumPyTests::EnableIfComplex<T, void> testFloatInfo()
{
    Pothos::DType dtype(typeid(T));
    std::cout << "Testing " << dtype.toString() << std::endl;

    // For complex, just make sure the finfo returned corresponds to the scalar
    // type.
    auto finfo = PothosNumPyTests::getAndCallPlugin<Pothos::Proxy>("/numpy/typeinfo/finfo", dtype);
    auto scalarFInfo = PothosNumPyTests::getAndCallPlugin<Pothos::Proxy>(
                           "/numpy/typeinfo/finfo",
                           Pothos::DType(typeid(typename T::value_type)));

    POTHOS_TEST_EQUAL(
        finfo.call("dtype").call<std::string>("name"),
        scalarFInfo.call("dtype").call<std::string>("name"));
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
    std::string numpyConfigInfo = PothosNumPyTests::getAndCallPlugin<std::string>("/devices/numpy/info");
    POTHOS_TEST_FALSE(numpyConfigInfo.empty());
    auto json = nlohmann::json::parse(numpyConfigInfo);

    std::cout << json.dump() << std::endl;
}
