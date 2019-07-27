// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TestUtility.hpp"

#include <Pothos/Exception.hpp>
#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <iostream>

POTHOS_TEST_BLOCK("/scipy/tests", TestPhysicalConstant)
{
    auto plugin = Pothos::PluginRegistry::get("/scipy/constants/get_physical_constant");
    auto getter = plugin.getObject().extract<Pothos::Callable>();

    POTHOS_TEST_THROWS(
        getter.call<Pothos::Object>("Not a key"),
        Pothos::NotFoundException);

    auto physicalConstantInfo = getter.call<Pothos::Proxy>("atomic unit of mass");

    const double expectedValue = 9.10938356e-31;
    const std::string expectedUnit = "kg";
    const double expectedUncertainty = 1.1e-38;

    POTHOS_TEST_CLOSE(
        physicalConstantInfo.get<double>("Value"),
        expectedValue,
        1e-9);
    POTHOS_TEST_EQUAL(
        physicalConstantInfo.get<std::string>("Unit"),
        expectedUnit);
    POTHOS_TEST_CLOSE(
        physicalConstantInfo.get<double>("Uncertainty"),
        expectedUncertainty,
        1e-9);
}
