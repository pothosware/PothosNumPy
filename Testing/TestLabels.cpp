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
#include <vector>

//
// Get expected values for the labels.
//

template <typename T>
static T max(const std::vector<T>& inputs, size_t* pPosition)
{
    auto maxElement = std::max_element(inputs.begin(), inputs.end());
    *pPosition = (maxElement - inputs.begin());
    return *maxElement;
}

template <typename T>
static T min(const std::vector<T>& inputs, size_t* pPosition)
{
    auto minElement = std::min_element(inputs.begin(), inputs.end());
    *pPosition = (minElement - inputs.begin());
    return *minElement;
}

template <typename T>
static T ptp(const std::vector<T>& inputs)
{
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

    *pPosition = size_t(std::floor(inputs[inputs.size()/2]));

    return sortedInputs[*pPosition];
}

template <typename T>
static T stddev(const std::vector<T>& inputs)
{
    const T inputMean = mean(inputs);
    const size_t size = inputs.size();

    const T oneDivSizeMinusOne = 1.0 / static_cast<T>(size-1);

    std::vector<T> diffs;
    diffs.reserve(size);
    std::transform(
        inputs.begin(),
        inputs.end(),
        std::back_inserter(diffs),
        [&inputMean, &size](T input)
        {
            return std::pow((input - inputMean), T(2));
        });

    const T addedDiffs = std::accumulate(diffs.begin(), diffs.end(), T(0));

    return std::sqrt(oneDivSizeMinusOne * addedDiffs);
}

template <typename T>
static T variance(const std::vector<T>& inputs)
{
    return std::pow(stddev(inputs), T(2));
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

static std::string getTestPlanJSON(const std::vector<double>& inputs)
{
    size_t expectedMaxPosition = 0;
    size_t expectedMinPosition = 0;
    //size_t expectedMedianPosition = 0;

    const auto expectedMax = max(inputs, &expectedMaxPosition);
    const auto expectedMin = min(inputs, &expectedMinPosition);
    const auto expectedPTP = ptp(inputs);
    const auto expectedMean = mean(inputs);
    //const auto expectedMedian = median(inputs, &expectedMedianPosition);
    const auto expectedStdDev = stddev(inputs);
    const auto expectedVariance = variance(inputs);
    const auto expectedCountNonZeros = countNonZeros(inputs);

    auto topLevelJSON = nlohmann::json();
    auto& expectedLabelsJSON = topLevelJSON["expectedLabels"];
    expectedLabelsJSON = nlohmann::json::array();

    const std::vector<Pothos::Label> expectedLabels =
    {
        {"MAX", expectedMax, expectedMaxPosition},
        {"MIN", expectedMin, expectedMinPosition},
        {"PTP", expectedPTP, 0},
        {"MEAN", expectedMean, 0},
        {"STD", expectedStdDev, 0},
        {"VAR", expectedVariance, 0},
        {"NONZERO", expectedCountNonZeros, 0}
    };
    std::transform(
        expectedLabels.begin(),
        expectedLabels.end(),
        std::back_inserter(expectedLabelsJSON),
        [](const Pothos::Label& label)
        {
            nlohmann::json json;
            json["index"] = label.index;
            json["id"] = label.id;

            if(label.id == "NONZERO")
            {
                json["data"] = label.data.extract<size_t>();
            }
            else
            {
                json["data"] = label.data.extract<double>();
            }

            return json;
        });

    return topLevelJSON.dump();
}

POTHOS_TEST_BLOCK("/numpy/tests", test_labels)
{
    std::vector<double> inputs = linspace<double>(-10, 10, 50);
    inputs.emplace_back(0.0); // To test PTP
    std::random_shuffle(inputs.begin(), inputs.end());

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
        // Pothos::BlockRegistry::make("numpy/median", dtype, false),
        Pothos::BlockRegistry::make("/numpy/std", dtype, false),
        Pothos::BlockRegistry::make("/numpy/var", dtype, false),
        Pothos::BlockRegistry::make("/numpy/ptp", dtype),
        Pothos::BlockRegistry::make("/numpy/count_nonzero", dtype)
    };

    auto collectorSink = Pothos::BlockRegistry::make(
                             "/blocks/collector_sink",
                             dtype);

    // Execute the topology.
    {
        auto topology = Pothos::Topology::make();

        for(const auto& block: numpyBlocks)
        {
            topology->connect(
                vectorSource,
                0,
                block,
                0);
        }
        for(const auto& block: numpyBlocks)
        {
            topology->connect(
                block,
                0,
                collectorSink,
                0);
        }

        topology->commit();
        POTHOS_TEST_TRUE(topology->waitInactive(0.01, 0.0));
    }

    collectorSink.call("verifyTestPlan", getTestPlanJSON(inputs));
}
