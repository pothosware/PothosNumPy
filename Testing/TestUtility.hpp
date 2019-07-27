// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INCLUDED_TEST_UTILITY_HPP
#define INCLUDED_TEST_UTILITY_HPP

#include <Pothos/Framework/BufferChunk.hpp>
#include <Pothos/Framework/DType.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Testing.hpp>

#include <algorithm>
#include <complex>
#include <cstring>
#include <type_traits>
#include <vector>

//
// Useful typedefs
//

template <typename T>
struct IsComplex : std::false_type {};

template <typename T>
struct IsComplex<std::complex<T>> : std::true_type {};

template <typename T, typename U>
using EnableIfInteger = typename std::enable_if<!IsComplex<T>::value && !std::is_floating_point<T>::value, U>::type;

template <typename T, typename U>
using EnableIfFloat = typename std::enable_if<!IsComplex<T>::value && std::is_floating_point<T>::value, U>::type;

template <typename T, typename U>
using EnableIfComplex = typename std::enable_if<IsComplex<T>::value, U>::type;

template <typename T, typename U>
using EnableIfNotComplex = typename std::enable_if<!IsComplex<T>::value, U>::type;

//
// Utility functions
//

template <typename T>
static inline EnableIfInteger<T, void> testEqual(T x, T y)
{
    POTHOS_TEST_EQUAL(x, y);
}

template <typename T>
static inline EnableIfFloat<T, void> testEqual(T x, T y)
{
    POTHOS_TEST_CLOSE(x, y, 1e-6);
}

template <typename T>
static inline EnableIfComplex<T, void> testEqual(T x, T y)
{
    testEqual(x.real(), y.real());
    testEqual(x.imag(), y.imag());
}

template <typename T>
static Pothos::BufferChunk stdVectorToBufferChunk(
    const Pothos::DType& dtype,
    const std::vector<T>& vectorIn)
{
    auto ret = Pothos::BufferChunk(
                   dtype,
                   (vectorIn.size() / dtype.dimension()));
    auto buf = ret.as<T*>();
    for(size_t i = 0; i < vectorIn.size(); ++i)
    {
        buf[i] = vectorIn[i];
    }

    return ret;
}

template <typename In, typename Out>
static std::vector<Out> staticCastVector(const std::vector<In>& vectorIn)
{
    std::vector<Out> vectorOut;
    vectorOut.reserve(vectorIn.size());
    std::transform(
        vectorIn.begin(),
        vectorIn.end(),
        std::back_inserter(vectorOut),
        [](In val){return static_cast<Out>(val);});

    return vectorOut;
}

template <typename In, typename Out>
static std::vector<Out> reinterpretCastVector(const std::vector<In>& vectorIn)
{
    static_assert(sizeof(In) == sizeof(Out));

    std::vector<Out> vectorOut(vectorIn.size());
    std::memcpy(
        vectorOut.data(),
        vectorIn.data(),
        vectorIn.size() * sizeof(In));

    return vectorOut;
}

// Assumption: vectorIn is an even size
template <typename T>
static std::vector<std::complex<T>> toComplexVector(const std::vector<T>& vectorIn)
{
    std::vector<std::complex<T>> vectorOut(vectorIn.size() / 2);
    std::memcpy(
        vectorOut.data(),
        vectorIn.data(),
        vectorIn.size() * sizeof(T));

    return vectorOut;
}

// https://gist.github.com/lorenzoriano/5414671
template <typename T>
static std::vector<T> linspace(T a, T b, size_t N) {
    T h = (b - a) / static_cast<T>(N-1);
    std::vector<T> xs(N);
    typename std::vector<T>::iterator x;
    T val;
    for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h)
        *x = val;
    return xs;
}

template <typename T>
static typename std::enable_if<std::is_floating_point<T>::value, void>::type testBufferChunk(
    const Pothos::BufferChunk& bufferChunk,
    const std::vector<T>& expectedOutputs,
    T epsilon = 1e-6)
{
    auto pOut = bufferChunk.as<const T*>();
    for (size_t i = 0; i < bufferChunk.elements(); i++)
    {
        POTHOS_TEST_CLOSE(
            expectedOutputs[i],
            pOut[i],
            epsilon);
    }
}

template <typename T>
static typename std::enable_if<std::is_integral<T>::value, void>::type testBufferChunk(
    const Pothos::BufferChunk& bufferChunk,
    const std::vector<T>& expectedOutputs)
{
    auto pOut = bufferChunk.as<const T*>();
    for (size_t i = 0; i < bufferChunk.elements(); i++)
    {
        POTHOS_TEST_EQUAL(
            expectedOutputs[i],
            pOut[i]);
    }
}

template <typename ReturnType, typename... ArgsType>
ReturnType getAndCallPlugin(
    const std::string& proxyPath,
    ArgsType&&... args)
{
    auto plugin = Pothos::PluginRegistry::get(proxyPath);
    auto getter = plugin.getObject().extract<Pothos::Callable>();

    return getter.call<ReturnType>(args...);
}

#endif /* INCLUDED_TEST_UTILITY_HPP */
