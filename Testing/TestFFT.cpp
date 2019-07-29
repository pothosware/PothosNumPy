// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/Thread.h>

#include <complex>
#include <iostream>
#include <string>

//
// Parameters
//

template <typename T, typename U>
struct BlockTestParams
{
    std::string fwdBlockRegistryPath;
    std::string invBlockRegistryPath;
    std::vector<T> inputs;
    std::vector<U> outputs;
};

//
// Utility code
//

template <typename T, typename U>
static EnableIfBothComplex<T, U, BlockTestParams<T, U>> getTestParams(
    const std::string& fwdBlockRegistryPath,
    const std::string& invBlockRegistryPath)
{
    return
    {
        fwdBlockRegistryPath,
        invBlockRegistryPath,
        {
            // Primes
            {2.0f, 3.0f},
            {5.0f, 7.0f},
            {11.0f, 13.0f},
            {17.0f, 19.0f},
            {23.0f, 29.0f},
            {31.0f, 37.0f},
            {41.0f, 43.0f},
            {47.0f, 53.0f},
            {59.0f, 61.0f},
            {67.0f, 71.0f},
            {73.0f, 79.0f},
            {83.0f, 89.0f},
            {97.0f, 101.0f},
            {103.0f, 107.0f},
            {109.0f, 113.0f},
            {127.0f, 131.0f},
            {137.0f, 139.0f},
            {149.0f, 151.0f},
            {157.0f, 163.0f},
            {167.0f, 173.0f},
            {179.0f, 181.0f},
            {191.0f, 193.0f},
            {197.0f, 199.0f},
            {211.0f, 223.0f},
            {227.0f, 229.0f},
            {233.0f, 239.0f},
            {241.0f, 251.0f},
            {257.0f, 263.0f},
            {269.0f, 271.0f},
            {277.0f, 281.0f},
            {283.0f, 293.0f},
            {307.0f, 311.0f}
        },
        {
            // Expected outputs after processing primes
            {4377.0f, 4516.0f},
            {-1706.1268310546875f, 1638.4256591796875f},
            {-915.2083740234375f, 660.69427490234375f},
            {-660.370361328125f, 381.59600830078125f},
            {-499.96044921875f, 238.41630554199219f},
            {-462.26748657226562f, 152.88948059082031f},
            {-377.98440551757812f, 77.5928955078125f},
            {-346.85821533203125f, 47.152004241943359f},
            {-295.0f, 20.0f},
            {-286.33609008789062f, -22.257017135620117f},
            {-271.52999877929688f, -33.081821441650391f},
            {-224.6358642578125f, -67.019538879394531f},
            {-244.24473571777344f, -91.524826049804688f},
            {-203.09068298339844f, -108.54627227783203f},
            {-198.45195007324219f, -115.90768432617188f},
            {-182.97744750976562f, -128.12318420410156f},
            {-167.0f, -180.0f},
            {-130.33688354492188f, -173.83778381347656f},
            {-141.19784545898438f, -190.28807067871094f},
            {-111.09677124023438f, -214.48896789550781f},
            {-70.039543151855469f, -242.41630554199219f},
            {-68.960540771484375f, -228.30015563964844f},
            {-53.049201965332031f, -291.47097778320312f},
            {-28.695289611816406f, -317.64553833007812f},
            {57.0f, -300.0f},
            {45.301143646240234f, -335.69509887695312f},
            {91.936195373535156f, -373.32437133789062f},
            {172.09465026855469f, -439.275146484375f},
            {242.24473571777344f, -504.47515869140625f},
            {387.81732177734375f, -666.6788330078125f},
            {689.48553466796875f, -918.2142333984375f},
            {1646.539306640625f, -1694.1956787109375f}
        }
    };
}

template <typename T, typename U>
static EnableIfSecondComplex<T, U, BlockTestParams<T, U>> getTestParams(
    const std::string& fwdBlockRegistryPath,
    const std::string& invBlockRegistryPath)
{
    return
    {
        fwdBlockRegistryPath,
        invBlockRegistryPath,
        {
            2.0f,
            3.0f,
            5.0f,
            7.0f,
            11.0f,
            13.0f,
            17.0f,
            19.0f,
            23.0f,
            29.0f,
            31.0f,
            37.0f,
            41.0f,
            43.0f,
            47.0f,
            53.0f,
            59.0f,
            61.0f,
            67.0f,
            71.0f,
            73.0f,
            79.0f,
            83.0f,
            89.0f,
            97.0f,
            101.0f,
            103.0f,
            107.0f,
            109.0f,
            113.0f,
            127.0f,
            131.0f
        },
        {
            {1851.00000000f, 0.0f},
            {-6.61444228f, 701.09037655f},
            {-42.04129017f, 336.61190886f},
            {-49.47304737f, 211.75083962f},
            {-41.68629150f, 155.68124087f},
            {-54.91767178f, 145.18284377f},
            {-61.81068799f, 111.60856965f},
            {-69.75059856f, 85.73713738f},
            {-65.00000000f, 72.0f},
            {-73.61598442f, 53.91205472f},
            {-73.15987476f, 51.27552517f},
            {-70.25765877f, 24.26944698f},
            {-64.31370850f, 19.68124087f},
            {-70.32218483f, 19.14191661f},
            {-58.98814709f, 12.27886438f},
            {-61.04841199f, 5.56976767f},
            {-61.00000000f, 0.0f}
        }
    };
}

template <typename T>
static inline EnableIfNotComplex<T, T> getEpsilon()
{
    return T(1e-4);
}

template <typename T>
static inline EnableIfComplex<T, T> getEpsilon()
{
    using U = typename T::value_type;

    return T{getEpsilon<U>(), getEpsilon<U>()};
}

//
// Test code
//

template <typename T, typename U>
static void testFFT(
    const std::string& fwdBlockRegistryPath,
    const std::string& invBlockRegistryPath)
{
    const auto testParams = getTestParams<T, U>(
                                fwdBlockRegistryPath,
                                invBlockRegistryPath);
    POTHOS_TEST_TRUE(!testParams.inputs.empty());
    POTHOS_TEST_TRUE(!testParams.outputs.empty());

    Pothos::DType inputDType(typeid(T));
    Pothos::DType outputDType(typeid(T));
    std::cout << "Testing " << inputDType.toString() << " to "
                            << outputDType.toString() << std::endl;

    auto feeder = Pothos::BlockRegistry::make(
                      "/blocks/feeder_source",
                      inputDType);
    auto fwdFFTBlock = Pothos::BlockRegistry::make(
                           testParams.fwdBlockRegistryPath,
                           inputDType,
                           testParams.inputs.size());
    auto invFFTBlock = Pothos::BlockRegistry::make(
                           testParams.invBlockRegistryPath,
                           outputDType,
                           testParams.outputs.size());
    auto fwdCollector = Pothos::BlockRegistry::make(
                            "/blocks/collector_sink",
                            outputDType);
    auto invCollector = Pothos::BlockRegistry::make(
                            "/blocks/collector_sink",
                            inputDType);

    // Load the feeder
    feeder.call(
        "feedBuffer",
        stdVectorToBufferChunk(
            inputDType,
            testParams.inputs));

    // Run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, fwdFFTBlock, 0);
        topology.connect(fwdFFTBlock, 0, invFFTBlock, 0);
        topology.connect(fwdFFTBlock, 0, fwdCollector, 0);
        topology.connect(invFFTBlock, 0, invCollector, 0);
        topology.commit();

        // When this block exits, the flowgraph will stop.
        Poco::Thread::sleep(10);
    }

    // Test the collectors
    std::cout << " * Testing " << fwdBlockRegistryPath << std::endl;
    testBufferChunk(
        fwdCollector.call("getBuffer"),
        testParams.outputs,
        getEpsilon<U>());
    std::cout << " * Testing " << invBlockRegistryPath << std::endl;
    testBufferChunk(
        invCollector.call("getBuffer"),
        testParams.inputs,
        getEpsilon<T>());
}

POTHOS_TEST_BLOCK("/numpy/tests", test_fft)
{
    const std::string fft = "/numpy/fft/fft";
    const std::string ifft = "/numpy/fft/ifft";
    const std::string rfft = "/numpy/fft/rfft";
    const std::string irfft = "/numpy/fft/irfft";

    testFFT<std::complex<float>, std::complex<float>>(fft, ifft);
    testFFT<std::complex<double>, std::complex<double>>(fft, ifft);
    testFFT<float, std::complex<float>>(rfft, irfft);
    testFFT<double, std::complex<double>>(rfft, irfft);
}
