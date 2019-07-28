// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: GPL-3.0-or-later

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
    std::string numpyConfigInfo = getAndCallPlugin<std::string>("/numpy/info/config_info");
    POTHOS_TEST_TRUE(!numpyConfigInfo.empty());
    auto json = nlohmann::json::parse(numpyConfigInfo);

    std::string numpyVersion = getAndCallPlugin<std::string>("/numpy/info/version");
    POTHOS_TEST_TRUE(!numpyVersion.empty());
}
