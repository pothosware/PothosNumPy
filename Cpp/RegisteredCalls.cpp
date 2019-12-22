// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include <Pothos/Callable.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <functional>

static std::string getNumPyConfigInfoJSONString()
{
    auto pythonEnv = Pothos::ProxyEnvironment::make("python");
    auto PothosNumPy = pythonEnv->findProxy("PothosNumPy");

    return PothosNumPy.call<std::string>("getNumPyConfigInfoJSONString");
}

static Pothos::Proxy getNumPyIntInfo(const Pothos::DType& dtype)
{
    auto pythonEnv = Pothos::ProxyEnvironment::make("python");
    auto PothosNumPy = pythonEnv->findProxy("PothosNumPy");

    return PothosNumPy.call("getNumPyIntInfoFromPothosDType", dtype);
}

static Pothos::Proxy getNumPyFloatInfo(const Pothos::DType& dtype)
{
    auto pythonEnv = Pothos::ProxyEnvironment::make("python");
    auto PothosNumPy = pythonEnv->findProxy("PothosNumPy");

    return PothosNumPy.call("getNumPyFloatInfoFromPothosDType", dtype);
}

pothos_static_block(registerCalls)
{
    Pothos::PluginRegistry::addCall(
        "/devices/numpy/info",
        Pothos::Callable(getNumPyConfigInfoJSONString));

    Pothos::PluginRegistry::addCall(
        "/numpy/info/finfo",
        Pothos::Callable(getNumPyFloatInfo));

    Pothos::PluginRegistry::addCall(
        "/numpy/info/iinfo",
        Pothos::Callable(getNumPyIntInfo));
}
