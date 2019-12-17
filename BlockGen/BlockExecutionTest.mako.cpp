// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

// This file was generated on ${Now}.

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

using str = std::string;
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

    // Allow a longer timeout for random blocks.
    const bool longTimeout = (std::string::npos != blockRegistryPath.find("random"));

    testBlockExecutionCommon(testBlock, longTimeout);
}

%for numParams in range(1,maxNumParams+1):
template <typename BlockType
%for paramNum in range(numParams):
         ,typename Param${paramNum}Type
%endfor
         >
static void testAutoBlockExecutionFunc${numParams}Param(
    const std::string& blockRegistryPath
%for paramNum in range(numParams):
    ,const std::string& param${paramNum}Name
    ,Param${paramNum}Type param${paramNum}Value1
    ,Param${paramNum}Type param${paramNum}Value2
    ,const std::vector<Param${paramNum}Type>& param${paramNum}InvalidValues
%endfor
    )
{
    static const Pothos::DType dtype(typeid(BlockType));
    std::cout << blockRegistryPath << "(" << dtype.toString() << ")" << std::endl;

%for paramNum in range(numParams):
    const std::string getter${paramNum} = "get" + param${paramNum}Name;
    const std::string setter${paramNum} = "set" + param${paramNum}Name;
%endfor

    auto testBlock = Pothos::BlockRegistry::make(
                         blockRegistryPath,
                         dtype
%for paramNum in range(numParams):
                         ,param${paramNum}Value1
%endfor
                         );
%for paramNum in range(numParams):
    testEqual(param${paramNum}Value1, testBlock.template call<Param${paramNum}Type>(getter${paramNum}));
%endfor

    testBlockExecutionCommon(testBlock);

%for setParamNum in range(numParams):
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
%for paramNum in range(numParams):
                        ,param${paramNum}Value1
%endfor
                        );
        testBlock.template call(setter${setParamNum}, param${setParamNum}Value2);
        testEqual(param${setParamNum}Value2, testBlock.template call<Param${setParamNum}Type>(getter${setParamNum}));
    }
%endfor
%for badParamNum in range(numParams):
    {
        testBlock = Pothos::BlockRegistry::make(
                        blockRegistryPath,
                        dtype
%for paramNum in range(numParams):
                        ,param${paramNum}Value1
%endfor
                        );
        for(const Param${badParamNum}Type& invalidParam: param${badParamNum}InvalidValues)
        {
            POTHOS_TEST_THROWS(
                testBlock.template call(setter${badParamNum}, invalidParam),
                Pothos::ProxyExceptionMessage);
        }
    }
%endfor
}
%endfor

//
// Test code
//
%for typedefName,typeName in sfinaeMap.items():

template <typename T>
static EnableIf${typedefName}<T, void> testAutoBlockExecution()
{
%for blockName,blockInfo in blockYAML.items():
    %if (not blockInfo.get("skipExecTest", False) and blockInfo.get("subclass", False) and blockInfo["class"] not in ["FixedSingleOutputSource"]):
        %if "blockType" in blockInfo:
            %if (typeName in blockInfo["blockType"]) or ("all" in blockInfo["blockType"]):
    testAutoBlockExecutionFunc${len(blockInfo["funcArgs"])}Param<T, ${", ".join([("T" if param["dtype"] == "blockType" else param["dtype"]) for param in blockInfo["funcArgs"]])}>(
        "/numpy/${blockName}"
                %for funcArg in blockInfo["funcArgs"]:
        ,"${funcArg["name"][0].upper() + funcArg["name"][1:]}"
                    %if ("testValue1" in funcArg) and ("testValue2" in funcArg):
        ,${funcArg["testValue1"]}
        ,${funcArg["testValue2"]}
                    %else:
        ,${funcArg["validValues"][0]}
        ,${funcArg["validValues"][1]}
                    %endif
        ,std::vector<${"T" if funcArg["dtype"] == "blockType" else funcArg["dtype"]}>{${", ".join([str(x) for x in funcArg.get(badParamsMap[typedefName], funcArg.get("badValues", []))])}}
                %endfor
        );
            %endif
        %endif
    %elif (not blockInfo.get("skipExecTest", False) and not blockInfo.get("subclass", False) and blockInfo["class"] not in ["ForwardAndPostLabelBlock"]):
        %if "blockType" in blockInfo:
            %if (typeName in blockInfo["blockType"]) or ("all" in blockInfo["blockType"]):
    testAutoBlockExecutionFunc<T>("/numpy/${blockName}", ${"true" if blockInfo["class"] == "NToOneBlock" else "false"});
            %endif
        %endif
    %endif
%endfor

    testManualBlockExecution<T>();
}
%endfor

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
