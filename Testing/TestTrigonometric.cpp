// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Framework.hpp>
#include <Pothos/Testing.hpp>

#include <iostream>
#include <vector>

#include <cmath>

//
// Utility code
//

constexpr size_t bufferLen = 64;

//
// To be used when we want both sides of a noncontinuous domain
//
// ----------------|   |----------------
//
template <typename T>
std::vector<T> linspaceOutsideRange(T leftMin, T leftMax, T rightMin, T rightMax, size_t N)
{
    auto output = NPTests::linspace<T>(leftMin, leftMax, N / 2);
    auto rightLinSpace = NPTests::linspace<T>(rightMin, rightMax, N / 2);

    output.insert(output.end(), rightLinSpace.begin(), rightLinSpace.end());
    return output;
}

//
// Test implementation
//

template <typename T>
struct TestParams
{
    Pothos::BufferChunk inputs;
    Pothos::BufferChunk expectedOutputs;

    TestParams(const std::vector<T>& inputVec)
    {
        static const Pothos::DType dtype(typeid(T));

        inputs = NPTests::stdVectorToBufferChunk(inputVec);
        expectedOutputs = Pothos::BufferChunk(dtype, inputVec.size());
    }
};

template <typename T>
static void testTrigonometricOperation(
    const std::string& blockPath,
    const TestParams<T>& testParams)
{
    std::cout << " * Testing " << blockPath << "..." << std::endl;

    static const Pothos::DType dtype(typeid(T));

    auto feeder = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    feeder.call("feedBuffer", testParams.inputs);

    auto trig = Pothos::BlockRegistry::make(blockPath, dtype);
    auto sink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    {
        Pothos::Topology topology;

        topology.connect(feeder, 0, trig, 0);
        topology.connect(trig, 0, sink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    NPTests::testBufferChunk(
        testParams.expectedOutputs,
        sink.call<Pothos::BufferChunk>("getBuffer"));
}

template <typename T>
static void testTrigonmetricBlockForType()
{
    std::cout << "Testing " << Pothos::DType(typeid(T)).name() << "..." << std::endl;

    // Slightly offset from bounds of domain with real values
    const auto piDiv2Inputs = NPTests::linspace<T>(T(-M_PI/2 + 0.01), T(M_PI/2 - 0.01), bufferLen);
    const auto oneToOneInputs = NPTests::linspace<T>(T(-0.99), T(0.99), bufferLen);
    const auto oneToPiInputs = NPTests::linspace<T>(T(1.01), T(M_PI), bufferLen);
    const auto zeroToOneInputs = NPTests::linspace<T>(T(0.01), T(0.99), bufferLen);

    const auto outsideOneToOneInputs = linspaceOutsideRange<T>(
                                           T(-M_PI / 2 + 0.01), T(-1.01),
                                           T(1.01), T(M_PI / 2 + 0.01),
                                           bufferLen);

    TestParams<T> cosTestParams(piDiv2Inputs);
    TestParams<T> sinTestParams(piDiv2Inputs);
    TestParams<T> tanTestParams(piDiv2Inputs);

    TestParams<T> acosTestParams(oneToOneInputs);
    TestParams<T> asinTestParams(oneToOneInputs);
    TestParams<T> atanTestParams(oneToOneInputs);

    TestParams<T> coshTestParams(piDiv2Inputs);
    TestParams<T> sinhTestParams(piDiv2Inputs);
    TestParams<T> tanhTestParams(piDiv2Inputs);

    TestParams<T> acoshTestParams(oneToPiInputs);
    TestParams<T> asinhTestParams(piDiv2Inputs);
    TestParams<T> atanhTestParams(oneToOneInputs);

    for (size_t elem = 0; elem < bufferLen; ++elem)
    {
        cosTestParams.expectedOutputs.template as<T*>()[elem] = std::cos(piDiv2Inputs[elem]);
        sinTestParams.expectedOutputs.template as<T*>()[elem] = std::sin(piDiv2Inputs[elem]);
        tanTestParams.expectedOutputs.template as<T*>()[elem] = std::tan(piDiv2Inputs[elem]);

        acosTestParams.expectedOutputs.template as<T*>()[elem] = std::acos(oneToOneInputs[elem]);
        asinTestParams.expectedOutputs.template as<T*>()[elem] = std::asin(oneToOneInputs[elem]);
        atanTestParams.expectedOutputs.template as<T*>()[elem] = std::atan(oneToOneInputs[elem]);

        coshTestParams.expectedOutputs.template as<T*>()[elem] = std::cosh(piDiv2Inputs[elem]);
        sinhTestParams.expectedOutputs.template as<T*>()[elem] = std::sinh(piDiv2Inputs[elem]);
        tanhTestParams.expectedOutputs.template as<T*>()[elem] = std::tanh(piDiv2Inputs[elem]);

        acoshTestParams.expectedOutputs.template as<T*>()[elem] = std::acosh(oneToPiInputs[elem]);
        asinhTestParams.expectedOutputs.template as<T*>()[elem] = std::asinh(piDiv2Inputs[elem]);
        atanhTestParams.expectedOutputs.template as<T*>()[elem] = std::atanh(oneToOneInputs[elem]);
    }

    testTrigonometricOperation<T>("/numpy/cos", cosTestParams);
    testTrigonometricOperation<T>("/numpy/sin", sinTestParams);
    testTrigonometricOperation<T>("/numpy/tan", tanTestParams);

    testTrigonometricOperation<T>("/numpy/arccos", acosTestParams);
    testTrigonometricOperation<T>("/numpy/arcsin", asinTestParams);
    testTrigonometricOperation<T>("/numpy/arctan", atanTestParams);

    testTrigonometricOperation<T>("/numpy/cosh", coshTestParams);
    testTrigonometricOperation<T>("/numpy/sinh", sinhTestParams);
    testTrigonometricOperation<T>("/numpy/tanh", tanhTestParams);

    testTrigonometricOperation<T>("/numpy/arccosh", acoshTestParams);
    testTrigonometricOperation<T>("/numpy/arcsinh", asinhTestParams);
    testTrigonometricOperation<T>("/numpy/arctanh", atanhTestParams);
}

POTHOS_TEST_BLOCK("/numpy/tests", test_trigonometric)
{
    testTrigonmetricBlockForType<float>();
    testTrigonmetricBlockForType<double>();
}
