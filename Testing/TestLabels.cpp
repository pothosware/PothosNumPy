// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include "TestUtility.hpp"

#include <nlohmann/json.hpp>

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <vector>

//
// Get expected values for the labels.
//

template <typename T>
static T max(const std::vector<T>& inputs, size_t* pPosition)
{
    auto maxElementIter = std::max_element(inputs.begin(), inputs.end());
    POTHOS_TEST_TRUE(inputs.end() != maxElementIter);

    *pPosition = std::distance(inputs.begin(), maxElementIter);
    return *maxElementIter;
}

template <typename T>
static T min(const std::vector<T>& inputs, size_t* pPosition)
{
    auto minElementIter = std::min_element(inputs.begin(), inputs.end());
    POTHOS_TEST_TRUE(inputs.end() != minElementIter);

    *pPosition = std::distance(inputs.begin(), minElementIter);
    return *minElementIter;
}

template <typename T>
static T ptp(const std::vector<T>& inputs)
{
    // Unused
    size_t _1, _2;

    return max(inputs, &_1) - min(inputs, &_2);
}

template <typename T>
static T mean(const std::vector<T>& inputs)
{
    return std::accumulate(
               inputs.begin(),
               inputs.end(),
               T(0)) / static_cast<T>(inputs.size());
}

template <typename T>
static T median(const std::vector<T>& inputs, size_t* pPosition)
{
    std::vector<T> sortedInputs(inputs);
    std::sort(sortedInputs.begin(), sortedInputs.end());

    auto sortedIndex = size_t(std::floor(inputs.size()/2));
    auto unsortedIter = std::find(
                            inputs.begin(),
                            inputs.end(),
                            sortedInputs[sortedIndex]);
    POTHOS_TEST_TRUE(unsortedIter != inputs.end());

    *pPosition = std::distance(inputs.begin(), unsortedIter);

    return sortedInputs[sortedIndex];
}

template <typename T>
static T stddev(const std::vector<T>& inputs)
{
    // Borderline cheating, but we can't reliably account for the floating-point
    // conversions when generating a test value.
    auto env = Pothos::ProxyEnvironment::make("python");
    auto numpy = env->findProxy("numpy");
    return numpy.call<T>("std", inputs);
}

template <typename T>
static T variance(const std::vector<T>& inputs)
{
    // Borderline cheating, but we can't reliably account for the floating-point
    // conversions when generating a test value.
    auto env = Pothos::ProxyEnvironment::make("python");
    auto numpy = env->findProxy("numpy");
    return numpy.call<T>("var", inputs);
}

template <typename T>
static size_t countNonZeros(const std::vector<T>& inputs)
{
    return std::count_if(
               inputs.begin(),
               inputs.end(),
               [](T input){return (input != T(0));});
}

//
// Make sure that blocks that post labels post the labels we expect.
//

static std::vector<Pothos::Label> getExpectedLabels(const std::vector<double>& inputs)
{
    size_t expectedMaxPosition = 0;
    size_t expectedMinPosition = 0;
    size_t expectedMedianPosition = 0;

    const auto expectedMax = max(inputs, &expectedMaxPosition);
    const auto expectedMin = min(inputs, &expectedMinPosition);
    const auto expectedPTP = ptp(inputs);
    const auto expectedMean = mean(inputs);
    const auto expectedMedian = median(inputs, &expectedMedianPosition);
    const auto expectedStdDev = stddev(inputs);
    const auto expectedVariance = variance(inputs);
    const auto expectedCountNonZeros = countNonZeros(inputs);

    return std::vector<Pothos::Label>
    ({
        Pothos::Label("MAX", expectedMax, expectedMaxPosition),
        Pothos::Label("MIN", expectedMin, expectedMinPosition),
        Pothos::Label("MEAN", expectedMean, 0),
        Pothos::Label("MEDIAN", expectedMedian, expectedMedianPosition),
        Pothos::Label("STD", expectedStdDev, 0),
        Pothos::Label("VAR", expectedVariance, 0),
        Pothos::Label("PTP", expectedPTP, 0),
        Pothos::Label("NONZERO", expectedCountNonZeros, 0)
    });
}

POTHOS_TEST_BLOCK("/numpy/tests", test_labels)
{
    std::random_device rd;
    std::mt19937 g(rd());

    std::vector<double> inputs = linspace<double>(-10, 10, 50);
    inputs.emplace_back(0.0); // To test PTP
    std::shuffle(inputs.begin(), inputs.end(), g);

    const auto dtype = Pothos::DType("float64");

    auto vectorSource = Pothos::BlockRegistry::make(
                            "/blocks/vector_source",
                            dtype);
    vectorSource.call("setMode", "ONCE");
    vectorSource.call("setElements", inputs);

    // NaN functions will be tested elsewhere.
    const std::vector<Pothos::Proxy> numpyBlocks =
    {
        Pothos::BlockRegistry::make("/numpy/max", dtype, false),
        Pothos::BlockRegistry::make("/numpy/min", dtype, false),
        Pothos::BlockRegistry::make("/numpy/mean", dtype, false),
        Pothos::BlockRegistry::make("/numpy/median", dtype, false),
        Pothos::BlockRegistry::make("/numpy/std", dtype, false),
        Pothos::BlockRegistry::make("/numpy/var", dtype, false),
        Pothos::BlockRegistry::make("/numpy/ptp", dtype),
        Pothos::BlockRegistry::make("/numpy/count_nonzero", dtype)
    };
    const size_t numBlocks = numpyBlocks.size();

    std::vector<Pothos::Proxy> collectorSinks;
    for(size_t blockIndex = 0; blockIndex < numBlocks; ++blockIndex)
    {
        collectorSinks.emplace_back(Pothos::BlockRegistry::make(
                                        "/blocks/collector_sink",
                                        dtype));
    }

    // Execute the topology.
    {
        auto topology = Pothos::Topology::make();

        for(size_t blockIndex = 0; blockIndex < numBlocks; ++blockIndex)
        {
            topology->connect(
                vectorSource,
                0,
                numpyBlocks[blockIndex],
                0);
            topology->connect(
                numpyBlocks[blockIndex],
                0,
                collectorSinks[blockIndex],
                0);
        }

        topology->commit();
        POTHOS_TEST_TRUE(topology->waitInactive(0.01, 0.0));
    }

    auto expectedLabels = getExpectedLabels(inputs);
    POTHOS_TEST_EQUAL(expectedLabels.size(), numBlocks);

    for(size_t labelIndex = 0; labelIndex < numBlocks; ++labelIndex)
    {
        const auto& expectedLabel = expectedLabels[labelIndex];

        auto blockLabels = collectorSinks[labelIndex].call<std::vector<Pothos::Label>>("getLabels");
        POTHOS_TEST_EQUAL(1, blockLabels.size());
        const auto& blockLabel = blockLabels[0];

        std::cout << "Testing label " << expectedLabel.id << std::endl;

        std::cout << " * ID..." << std::endl;
        POTHOS_TEST_EQUAL(
            expectedLabel.id,
            blockLabel.id);
        std::cout << " * Index..." << std::endl;
        testEqual(
            expectedLabel.index,
            blockLabel.index);
        std::cout << " * Data..." << std::endl;
        if(expectedLabel.id == "NONZERO")
        {
            testEqual(
                expectedLabel.data.convert<size_t>(),
                blockLabel.data.convert<size_t>());
        }
        else
        {
            testEqual(
                expectedLabel.data.convert<double>(),
                blockLabel.data.convert<double>());
        }
    }
}
