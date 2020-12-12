// Copyright (c) 2014-2016 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>

#include <algorithm>
#include <complex>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <vector>

//
// Common
//

constexpr size_t bufferLen = 64;

struct ArithmeticTestValues
{
    std::vector<Pothos::BufferChunk> inputs;
    Pothos::BufferChunk expectedOutputs;

    template <typename T>
    void setup(size_t numInputs, size_t bufferLength)
    {
        static const Pothos::DType dtype(typeid(T));

        for (size_t input = 0; input < numInputs; ++input)
        {
            inputs.emplace_back(Pothos::BufferChunk(dtype, bufferLength));
        }

        expectedOutputs = Pothos::BufferChunk(dtype, bufferLength);
    }
};

template <typename T>
static NPTests::EnableIfNotComplex<T, ArithmeticTestValues> getAddTestValues()
{
    constexpr auto numInputs = 3;

    static const Pothos::DType dtype(typeid(T));

    ArithmeticTestValues testValues;
    testValues.setup<T>(numInputs, bufferLen);

    for (size_t elem = 0; elem < bufferLen; ++elem)
    {
        testValues.inputs[0].as<T*>()[elem] = static_cast<T>(elem);
        testValues.inputs[1].as<T*>()[elem] = static_cast<T>(elem/2);
        testValues.inputs[2].as<T*>()[elem] = static_cast<T>(elem/4);

        if (std::is_signed<T>::value)
        {
            testValues.inputs[1].as<T*>()[elem] *= -1;
            testValues.inputs[2].as<T*>()[elem] *= -1;

            testValues.expectedOutputs.as<T*>()[elem] = static_cast<T>(elem - (elem / 2) - (elem / 4));
        }
        else
        {
            testValues.expectedOutputs.as<T*>()[elem] = static_cast<T>(elem + (elem / 2) + (elem / 4));
        }
    }

    return testValues;
}

// Fully co-opt the scalar implementation since complex addition is just (real+real, imag+imag)
template <typename T>
static NPTests::EnableIfComplex<T, ArithmeticTestValues> getAddTestValues()
{
    using ScalarType = typename T::value_type;
    static const Pothos::DType dtype(typeid(T));

    auto testValues = getAddTestValues<ScalarType>();
    for (auto& input : testValues.inputs) input.dtype = dtype;
    testValues.expectedOutputs.dtype = dtype;

    return testValues;
}

template <typename T>
static NPTests::EnableIfNotComplex<T, ArithmeticTestValues> getSubTestValues()
{
    constexpr auto numInputs = 2;

    static const Pothos::DType dtype(typeid(T));

    ArithmeticTestValues testValues;
    testValues.setup<T>(numInputs, bufferLen);

    for (size_t elem = 0; elem < bufferLen; ++elem)
    {
        testValues.inputs[0].as<T*>()[elem] = static_cast<T>(elem);
        testValues.inputs[1].as<T*>()[elem] = std::is_signed<T>::value ? static_cast<T>(elem * 2)
                                                                       : static_cast<T>(elem / 2);

        testValues.expectedOutputs.as<T*>()[elem] = testValues.inputs[0].as<T*>()[elem]
                                                  - testValues.inputs[1].as<T*>()[elem];
    }

    return testValues;
}

// Fully co-opt the scalar implementation since complex subtraction is just (real-real, imag-imag)
template <typename T>
static NPTests::EnableIfComplex<T, ArithmeticTestValues> getSubTestValues()
{
    using ScalarType = typename T::value_type;
    static const Pothos::DType dtype(typeid(T));

    auto testValues = getSubTestValues<ScalarType>();
    for (auto& input : testValues.inputs) input.dtype = dtype;
    testValues.expectedOutputs.dtype = dtype;

    return testValues;
}

template <typename T>
static NPTests::EnableIfNotComplex<T, ArithmeticTestValues> getMulTestValues()
{
    constexpr auto numInputs = 2;

    static const Pothos::DType dtype(typeid(T));

    ArithmeticTestValues testValues;
    testValues.setup<T>(numInputs, bufferLen);

    for (size_t elem = 0; elem < bufferLen; ++elem)
    {
        testValues.inputs[0].as<T*>()[elem] = static_cast<T>(elem);
        testValues.inputs[1].as<T*>()[elem] = static_cast<T>((elem % 2) + 1);

        if (std::is_signed<T>::value) testValues.inputs[1].as<T*>()[elem] *= -1;

        testValues.expectedOutputs.as<T*>()[elem] = testValues.inputs[0].as<T*>()[elem]
                                                  * testValues.inputs[1].as<T*>()[elem];
    }

    return testValues;
}

// Out of laziness, get the scalar version's values and recalculate the outputs.
template <typename T>
static NPTests::EnableIfComplex<T, ArithmeticTestValues> getMulTestValues()
{
    using ScalarType = typename T::value_type;
    static const Pothos::DType dtype(typeid(T));

    auto testValues = getMulTestValues<ScalarType>();
    POTHOS_TEST_EQUAL(2, testValues.inputs.size());

    for (auto& input : testValues.inputs) input.dtype = dtype;
    testValues.expectedOutputs.dtype = dtype;

    for (size_t elem = 0; elem < testValues.expectedOutputs.elements(); ++elem)
    {
        testValues.expectedOutputs.template as<T*>()[elem] =
            testValues.inputs[0].template as<const T*>()[elem] *
            testValues.inputs[1].template as<const T*>()[elem];
    }

    return testValues;
}

template <typename T>
static NPTests::EnableIfNotComplex<T, ArithmeticTestValues> getDivTestValues()
{
    constexpr auto numInputs = 2;

    static const Pothos::DType dtype(typeid(T));

    ArithmeticTestValues testValues;
    testValues.setup<T>(numInputs, bufferLen);

    for (size_t elem = 0; elem < bufferLen; ++elem)
    {
        testValues.inputs[0].as<T*>()[elem] = static_cast<T>(elem);
        testValues.inputs[1].as<T*>()[elem] = static_cast<T>((elem % 2) + 1);

        if (std::is_signed<T>::value) testValues.inputs[1].as<T*>()[elem] *= -1;

        testValues.expectedOutputs.as<T*>()[elem] = testValues.inputs[0].as<T*>()[elem]
                                                  / testValues.inputs[1].as<T*>()[elem];
    }

    return testValues;
}

// Out of laziness, get the scalar version's values and recalculate the outputs.
template <typename T>
static NPTests::EnableIfComplex<T, ArithmeticTestValues> getDivTestValues()
{
    using ScalarType = typename T::value_type;
    static const Pothos::DType dtype(typeid(T));

    auto testValues = getDivTestValues<ScalarType>();
    POTHOS_TEST_EQUAL(2, testValues.inputs.size());

    for (auto& input : testValues.inputs) input.dtype = dtype;
    testValues.expectedOutputs.dtype = dtype;

    for (size_t elem = 0; elem < testValues.expectedOutputs.elements(); ++elem)
    {
        testValues.expectedOutputs.template as<T*>()[elem] =
            testValues.inputs[0].template as<const T*>()[elem] /
            testValues.inputs[1].template as<const T*>()[elem];
    }

    return testValues;
}

template <typename T>
static NPTests::EnableIfNotComplex<T, ArithmeticTestValues> getFloorDivTestValues()
{
    constexpr auto numInputs = 2;

    static const Pothos::DType dtype(typeid(T));

    ArithmeticTestValues testValues;
    testValues.setup<T>(numInputs, bufferLen);

    for (size_t elem = 0; elem < bufferLen; ++elem)
    {
        testValues.inputs[0].as<T*>()[elem] = static_cast<T>(elem);
        testValues.inputs[1].as<T*>()[elem] = static_cast<T>((elem % 2) + 1);

        if (std::is_signed<T>::value) testValues.inputs[1].as<T*>()[elem] *= -1;

        testValues.expectedOutputs.as<T*>()[elem] = std::floor(testValues.inputs[0].as<T*>()[elem] /
                                                               testValues.inputs[1].as<T*>()[elem]);
    }

    return testValues;
}

template <typename T>
static NPTests::EnableIfNotComplex<T, ArithmeticTestValues> getRemainderTestValues()
{
    constexpr auto numInputs = 2;

    static const Pothos::DType dtype(typeid(T));

    ArithmeticTestValues testValues;
    testValues.setup<T>(numInputs, bufferLen);

    for (size_t elem = 0; elem < bufferLen; ++elem)
    {
        testValues.inputs[0].as<T*>()[elem] = static_cast<T>(elem);
        testValues.inputs[1].as<T*>()[elem] = T(5);

        testValues.expectedOutputs.as<T*>()[elem] = static_cast<unsigned long long>(std::floor(testValues.inputs[0].as<T*>()[elem]))
                                                  % static_cast<unsigned long long>(std::floor(testValues.inputs[1].as<T*>()[elem]));
    }

    return testValues;
}

template <typename T>
static NPTests::EnableIfNotComplex<T, ArithmeticTestValues> getFModTestValues()
{
    constexpr auto numInputs = 2;

    static const Pothos::DType dtype(typeid(T));

    ArithmeticTestValues testValues;
    testValues.setup<T>(numInputs, bufferLen);

    for (size_t elem = 0; elem < bufferLen; ++elem)
    {
        testValues.inputs[0].as<T*>()[elem] = static_cast<T>(elem);
        testValues.inputs[1].as<T*>()[elem] = T(5);
        if(std::is_signed<T>::value && (elem % 2)) testValues.inputs[1].as<T*>()[elem] *= -1;

        testValues.expectedOutputs.as<T*>()[elem] = static_cast<long long>(std::floor(testValues.inputs[0].as<T*>()[elem]))
                                                  % static_cast<long long>(std::floor(testValues.inputs[1].as<T*>()[elem]));
    }

    return testValues;
}

template <typename T>
static void testArithmeticBlock(
    const std::string& blockPath,
    bool useNumInputsParam,
    const ArithmeticTestValues& testValues)
{
    const Pothos::DType dtype(typeid(T));

    std::cout << " * Testing " << blockPath << "..." << std::endl;

    const auto numInputs = testValues.inputs.size();

    Pothos::Proxy arithmetic;
    if(useNumInputsParam)
    {
        arithmetic = Pothos::BlockRegistry::make(
                         blockPath,
                         dtype,
                         numInputs);
    }
    else
    {
        arithmetic = Pothos::BlockRegistry::make(
                         blockPath,
                         dtype);
    }

    std::vector<Pothos::Proxy> feeders(numInputs);
    for (size_t input = 0; input < numInputs; ++input)
    {
        feeders[input] = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
        feeders[input].call("feedBuffer", testValues.inputs[input]);
    }

    auto sink = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    {
        Pothos::Topology topology;

        for (size_t input = 0; input < numInputs; ++input)
        {
            topology.connect(feeders[input], 0, arithmetic, input);
        }
        topology.connect(arithmetic, 0, sink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    NPTests::testBufferChunk(
        testValues.expectedOutputs,
        sink.call<Pothos::BufferChunk>("getBuffer"));
}

template <typename T>
static void testArithmeticCommon()
{
    std::cout << "Testing " << Pothos::DType(typeid(T)).toString() << "..." << std::endl;

    testArithmeticBlock<T>("/numpy/add", true, getAddTestValues<T>());
    testArithmeticBlock<T>("/numpy/subtract", false, getSubTestValues<T>());
    testArithmeticBlock<T>("/numpy/multiply", true, getMulTestValues<T>());

    // NumPy has no divide for integral types.
    if(!std::is_integral<T>::value) testArithmeticBlock<T>("/numpy/divide", false, getDivTestValues<T>());
    if(!std::is_integral<T>::value) testArithmeticBlock<T>("/numpy/true_divide", false, getDivTestValues<T>());
}

template <typename T>
static NPTests::EnableIfNotComplex<T, void> testArithmetic()
{
    testArithmeticCommon<T>();

    if(!std::is_integral<T>::value) testArithmeticBlock<T>("/numpy/floor_divide", false, getFloorDivTestValues<T>());
    testArithmeticBlock<T>("/numpy/remainder", false, getRemainderTestValues<T>());
    testArithmeticBlock<T>("/numpy/fmod", false, getFModTestValues<T>());
}

template <typename T>
static NPTests::EnableIfComplex<T, void> testArithmetic()
{
    testArithmeticCommon<T>();
}

POTHOS_TEST_BLOCK("/numpy/tests", test_arithmetic_blocks)
{
    testArithmetic<std::int8_t>();
    testArithmetic<std::int16_t>();
    testArithmetic<std::int16_t>();
    testArithmetic<std::int32_t>();
    testArithmetic<std::int64_t>();
    testArithmetic<std::uint8_t>();
    testArithmetic<std::uint16_t>();
    testArithmetic<std::uint32_t>();
    testArithmetic<std::uint64_t>();
    testArithmetic<float>();
    testArithmetic<double>();
    testArithmetic<std::complex<float>>();
    testArithmetic<std::complex<double>>();
}
