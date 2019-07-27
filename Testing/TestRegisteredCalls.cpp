// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TestUtility.hpp"

#include <nlohmann/json.hpp>

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <string>

POTHOS_TEST_BLOCK("/numpy/tests", test_registered_calls)
{
    // Make sure this is valid JSON. This will throw if the JSON is invalid.
    std::string numpyConfigInfo = getAndCallPlugin<std::string>("/numpy/info/config_info");
    POTHOS_TEST_TRUE(!numpyConfigInfo.empty());
    auto json = nlohmann::json::parse(numpyConfigInfo);

    std::string numpyVersion = getAndCallPlugin<std::string>("/numpy/info/version");
    POTHOS_TEST_TRUE(!numpyVersion.empty());
}
