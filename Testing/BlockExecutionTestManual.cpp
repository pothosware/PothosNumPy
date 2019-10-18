// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

/*

// This file was generated on 2019-10-18 16:20:53.193678.

#include "Testing/BlockExecutionTest.hpp"
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
#include <type_traits>
#include <unordered_map>

using uint = unsigned int;

template <typename T>
static void testAutoBlockExecutionFunc(
    const std::string& blockRegistryPath,
    bool hasNChans)
{
    static constexpr size_t nchans = 3;

    static const Pothos::DType dtype(typeid(T));
    std::cout << blockRegistryPath << "(" << dtype.toString() << ")" << std::endl;

    Pothos::Proxy testBlock;
    if(hasNChans)
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype,
                        nchans);
        POTHOS_TEST_EQUAL(
            nchans,
            testBlock.call<size_t>("getNumChannels"));
    }
    else
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype);
    }

    testBlockExecutionCommon<T>(testBlock);
}

template <typename BlockType
         ,typename Param0Type
         >
static void testAutoBlockExecutionFunc1Param(
    const std::string& blockRegistryPath
    ,const std::string& param0Name
    ,Param0Type param0Value1
    ,Param0Type param0Value2
    ,const std::vector<Param0Type>& param0InvalidValues
    )
{
    static const Pothos::DType dtype(typeid(BlockType));
    std::cout << blockRegistryPath << "(" << dtype.toString() << ")" << std::endl;

    const std::string getter0 = "get" + param0Name;
    const std::string setter0 = "set" + param0Name;

    auto testBlock = Pothos::BlockRegistry::make(
                         blockRegistryPath,
                         dtype
                         ,param0Value1
                         );
    testEqual(param0Value1, testBlock.template call<Param0Type>(getter0));

    testBlockExecutionCommon<BlockType>(testBlock);

    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
                        ,param0Value1
                        );
        testBlock.template call(setter0, param0Value2);
        testEqual(param0Value2, testBlock.template call<Param0Type>(getter0));
    }
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
                        ,param0Value1
                        );
        for(const Param0Type& invalidParam: param0InvalidValues)
        {
            POTHOS_TEST_THROWS(
                testBlock.template call(setter0, invalidParam),
                Pothos::ProxyExceptionMessage);
        }
    }
}
template <typename BlockType
         ,typename Param0Type
         ,typename Param1Type
         >
static void testAutoBlockExecutionFunc2Param(
    const std::string& blockRegistryPath
    ,const std::string& param0Name
    ,Param0Type param0Value1
    ,Param0Type param0Value2
    ,const std::vector<Param0Type>& param0InvalidValues
    ,const std::string& param1Name
    ,Param1Type param1Value1
    ,Param1Type param1Value2
    ,const std::vector<Param1Type>& param1InvalidValues
    )
{
    static const Pothos::DType dtype(typeid(BlockType));
    std::cout << blockRegistryPath << "(" << dtype.toString() << ")" << std::endl;

    const std::string getter0 = "get" + param0Name;
    const std::string setter0 = "set" + param0Name;
    const std::string getter1 = "get" + param1Name;
    const std::string setter1 = "set" + param1Name;

    auto testBlock = Pothos::BlockRegistry::make(
                         blockRegistryPath,
                         dtype
                         ,param0Value1
                         ,param1Value1
                         );
    testEqual(param0Value1, testBlock.template call<Param0Type>(getter0));
    testEqual(param1Value1, testBlock.template call<Param1Type>(getter1));

    testBlockExecutionCommon<BlockType>(testBlock);

    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
                        ,param0Value1
                        ,param1Value1
                        );
        testBlock.template call(setter0, param0Value2);
        testEqual(param0Value2, testBlock.template call<Param0Type>(getter0));
    }
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
                        ,param0Value1
                        ,param1Value1
                        );
        testBlock.template call(setter1, param1Value2);
        testEqual(param1Value2, testBlock.template call<Param1Type>(getter1));
    }
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
                        ,param0Value1
                        ,param1Value1
                        );
        for(const Param0Type& invalidParam: param0InvalidValues)
        {
            POTHOS_TEST_THROWS(
                testBlock.template call(setter0, invalidParam),
                Pothos::ProxyExceptionMessage);
        }
    }
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
                        ,param0Value1
                        ,param1Value1
                        );
        for(const Param1Type& invalidParam: param1InvalidValues)
        {
            POTHOS_TEST_THROWS(
                testBlock.template call(setter1, invalidParam),
                Pothos::ProxyExceptionMessage);
        }
    }
}
template <typename BlockType
         ,typename Param0Type
         ,typename Param1Type
         ,typename Param2Type
         >
static void testAutoBlockExecutionFunc3Param(
    const std::string& blockRegistryPath
    ,const std::string& param0Name
    ,Param0Type param0Value1
    ,Param0Type param0Value2
    ,const std::vector<Param0Type>& param0InvalidValues
    ,const std::string& param1Name
    ,Param1Type param1Value1
    ,Param1Type param1Value2
    ,const std::vector<Param1Type>& param1InvalidValues
    ,const std::string& param2Name
    ,Param2Type param2Value1
    ,Param2Type param2Value2
    ,const std::vector<Param2Type>& param2InvalidValues
    )
{
    static const Pothos::DType dtype(typeid(BlockType));
    std::cout << blockRegistryPath << "(" << dtype.toString() << ")" << std::endl;

    const std::string getter0 = "get" + param0Name;
    const std::string setter0 = "set" + param0Name;
    const std::string getter1 = "get" + param1Name;
    const std::string setter1 = "set" + param1Name;
    const std::string getter2 = "get" + param2Name;
    const std::string setter2 = "set" + param2Name;

    auto testBlock = Pothos::BlockRegistry::make(
                         blockRegistryPath,
                         dtype
                         ,param0Value1
                         ,param1Value1
                         ,param2Value1
                         );
    testEqual(param0Value1, testBlock.template call<Param0Type>(getter0));
    testEqual(param1Value1, testBlock.template call<Param1Type>(getter1));
    testEqual(param2Value1, testBlock.template call<Param2Type>(getter2));

    testBlockExecutionCommon<BlockType>(testBlock);

    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
                        ,param0Value1
                        ,param1Value1
                        ,param2Value1
                        );
        testBlock.template call(setter0, param0Value2);
        testEqual(param0Value2, testBlock.template call<Param0Type>(getter0));
    }
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
                        ,param0Value1
                        ,param1Value1
                        ,param2Value1
                        );
        testBlock.template call(setter1, param1Value2);
        testEqual(param1Value2, testBlock.template call<Param1Type>(getter1));
    }
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
                        ,param0Value1
                        ,param1Value1
                        ,param2Value1
                        );
        testBlock.template call(setter2, param2Value2);
        testEqual(param2Value2, testBlock.template call<Param2Type>(getter2));
    }
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
                        ,param0Value1
                        ,param1Value1
                        ,param2Value1
                        );
        for(const Param0Type& invalidParam: param0InvalidValues)
        {
            POTHOS_TEST_THROWS(
                testBlock.template call(setter0, invalidParam),
                Pothos::ProxyExceptionMessage);
        }
    }
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
                        ,param0Value1
                        ,param1Value1
                        ,param2Value1
                        );
        for(const Param1Type& invalidParam: param1InvalidValues)
        {
            POTHOS_TEST_THROWS(
                testBlock.template call(setter1, invalidParam),
                Pothos::ProxyExceptionMessage);
        }
    }
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
                        ,param0Value1
                        ,param1Value1
                        ,param2Value1
                        );
        for(const Param2Type& invalidParam: param2InvalidValues)
        {
            POTHOS_TEST_THROWS(
                testBlock.template call(setter2, invalidParam),
                Pothos::ProxyExceptionMessage);
        }
    }
}

//
// Test code
//

template <typename T>
static EnableIfInteger<T, void> testAutoBlockExecution()
{
    testAutoBlockExecutionFunc<T>("/numpy/random/permutation", false);
    testAutoBlockExecutionFunc<T>("/numpy/unique", false);
    testAutoBlockExecutionFunc<T>("/numpy/copysign", false);
    testAutoBlockExecutionFunc<T>("/numpy/positive", false);
    testAutoBlockExecutionFunc<T>("/numpy/flip", false);
    testAutoBlockExecutionFunc1Param<T, int>(
        "/numpy/roll"
        ,"Shift"
        ,10
        ,2
        ,std::vector<int>{}
        );
    testAutoBlockExecutionFunc1Param<T, int>(
        "/numpy/partition"
        ,"Index"
        ,4
        ,2
        ,std::vector<int>{-1}
        );
    testAutoBlockExecutionFunc<T>("/numpy/trim_zeros", false);
    testAutoBlockExecutionFunc<T>("/numpy/negative", false);
    testAutoBlockExecutionFunc<T>("/numpy/ravel", true);
    testAutoBlockExecutionFunc<T>("/numpy/sort", false);
    testAutoBlockExecutionFunc<T>("/numpy/ones", false);
    testAutoBlockExecutionFunc3Param<T, T, T, uint>(
        "/numpy/linspace"
        ,"Start"
        ,10
        ,2
        ,std::vector<T>{9}
        ,"Stop"
        ,4
        ,10
        ,std::vector<T>{11}
        ,"NumValues"
        ,3
        ,4
        ,std::vector<uint>{0, 5}
        );
    testAutoBlockExecutionFunc<T>("/numpy/zeros", false);
    testAutoBlockExecutionFunc1Param<T, T>(
        "/numpy/full"
        ,"FillValue"
        ,0
        ,1
        ,std::vector<T>{}
        );
    testAutoBlockExecutionFunc3Param<T, T, T, T>(
        "/numpy/arange"
        ,"Start"
        ,10
        ,20
        ,std::vector<T>{}
        ,"Stop"
        ,20
        ,10
        ,std::vector<T>{}
        ,"Step"
        ,1
        ,2
        ,std::vector<T>{}
        );
    testAutoBlockExecutionFunc<T>("/numpy/ptp", false);
    testAutoBlockExecutionFunc<T>("/numpy/mean", false);
    testAutoBlockExecutionFunc<T>("/numpy/std", false);
    testAutoBlockExecutionFunc<T>("/numpy/var", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanmean", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanstd", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanvar", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanmax", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanmin", false);
    testAutoBlockExecutionFunc<T>("/numpy/count_nonzero", false);
    testAutoBlockExecutionFunc<T>("/numpy/add", true);
    testAutoBlockExecutionFunc<T>("/numpy/subtract", false);
    testAutoBlockExecutionFunc<T>("/numpy/remainder", false);
    testAutoBlockExecutionFunc<T>("/numpy/fmod", false);
    testAutoBlockExecutionFunc<T>("/numpy/square", false);
    testAutoBlockExecutionFunc<T>("/numpy/absolute", false);
    testAutoBlockExecutionFunc<T>("/numpy/invert", false);
    testAutoBlockExecutionFunc<T>("/numpy/bitwise_and", false);
    testAutoBlockExecutionFunc<T>("/numpy/bitwise_or", false);
    testAutoBlockExecutionFunc<T>("/numpy/bitwise_xor", false);
}

template <typename T>
static EnableIfUnsignedInt<T, void> testAutoBlockExecution()
{
    testAutoBlockExecutionFunc<T>("/numpy/random/permutation", false);
    testAutoBlockExecutionFunc<T>("/numpy/unique", false);
    testAutoBlockExecutionFunc<T>("/numpy/flip", false);
    testAutoBlockExecutionFunc1Param<T, int>(
        "/numpy/roll"
        ,"Shift"
        ,10
        ,2
        ,std::vector<int>{}
        );
    testAutoBlockExecutionFunc1Param<T, int>(
        "/numpy/partition"
        ,"Index"
        ,4
        ,2
        ,std::vector<int>{-1}
        );
    testAutoBlockExecutionFunc<T>("/numpy/trim_zeros", false);
    testAutoBlockExecutionFunc<T>("/numpy/ravel", true);
    testAutoBlockExecutionFunc<T>("/numpy/sort", false);
    testAutoBlockExecutionFunc<T>("/numpy/ones", false);
    testAutoBlockExecutionFunc3Param<T, T, T, uint>(
        "/numpy/linspace"
        ,"Start"
        ,10
        ,2
        ,std::vector<T>{9}
        ,"Stop"
        ,4
        ,10
        ,std::vector<T>{11}
        ,"NumValues"
        ,3
        ,4
        ,std::vector<uint>{0, 5}
        );
    testAutoBlockExecutionFunc<T>("/numpy/zeros", false);
    testAutoBlockExecutionFunc1Param<T, T>(
        "/numpy/full"
        ,"FillValue"
        ,0
        ,1
        ,std::vector<T>{}
        );
    testAutoBlockExecutionFunc3Param<T, T, T, T>(
        "/numpy/arange"
        ,"Start"
        ,10
        ,20
        ,std::vector<T>{}
        ,"Stop"
        ,20
        ,10
        ,std::vector<T>{}
        ,"Step"
        ,1
        ,2
        ,std::vector<T>{}
        );
    testAutoBlockExecutionFunc<T>("/numpy/ptp", false);
    testAutoBlockExecutionFunc<T>("/numpy/mean", false);
    testAutoBlockExecutionFunc<T>("/numpy/std", false);
    testAutoBlockExecutionFunc<T>("/numpy/var", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanmean", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanstd", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanvar", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanmax", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanmin", false);
    testAutoBlockExecutionFunc<T>("/numpy/count_nonzero", false);
    testAutoBlockExecutionFunc<T>("/numpy/add", true);
    testAutoBlockExecutionFunc<T>("/numpy/subtract", false);
    testAutoBlockExecutionFunc<T>("/numpy/remainder", false);
    testAutoBlockExecutionFunc<T>("/numpy/fmod", false);
    testAutoBlockExecutionFunc<T>("/numpy/square", false);
    testAutoBlockExecutionFunc<T>("/numpy/invert", false);
    testAutoBlockExecutionFunc<T>("/numpy/bitwise_and", false);
    testAutoBlockExecutionFunc<T>("/numpy/bitwise_or", false);
    testAutoBlockExecutionFunc<T>("/numpy/bitwise_xor", false);
}

template <typename T>
static EnableIfFloat<T, void> testAutoBlockExecution()
{
    testAutoBlockExecutionFunc<T>("/numpy/random/permutation", false);
    testAutoBlockExecutionFunc2Param<T, float, float>(
        "/numpy/random/beta"
        ,"Alpha"
        ,2.0
        ,4.0
        ,std::vector<float>{0.0}
        ,"Beta"
        ,4.0
        ,2.0
        ,std::vector<float>{0.0}
        );
    testAutoBlockExecutionFunc2Param<T, uint, float>(
        "/numpy/random/binomial"
        ,"N"
        ,0
        ,1
        ,std::vector<uint>{}
        ,"P"
        ,0.25
        ,0.75
        ,std::vector<float>{-0.01, 1.01}
        );
    testAutoBlockExecutionFunc1Param<T, float>(
        "/numpy/random/chisquare"
        ,"DegreesOfFreedom"
        ,0.5
        ,1.0
        ,std::vector<float>{0.0}
        );
    testAutoBlockExecutionFunc1Param<T, float>(
        "/numpy/random/exponential"
        ,"Scale"
        ,0.5
        ,1.5
        ,std::vector<float>{0.0}
        );
    testAutoBlockExecutionFunc2Param<T, float, float>(
        "/numpy/random/f"
        ,"NumeratorDOF"
        ,0.5
        ,1.0
        ,std::vector<float>{0.0}
        ,"DenominatorDOF"
        ,0.5
        ,1.0
        ,std::vector<float>{0.0}
        );
    testAutoBlockExecutionFunc2Param<T, float, float>(
        "/numpy/random/gamma"
        ,"Shape"
        ,0.5
        ,1.0
        ,std::vector<float>{0.0}
        ,"Scale"
        ,0.5
        ,1.5
        ,std::vector<float>{0.0}
        );
    testAutoBlockExecutionFunc1Param<T, float>(
        "/numpy/random/geometric"
        ,"P"
        ,0.0
        ,0.5
        ,std::vector<float>{-0.01, 1.01}
        );
    testAutoBlockExecutionFunc2Param<T, float, float>(
        "/numpy/random/gumbel"
        ,"Location"
        ,-5.0
        ,5.0
        ,std::vector<float>{}
        ,"Scale"
        ,0.5
        ,1.5
        ,std::vector<float>{-0.01}
        );
    testAutoBlockExecutionFunc2Param<T, float, float>(
        "/numpy/random/laplace"
        ,"Location"
        ,-5.0
        ,5.0
        ,std::vector<float>{}
        ,"Scale"
        ,0.5
        ,1.5
        ,std::vector<float>{-0.01}
        );
    testAutoBlockExecutionFunc2Param<T, float, float>(
        "/numpy/random/logistic"
        ,"Location"
        ,-5.0
        ,5.0
        ,std::vector<float>{}
        ,"Scale"
        ,0.0
        ,1.5
        ,std::vector<float>{-0.01}
        );
    testAutoBlockExecutionFunc2Param<T, float, float>(
        "/numpy/random/lognormal"
        ,"Mean"
        ,-5.0
        ,5.0
        ,std::vector<float>{}
        ,"Sigma"
        ,0.0
        ,1.5
        ,std::vector<float>{-0.01}
        );
    testAutoBlockExecutionFunc1Param<T, float>(
        "/numpy/random/logseries"
        ,"P"
        ,0.25
        ,0.75
        ,std::vector<float>{0.0, 1.0}
        );
    testAutoBlockExecutionFunc2Param<T, float, float>(
        "/numpy/random/negative_binomial"
        ,"N"
        ,0.01
        ,1.0
        ,std::vector<float>{0.0}
        ,"P"
        ,0.25
        ,0.75
        ,std::vector<float>{-0.01, 1.01}
        );
    testAutoBlockExecutionFunc2Param<T, float, float>(
        "/numpy/random/noncentral_chisquare"
        ,"DegreesOfFreedom"
        ,0.01
        ,1.0
        ,std::vector<float>{0.0}
        ,"NonCentrality"
        ,0.25
        ,0.75
        ,std::vector<float>{-0.01}
        );
    testAutoBlockExecutionFunc3Param<T, float, float, float>(
        "/numpy/random/noncentral_f"
        ,"NumeratorDOF"
        ,1.5
        ,2.0
        ,std::vector<float>{1.0}
        ,"DenominatorDOF"
        ,0.5
        ,1.0
        ,std::vector<float>{0.0}
        ,"NonCentrality"
        ,0.25
        ,0.75
        ,std::vector<float>{-0.01}
        );
    testAutoBlockExecutionFunc2Param<T, float, float>(
        "/numpy/random/normal"
        ,"Location"
        ,-5.0
        ,5.0
        ,std::vector<float>{}
        ,"Scale"
        ,0.5
        ,1.5
        ,std::vector<float>{-0.01}
        );
    testAutoBlockExecutionFunc1Param<T, float>(
        "/numpy/random/pareto"
        ,"A"
        ,0.5
        ,1.5
        ,std::vector<float>{0.0}
        );
    testAutoBlockExecutionFunc1Param<T, float>(
        "/numpy/random/poisson"
        ,"Mu"
        ,0.0
        ,0.5
        ,std::vector<float>{-0.01}
        );
    testAutoBlockExecutionFunc1Param<T, float>(
        "/numpy/random/power"
        ,"A"
        ,1.0
        ,1.5
        ,std::vector<float>{0.99}
        );
    testAutoBlockExecutionFunc1Param<T, float>(
        "/numpy/random/rayleigh"
        ,"Scale"
        ,0.0
        ,1.5
        ,std::vector<float>{-0.01}
        );
    testAutoBlockExecutionFunc<T>("/numpy/random/standard_cauchy", false);
    testAutoBlockExecutionFunc1Param<T, float>(
        "/numpy/random/standard_t"
        ,"DegreesOfFreedom"
        ,0.5
        ,1.0
        ,std::vector<float>{0.0}
        );
    testAutoBlockExecutionFunc2Param<T, float, float>(
        "/numpy/random/vonmises"
        ,"Mu"
        ,-5.0
        ,5.0
        ,std::vector<float>{}
        ,"Kappa"
        ,0.5
        ,1.5
        ,std::vector<float>{-0.01}
        );
    testAutoBlockExecutionFunc2Param<T, float, float>(
        "/numpy/random/wald"
        ,"Mean"
        ,0.5
        ,1.5
        ,std::vector<float>{0.0}
        ,"Scale"
        ,0.5
        ,1.5
        ,std::vector<float>{0.0}
        );
    testAutoBlockExecutionFunc1Param<T, float>(
        "/numpy/random/weibull"
        ,"A"
        ,0.0
        ,0.5
        ,std::vector<float>{-0.01}
        );
    testAutoBlockExecutionFunc1Param<T, float>(
        "/numpy/random/zipf"
        ,"A"
        ,1.5
        ,2.0
        ,std::vector<float>{1.0}
        );
    testAutoBlockExecutionFunc<T>("/numpy/unique", false);
    testAutoBlockExecutionFunc<T>("/numpy/copysign", false);
    testAutoBlockExecutionFunc<T>("/numpy/positive", false);
    testAutoBlockExecutionFunc<T>("/numpy/flip", false);
    testAutoBlockExecutionFunc1Param<T, int>(
        "/numpy/roll"
        ,"Shift"
        ,10
        ,2
        ,std::vector<int>{}
        );
    testAutoBlockExecutionFunc1Param<T, int>(
        "/numpy/partition"
        ,"Index"
        ,4
        ,2
        ,std::vector<int>{-1}
        );
    testAutoBlockExecutionFunc<T>("/numpy/trim_zeros", false);
    testAutoBlockExecutionFunc<T>("/numpy/negative", false);
    testAutoBlockExecutionFunc<T>("/numpy/ravel", true);
    testAutoBlockExecutionFunc<T>("/numpy/sort", false);
    testAutoBlockExecutionFunc<T>("/numpy/ones", false);
    testAutoBlockExecutionFunc3Param<T, T, T, uint>(
        "/numpy/linspace"
        ,"Start"
        ,10
        ,2
        ,std::vector<T>{}
        ,"Stop"
        ,4
        ,10
        ,std::vector<T>{}
        ,"NumValues"
        ,3
        ,4
        ,std::vector<uint>{}
        );
    testAutoBlockExecutionFunc<T>("/numpy/zeros", false);
    testAutoBlockExecutionFunc1Param<T, T>(
        "/numpy/full"
        ,"FillValue"
        ,0
        ,1
        ,std::vector<T>{}
        );
    testAutoBlockExecutionFunc3Param<T, T, T, T>(
        "/numpy/arange"
        ,"Start"
        ,10
        ,20
        ,std::vector<T>{}
        ,"Stop"
        ,20
        ,10
        ,std::vector<T>{}
        ,"Step"
        ,1
        ,2
        ,std::vector<T>{}
        );
    testAutoBlockExecutionFunc3Param<T, T, T, uint>(
        "/numpy/logspace"
        ,"Start"
        ,10
        ,2
        ,std::vector<T>{}
        ,"Stop"
        ,4
        ,10
        ,std::vector<T>{}
        ,"NumValues"
        ,3
        ,4
        ,std::vector<uint>{0}
        );
    testAutoBlockExecutionFunc3Param<T, T, T, uint>(
        "/numpy/geomspace"
        ,"Start"
        ,10
        ,2
        ,std::vector<T>{}
        ,"Stop"
        ,4
        ,10
        ,std::vector<T>{}
        ,"NumValues"
        ,3
        ,4
        ,std::vector<uint>{0}
        );
    testAutoBlockExecutionFunc<T>("/numpy/sin", false);
    testAutoBlockExecutionFunc<T>("/numpy/cos", false);
    testAutoBlockExecutionFunc<T>("/numpy/tan", false);
    testAutoBlockExecutionFunc<T>("/numpy/arcsin", false);
    testAutoBlockExecutionFunc<T>("/numpy/arccos", false);
    testAutoBlockExecutionFunc<T>("/numpy/arctan", false);
    testAutoBlockExecutionFunc<T>("/numpy/sinh", false);
    testAutoBlockExecutionFunc<T>("/numpy/cosh", false);
    testAutoBlockExecutionFunc<T>("/numpy/tanh", false);
    testAutoBlockExecutionFunc<T>("/numpy/arcsinh", false);
    testAutoBlockExecutionFunc<T>("/numpy/arccosh", false);
    testAutoBlockExecutionFunc<T>("/numpy/arctanh", false);
    testAutoBlockExecutionFunc<T>("/numpy/deg2rad", false);
    testAutoBlockExecutionFunc<T>("/numpy/rad2deg", false);
    testAutoBlockExecutionFunc<T>("/numpy/ptp", false);
    testAutoBlockExecutionFunc<T>("/numpy/mean", false);
    testAutoBlockExecutionFunc<T>("/numpy/std", false);
    testAutoBlockExecutionFunc<T>("/numpy/var", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanmean", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanstd", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanvar", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanmax", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanmin", false);
    testAutoBlockExecutionFunc<T>("/numpy/count_nonzero", false);
    testAutoBlockExecutionFunc<T>("/numpy/exp", false);
    testAutoBlockExecutionFunc<T>("/numpy/expm1", false);
    testAutoBlockExecutionFunc<T>("/numpy/exp2", false);
    testAutoBlockExecutionFunc<T>("/numpy/log", false);
    testAutoBlockExecutionFunc<T>("/numpy/log10", false);
    testAutoBlockExecutionFunc<T>("/numpy/log2", false);
    testAutoBlockExecutionFunc<T>("/numpy/log1p", false);
    testAutoBlockExecutionFunc<T>("/numpy/logaddexp", false);
    testAutoBlockExecutionFunc<T>("/numpy/logaddexp2", false);
    testAutoBlockExecutionFunc<T>("/numpy/add", true);
    testAutoBlockExecutionFunc<T>("/numpy/subtract", false);
    testAutoBlockExecutionFunc<T>("/numpy/reciprocal", false);
    testAutoBlockExecutionFunc<T>("/numpy/multiply", true);
    testAutoBlockExecutionFunc<T>("/numpy/divide", false);
    testAutoBlockExecutionFunc<T>("/numpy/true_divide", false);
    testAutoBlockExecutionFunc<T>("/numpy/floor_divide", false);
    testAutoBlockExecutionFunc<T>("/numpy/remainder", false);
    testAutoBlockExecutionFunc<T>("/numpy/fmod", false);
    testAutoBlockExecutionFunc<T>("/numpy/sqrt", false);
    testAutoBlockExecutionFunc<T>("/numpy/cbrt", false);
    testAutoBlockExecutionFunc<T>("/numpy/square", false);
    testAutoBlockExecutionFunc<T>("/numpy/absolute", false);
    testAutoBlockExecutionFunc<T>("/numpy/fabs", false);
    testAutoBlockExecutionFunc<T>("/numpy/rint", false);
    testAutoBlockExecutionFunc<T>("/numpy/fix", false);
    testAutoBlockExecutionFunc<T>("/numpy/ceil", false);
    testAutoBlockExecutionFunc<T>("/numpy/floor", false);
    testAutoBlockExecutionFunc<T>("/numpy/trunc", false);
    testAutoBlockExecutionFunc<T>("/numpy/i0", false);
    testAutoBlockExecutionFunc<T>("/numpy/sinc", false);
}

template <typename T>
static EnableIfComplex<T, void> testAutoBlockExecution()
{
    testAutoBlockExecutionFunc<T>("/numpy/random/permutation", false);
    testAutoBlockExecutionFunc<T>("/numpy/unique", false);
    testAutoBlockExecutionFunc<T>("/numpy/positive", false);
    testAutoBlockExecutionFunc<T>("/numpy/flip", false);
    testAutoBlockExecutionFunc1Param<T, int>(
        "/numpy/roll"
        ,"Shift"
        ,10
        ,2
        ,std::vector<int>{}
        );
    testAutoBlockExecutionFunc1Param<T, int>(
        "/numpy/partition"
        ,"Index"
        ,4
        ,2
        ,std::vector<int>{-1}
        );
    testAutoBlockExecutionFunc<T>("/numpy/trim_zeros", false);
    testAutoBlockExecutionFunc<T>("/numpy/negative", false);
    testAutoBlockExecutionFunc<T>("/numpy/ravel", true);
    testAutoBlockExecutionFunc<T>("/numpy/sort", false);
    testAutoBlockExecutionFunc<T>("/numpy/sort_complex", false);
    testAutoBlockExecutionFunc<T>("/numpy/ones", false);
    testAutoBlockExecutionFunc<T>("/numpy/zeros", false);
    testAutoBlockExecutionFunc1Param<T, T>(
        "/numpy/full"
        ,"FillValue"
        ,0
        ,1
        ,std::vector<T>{}
        );
    testAutoBlockExecutionFunc<T>("/numpy/ptp", false);
    testAutoBlockExecutionFunc<T>("/numpy/mean", false);
    testAutoBlockExecutionFunc<T>("/numpy/std", false);
    testAutoBlockExecutionFunc<T>("/numpy/var", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanmean", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanstd", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanvar", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanmax", false);
    testAutoBlockExecutionFunc<T>("/numpy/nanmin", false);
    testAutoBlockExecutionFunc<T>("/numpy/count_nonzero", false);
    testAutoBlockExecutionFunc<T>("/numpy/exp", false);
    testAutoBlockExecutionFunc<T>("/numpy/expm1", false);
    testAutoBlockExecutionFunc<T>("/numpy/exp2", false);
    testAutoBlockExecutionFunc<T>("/numpy/log", false);
    testAutoBlockExecutionFunc<T>("/numpy/log10", false);
    testAutoBlockExecutionFunc<T>("/numpy/log2", false);
    testAutoBlockExecutionFunc<T>("/numpy/log1p", false);
    testAutoBlockExecutionFunc<T>("/numpy/add", true);
    testAutoBlockExecutionFunc<T>("/numpy/subtract", false);
    testAutoBlockExecutionFunc<T>("/numpy/reciprocal", false);
    testAutoBlockExecutionFunc<T>("/numpy/multiply", true);
    testAutoBlockExecutionFunc<T>("/numpy/divide", false);
    testAutoBlockExecutionFunc<T>("/numpy/true_divide", false);
    testAutoBlockExecutionFunc<T>("/numpy/floor_divide", false);
    testAutoBlockExecutionFunc<T>("/numpy/sqrt", false);
    testAutoBlockExecutionFunc<T>("/numpy/square", false);
    testAutoBlockExecutionFunc<T>("/numpy/rint", false);
    testAutoBlockExecutionFunc<T>("/numpy/conjugate", false);
}

POTHOS_TEST_BLOCK("/numpy/tests", test_block_execution)
{
    // SFINAE will make these call the functions with the
    // applicable blocks.
    testAutoBlockExecution<std::int8_t>();
    testAutoBlockExecution<std::int16_t>();
    testAutoBlockExecution<std::int32_t>();
    testAutoBlockExecution<std::int64_t>();
    testAutoBlockExecution<std::uint8_t>();
    testAutoBlockExecution<std::uint16_t>();
    testAutoBlockExecution<std::uint32_t>();
    testAutoBlockExecution<std::uint64_t>();
    testAutoBlockExecution<float>();
    testAutoBlockExecution<double>();
    testAutoBlockExecution<std::complex<float>>();
    testAutoBlockExecution<std::complex<double>>();
}
*/
