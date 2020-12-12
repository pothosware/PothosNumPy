// Copyright (c) 2019-2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>

#include <Poco/Random.h>

#include <random>

namespace NPTests
{

// This is random enough for our use case.
template <typename T>
static EnableIfAnyInt<T, std::vector<T>> getRandomInputs(size_t numElements)
{
    static std::random_device rd;
    static std::mt19937 g(rd());
    static std::uniform_int_distribution<T> dist(
               std::numeric_limits<T>::min(),
               std::numeric_limits<T>::max());

    std::vector<T> randomInputs;
    for(size_t i = 0; i < numElements; ++i)
    {
        randomInputs.emplace_back(dist(g));
    }

    return randomInputs;
}

template <typename T>
static EnableIfFloat<T, std::vector<T>> getRandomInputs(size_t numElements)
{
    static std::random_device rd;
    static std::mt19937 g(rd());
    static std::uniform_real_distribution<T> dist(
               std::numeric_limits<T>::min(),
               std::numeric_limits<T>::max());

    std::vector<T> randomInputs;
    for(size_t i = 0; i < numElements; ++i)
    {
        randomInputs.emplace_back(dist(g));
    }

    return randomInputs;
}

template <typename T>
static EnableIfComplex<T, std::vector<T>> getRandomInputs(size_t numElements)
{
    using Scalar = typename T::value_type;

    return toComplexVector(getRandomInputs<Scalar>(numElements * 2));
}

Pothos::BufferChunk getRandomInputs(
    const std::string& type,
    size_t numElements)
{
    #define IfTypeGetRandomInputs(typeStr, ctype) \
        if(type == typeStr) \
            return stdVectorToBufferChunk<ctype>( \
                       getRandomInputs<ctype>(numElements));

    IfTypeGetRandomInputs("int8", std::int8_t)
    IfTypeGetRandomInputs("int16", std::int16_t)
    IfTypeGetRandomInputs("int32", std::int32_t)
    IfTypeGetRandomInputs("int64", std::int64_t)
    IfTypeGetRandomInputs("uint8", std::uint8_t)
    IfTypeGetRandomInputs("uint16", std::uint16_t)
    IfTypeGetRandomInputs("uint32", std::uint32_t)
    IfTypeGetRandomInputs("uint64", std::uint64_t)
    IfTypeGetRandomInputs("float32", float)
    IfTypeGetRandomInputs("float64", double)
    IfTypeGetRandomInputs("complex_float32", std::complex<float>)
    IfTypeGetRandomInputs("complex_float64", std::complex<double>)

    // Should never get here
    return Pothos::BufferChunk();
}

template <typename T>
static void getNonUniqueRandomTestInputs(
    Pothos::BufferChunk* pInput,
    size_t bufferLen)
{
    const auto dtype = Pothos::DType(typeid(T));

    auto inputVector = bufferChunkToStdVector<T>(getRandomInputs(dtype.name(), bufferLen));
    const auto originalSize = inputVector.size();

    // Duplicate some values.
    const auto numDuplicates = inputVector.size() / 10;
    constexpr size_t maxNumRepeats = 10;
    for(size_t dup = 0; dup < numDuplicates; ++dup)
    {
        const auto index = Poco::Random().next(Poco::UInt32(originalSize));
        const auto repeatCount = Poco::Random().next(maxNumRepeats)+1;

        for(size_t rep = 0; rep < repeatCount; ++rep)
        {
            inputVector.emplace_back(inputVector[index]);
        }
    }

    (*pInput) = stdVectorToBufferChunk(inputVector);
}

template <typename T>
static void getNonUniqueRandomTestInputs(
    std::vector<Pothos::BufferChunk>* pInputs,
    size_t numInputs,
    size_t bufferLen)
{
    const auto dtype = Pothos::DType(typeid(T));

    std::vector<std::vector<T>> inputVectors;
    for(size_t input = 0; input < numInputs; ++input)
    {
        inputVectors.emplace_back(bufferChunkToStdVector<T>(getRandomInputs(dtype.name(), bufferLen)));
    }

    // Duplicate some values.
    const auto numDuplicates = bufferLen / 10;
    constexpr size_t maxNumRepeats = 10;
    for(size_t srcInput = 0; srcInput < numInputs; ++srcInput)
    {
        for(size_t dstInput = 0; dstInput < numInputs; ++dstInput)
        {
            if(srcInput == dstInput) continue;

            for(size_t dup = 0; dup < numDuplicates; ++dup)
            {
                const auto index = Poco::Random().next(Poco::UInt32(bufferLen));
                const auto repeatCount = Poco::Random().next(maxNumRepeats)+1;

                for(size_t rep = 0; rep < repeatCount; ++rep)
                {
                    inputVectors[dstInput][index] = inputVectors[srcInput][index];
                }
            }
        }
    }

    pInputs->clear();
    std::transform(
        inputVectors.begin(),
        inputVectors.end(),
        std::back_inserter(*pInputs),
        stdVectorToBufferChunk<T>);
}

void getNonUniqueRandomTestInputs(
    const std::string& type,
    size_t numElements,
    Pothos::BufferChunk* pNonUniqueInputs)
{
    #define IfTypeThenGetTestInputs(typeStr, cType) \
        if(type == typeStr) \
        { \
            getNonUniqueRandomTestInputs<cType>( \
                pNonUniqueInputs, \
                numElements); \
        }

    IfTypeThenGetTestInputs("int8", std::int8_t)
    IfTypeThenGetTestInputs("int16", std::int16_t)
    IfTypeThenGetTestInputs("int32", std::int32_t)
    IfTypeThenGetTestInputs("int64", std::int64_t)
    IfTypeThenGetTestInputs("uint8", std::uint8_t)
    IfTypeThenGetTestInputs("uint16", std::uint16_t)
    IfTypeThenGetTestInputs("uint32", std::uint32_t)
    IfTypeThenGetTestInputs("uint64", std::uint64_t)
    IfTypeThenGetTestInputs("float32", float)
    IfTypeThenGetTestInputs("float64", double)

    #undef IfTypeThenGetTestInputs
}


void getNonUniqueRandomTestInputs(
    const std::string& type,
    size_t numInputs,
    size_t numElements,
    std::vector<Pothos::BufferChunk>* pNonUniqueInputs)
{
    #define IfTypeThenGetTestInputs(typeStr, cType) \
        if(type == typeStr) \
        { \
            getNonUniqueRandomTestInputs<cType>( \
                pNonUniqueInputs, \
                numInputs, \
                numElements); \
        }

    IfTypeThenGetTestInputs("int8", std::int8_t)
    IfTypeThenGetTestInputs("int16", std::int16_t)
    IfTypeThenGetTestInputs("int32", std::int32_t)
    IfTypeThenGetTestInputs("int64", std::int64_t)
    IfTypeThenGetTestInputs("uint8", std::uint8_t)
    IfTypeThenGetTestInputs("uint16", std::uint16_t)
    IfTypeThenGetTestInputs("uint32", std::uint32_t)
    IfTypeThenGetTestInputs("uint64", std::uint64_t)
    IfTypeThenGetTestInputs("float32", float)
    IfTypeThenGetTestInputs("float64", double)
}

template <typename T>
static constexpr EnableIfFloat<T,T> epsilon() {return T(1e-4);}

void testBufferChunk(
    const Pothos::BufferChunk& expectedBufferChunk,
    const Pothos::BufferChunk& actualBufferChunk)
{
    POTHOS_TEST_EQUAL(
        expectedBufferChunk.dtype.name(),
        actualBufferChunk.dtype.name());
    POTHOS_TEST_EQUAL(
        expectedBufferChunk.elements(),
        actualBufferChunk.elements());

    #define IfTypeThenCompare(typeStr, cType) \
        if(expectedBufferChunk.dtype.name() == typeStr) \
        { \
            POTHOS_TEST_EQUALA( \
                expectedBufferChunk.as<const cType*>(), \
                actualBufferChunk.as<const cType*>(), \
                expectedBufferChunk.elements()); \
            return; \
        }
    #define IfTypeThenCompareFloat(typeStr, cType) \
        if(expectedBufferChunk.dtype.name() == typeStr) \
        { \
            POTHOS_TEST_CLOSEA( \
                expectedBufferChunk.as<const cType*>(), \
                actualBufferChunk.as<const cType*>(), \
                epsilon<cType>(), \
                expectedBufferChunk.elements()); \
            return; \
        }
    #define IfTypeThenCompareComplex(typeStr, cType) \
        if(expectedBufferChunk.dtype.name() == typeStr) \
        { \
            for(size_t i = 0; i < expectedBufferChunk.elements(); ++i) \
            { \
                POTHOS_TEST_CLOSE( \
                    expectedBufferChunk.as<const cType*>()[i].real(), \
                    actualBufferChunk.as<const cType*>()[i].real(), \
                    epsilon<typename cType::value_type>()); \
                POTHOS_TEST_CLOSE( \
                    expectedBufferChunk.as<const cType*>()[i].imag(), \
                    actualBufferChunk.as<const cType*>()[i].imag(), \
                    epsilon<typename cType::value_type>()); \
            } \
            return; \
        }

    IfTypeThenCompare("int8", std::int8_t)
    IfTypeThenCompare("int16", std::int16_t)
    IfTypeThenCompare("int32", std::int32_t)
    IfTypeThenCompare("int64", std::int64_t)
    IfTypeThenCompare("uint8", std::uint8_t)
    IfTypeThenCompare("uint16", std::uint16_t)
    IfTypeThenCompare("uint32", std::uint32_t)
    IfTypeThenCompare("uint64", std::uint64_t)
    IfTypeThenCompareFloat("float32", float)
    IfTypeThenCompareFloat("float64", double)
    IfTypeThenCompareComplex("complex_float32", std::complex<float>)
    IfTypeThenCompareComplex("complex_float64", std::complex<double>)
}

}
