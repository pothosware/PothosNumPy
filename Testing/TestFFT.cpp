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
    std::vector<T> inputs;
    std::vector<U> outputs;
};

template <typename T, typename U, typename V>
struct HFFTTestParams
{
    std::vector<T> inputs;
    std::vector<U> outputs;
    std::vector<V> revOutputs; // rev(fwd) != fwd
};

//
// Utility code
//

template <typename T, typename U>
static EnableIfBothComplex<T, U, BlockTestParams<T, U>> getFFTTestParams()
{
    return
    {
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
static EnableIfSecondComplex<T, U, BlockTestParams<T, U>> getRFFTTestParams()
{
    return
    {
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
static EnableIfNotComplex<T, HFFTTestParams<T, T, std::complex<T>>> getHFFTTestParams()
{
    return
    {
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
            3.56900000e+03,
            -1.64928469e+03,
            1.07075581e+02,
            -1.53836002e+02,
            3.90284420e+01,
            -4.53271404e+01,
            1.91360555e+01,
            -3.16612516e+01,
            3.41560464e+01,
            -6.27234033e+01,
            3.90497458e+01,
            -2.65584499e+01,
            2.80602105e+01,
            -1.74606915e+01,
            8.77395827e+00,
            -1.04465222e+01,
            1.94275496e+01,
            -1.39193347e+01,
            3.09339995e-01,
            -1.70571897e+01,
            1.69617628e+01,
            6.27984087e+00,
            -9.93177578e+00,
            7.80465318e+00,
            -1.11310235e+01,
            -8.73754814e+00,
            -7.88886193e+00,
            1.27003850e+01,
            -2.58170535e+00,
            7.22734603e+00,
            -3.44532480e+00,
            7.00000000e+00,
            -3.44532480e+00,
            7.22734603e+00,
            -2.58170535e+00,
            1.27003850e+01,
            -7.88886193e+00,
            -8.73754814e+00,
            -1.11310235e+01,
            7.80465318e+00,
            -9.93177578e+00,
            6.27984087e+00,
            1.69617628e+01,
            -1.70571897e+01,
            3.09339995e-01,
            -1.39193347e+01,
            1.94275496e+01,
            -1.04465222e+01,
            8.77395827e+00,
            -1.74606915e+01,
            2.80602105e+01,
            -2.65584499e+01,
            3.90497458e+01,
            -6.27234033e+01,
            3.41560464e+01,
            -3.16612516e+01,
            1.91360555e+01,
            -4.53271404e+01,
            3.90284420e+01,
            -1.53836002e+02,
            1.07075581e+02,
            -1.64928469e+03
        },
        {
            {2.0f, +0.00000000e+00f},
            {3.0f, -1.83365867e-15f},
            {5.0f, +0.00000000e+00f},
            {7.0f, -1.83365867e-15f},
            {11.0f, -1.46692694e-14f},
            {13.0f, +3.66731735e-15f},
            {17.0f, +1.10019520e-14f},
            {19.0f, +3.66731735e-15f},
            {23.0f, -3.66731735e-15f},
            {29.0f, +3.66731735e-15f},
            {31.0f, +0.00000000e+00f},
            {37.0f, +1.28356107e-14f},
            {41.0f, +1.37524400e-14f},
            {43.0f, +1.23771960e-14f},
            {47.0f, +1.42108547e-14f},
            {53.0f, +9.94186812e-15f},
            {59.0f, +9.94186812e-15f},
            {61.0f, +1.42108547e-14f},
            {67.0f, +1.23771960e-14f},
            {71.0f, +1.37524400e-14f},
            {73.0f, +1.28356107e-14f},
            {79.0f, +0.00000000e+00f},
            {83.0f, +3.66731735e-15f},
            {89.0f, -3.66731735e-15f},
            {97.0f, +3.66731735e-15f},
            {101.0f, +1.10019520e-14f},
            {103.0f, +3.66731735e-15f},
            {107.0f, -1.46692694e-14f},
            {109.0f, -1.83365867e-15f},
            {113.0f, +0.00000000e+00f},
            {127.0f, -1.83365867e-15f},
            {131.0f, +0.00000000e+00f},
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

template <typename T>
static void testFFT()
{
    const std::string fwdBlockRegistryPath = "/numpy/fft/fft";
    const std::string invBlockRegistryPath = "/numpy/fft/ifft";

    using Complex = std::complex<T>;

    const auto testParams = getFFTTestParams<Complex, Complex>();
    POTHOS_TEST_TRUE(!testParams.inputs.empty());
    POTHOS_TEST_TRUE(!testParams.outputs.empty());

    Pothos::DType dtype(typeid(T));
    Pothos::DType complexDType(typeid(Complex));
    std::cout << "Testing " << complexDType.toString() << " to "
                            << complexDType.toString() << std::endl;

    auto feeder = Pothos::BlockRegistry::make(
                      "/blocks/feeder_source",
                      complexDType);
    auto fwdFFTBlock = Pothos::BlockRegistry::make(
                           fwdBlockRegistryPath,
                           complexDType,
                           testParams.inputs.size());
    auto invFFTBlock = Pothos::BlockRegistry::make(
                           invBlockRegistryPath,
                           complexDType,
                           testParams.outputs.size());
    auto fwdCollector = Pothos::BlockRegistry::make(
                            "/blocks/collector_sink",
                            complexDType);
    auto invCollector = Pothos::BlockRegistry::make(
                            "/blocks/collector_sink",
                            complexDType);

    // Load the feeder
    feeder.call(
        "feedBuffer",
        stdVectorToBufferChunk(
            complexDType,
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
        getEpsilon<Complex>());
    std::cout << " * Testing " << invBlockRegistryPath << std::endl;
    testBufferChunk(
        invCollector.call("getBuffer"),
        testParams.inputs,
        getEpsilon<Complex>());
}

template <typename T>
static void testRFFT()
{
    const std::string fwdBlockRegistryPath = "/numpy/fft/rfft";
    const std::string invBlockRegistryPath = "/numpy/fft/irfft";

    using Complex = std::complex<T>;

    const auto testParams = getRFFTTestParams<T, Complex>();
    POTHOS_TEST_TRUE(!testParams.inputs.empty());
    POTHOS_TEST_TRUE(!testParams.outputs.empty());

    Pothos::DType dtype(typeid(T));
    Pothos::DType complexDType(typeid(Complex));
    std::cout << "Testing " << dtype.toString() << " to "
                            << complexDType.toString() << std::endl;

    auto feeder = Pothos::BlockRegistry::make(
                      "/blocks/feeder_source",
                      dtype);
    auto fwdFFTBlock = Pothos::BlockRegistry::make(
                           fwdBlockRegistryPath,
                           dtype,
                           testParams.inputs.size());
    auto invFFTBlock = Pothos::BlockRegistry::make(
                           invBlockRegistryPath,
                           dtype,
                           testParams.outputs.size());
    auto fwdCollector = Pothos::BlockRegistry::make(
                            "/blocks/collector_sink",
                            complexDType);
    auto invCollector = Pothos::BlockRegistry::make(
                            "/blocks/collector_sink",
                            dtype);

    // Load the feeder
    feeder.call(
        "feedBuffer",
        stdVectorToBufferChunk(
            dtype,
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
        getEpsilon<Complex>());
    std::cout << " * Testing " << invBlockRegistryPath << std::endl;
    testBufferChunk(
        invCollector.call("getBuffer"),
        testParams.inputs,
        getEpsilon<T>());
}

template <typename T>
static void testHFFT()
{
    const std::string fwdBlockRegistryPath = "/numpy/fft/hfft";
    const std::string invBlockRegistryPath = "/numpy/fft/ihfft";

    const auto testParams = getHFFTTestParams<T>();
    POTHOS_TEST_TRUE(!testParams.inputs.empty());
    POTHOS_TEST_TRUE(!testParams.outputs.empty());

    Pothos::DType dtype(typeid(T));
    Pothos::DType complexDType(typeid(std::complex<T>));
    std::cout << "Testing " << dtype.toString() << " to "
                            << dtype.toString() << " to "
                            << complexDType.toString() << std::endl;

    auto feeder = Pothos::BlockRegistry::make(
                      "/blocks/feeder_source",
                      dtype);
    auto fwdFFTBlock = Pothos::BlockRegistry::make(
                           fwdBlockRegistryPath,
                           dtype,
                           testParams.inputs.size());
    auto invFFTBlock = Pothos::BlockRegistry::make(
                           invBlockRegistryPath,
                           dtype,
                           testParams.outputs.size());
    auto fwdCollector = Pothos::BlockRegistry::make(
                            "/blocks/collector_sink",
                            dtype);
    auto invCollector = Pothos::BlockRegistry::make(
                            "/blocks/collector_sink",
                            complexDType);

    // Load the feeder
    feeder.call(
        "feedBuffer",
        stdVectorToBufferChunk(
            dtype,
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
        getEpsilon<T>());
    std::cout << " * Testing " << invBlockRegistryPath << std::endl;
    testBufferChunk(
        invCollector.call("getBuffer"),
        testParams.revOutputs,
        getEpsilon<std::complex<T>>());
}

// TODO: test scalar into FFT
POTHOS_TEST_BLOCK("/numpy/tests", test_fft)
{
    // TODO: test scalar input
    testFFT<float>();
    testFFT<double>();

    testRFFT<float>();
    testRFFT<double>();

    // TODO: test complex input
    testHFFT<float>();
    testHFFT<double>();
}
