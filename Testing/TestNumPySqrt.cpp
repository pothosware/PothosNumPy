// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SimpleBlockTest.hpp"
#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <algorithm>
#include <cmath>
#include <complex>
#include <string>

template <typename T>
using TestFunc = T(*)(T);

template <typename T>
static SimpleBlockTestParams<T> getTestParamsForFunc(TestFunc<T> func)
{
    static const size_t numInputs = 123;

    SimpleBlockTestParams<T> testParams =
    {
        "/scipy/numpy/sqrt",
        linspace<T>(10.0f, 20.0f, numInputs),
        {},
        1e-6f
    };
    testParams.expectedOutputs.reserve(numInputs);

    std::transform(
        testParams.inputs.begin(),
        testParams.inputs.end(),
        std::back_inserter(testParams.expectedOutputs),
        func);

    return testParams;
}

template <typename T>
static void testNumPySqrt()
{
    simpleBlockTest(getTestParamsForFunc<T>(std::sqrt));
}

// TODO: test all other types
POTHOS_TEST_BLOCK("/scipy/tests", TestNumPySqrt)
{
    testNumPySqrt<float>();
    testNumPySqrt<double>();
}
