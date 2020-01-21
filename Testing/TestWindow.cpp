// Copyright (c) 2019-2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>

#include <iostream>
#include <string>

//
// Value tests for our window block. All expected values are from NumPy
// documentation.
//

// Test our window function by passing in identity and making sure the output
// of the window function itself is what's returned.
static void testWindowType(
    const std::string& windowType,
    const std::vector<double>& expectedValues)
{
    static const std::string blockRegistryPath = "/numpy/window";
    static const Pothos::DType dtype(typeid(double));

    static constexpr size_t ExpectedOutputSize = 12;
    static const std::vector<double> Identity(ExpectedOutputSize, 1.0);
    static constexpr double KaiserBeta = 14.0;

    std::cout << "Testing " << windowType << "..." << std::endl;

    auto windowBlock = Pothos::BlockRegistry::make(
                           blockRegistryPath,
                           dtype,
                           windowType);
    PothosNumPyTests::testEqual(
        windowType,
        windowBlock.call<std::string>("getWindowType"));

    windowBlock.call("setKaiserBeta", KaiserBeta);
    PothosNumPyTests::testEqual(
        KaiserBeta,
        windowBlock.call<double>("getKaiserBeta"));

    auto vectorSource = Pothos::BlockRegistry::make(
                            "/blocks/vector_source",
                            dtype);
    vectorSource.call("setElements", Identity);

    auto collectorSink = Pothos::BlockRegistry::make(
                             "/blocks/collector_sink",
                             dtype);

    // Execute the topology.
    {
        Pothos::Topology topology;

        topology.connect(
            vectorSource, 0,
            windowBlock, 0);
        topology.connect(
            windowBlock, 0,
            collectorSink, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01, 1.0));
    }

    PothosNumPyTests::testBufferChunk(
        collectorSink.call<Pothos::BufferChunk>("getBuffer"),
        PothosNumPyTests::stdVectorToBufferChunk(expectedValues));
}

POTHOS_TEST_BLOCK("/numpy/tests", test_windows)
{
    //
    // Bartlett
    //

    const std::vector<double> expectedBartlett =
    {
        0.0       ,  0.18181818,  0.36363636,  0.54545455,  0.72727273,
        0.90909091,  0.90909091,  0.72727273,  0.54545455,  0.36363636,
        0.18181818,  0.0
    };
    testWindowType("BARTLETT", expectedBartlett);

    //
    // Blackman
    //

    const std::vector<double> expectedBlackman =
    {
        -1.38777878e-17,  3.26064346e-02,   1.59903635e-01,
        4.14397981e-01,   7.36045180e-01,   9.67046769e-01,
        9.67046769e-01,   7.36045180e-01,   4.14397981e-01,
        1.59903635e-01,   3.26064346e-02,  -1.38777878e-17
    };
    testWindowType("BLACKMAN", expectedBlackman);

    //
    // Hamming
    //

    const std::vector<double> expectedHamming =
    {
        0.08      ,  0.15302337,  0.34890909,  0.60546483,  0.84123594,
        0.98136677,  0.98136677,  0.84123594,  0.60546483,  0.34890909,
        0.15302337,  0.08
    };
    testWindowType("HAMMING", expectedHamming);

    //
    // Hanning
    //

    const std::vector<double> expectedHanning =
    {
        0.0       , 0.07937323, 0.29229249, 0.57115742, 0.82743037,
        0.97974649, 0.97974649, 0.82743037, 0.57115742, 0.29229249,
        0.07937323, 0.0
    };
    testWindowType("HANNING", expectedHanning);

    //
    // Kaiser
    //

    const std::vector<double> expectedKaiser =
    {
        7.72686684e-06, 3.46009194e-03, 4.65200189e-02,
        2.29737120e-01, 5.99885316e-01, 9.45674898e-01,
        9.45674898e-01, 5.99885316e-01, 2.29737120e-01,
        4.65200189e-02, 3.46009194e-03, 7.72686684e-06
    };
    testWindowType("KAISER", expectedKaiser);
}
