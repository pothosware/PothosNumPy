// Copyright (c) 2019-2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>

namespace NPTests
{

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
            for(size_t i = 0; i < expectedBufferChunk.elements(); ++i) \
            { \
                POTHOS_TEST_CLOSE( \
                    expectedBufferChunk.as<const cType*>()[i], \
                    actualBufferChunk.as<const cType*>()[i], \
                    epsilon<cType>()); \
            } \
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
