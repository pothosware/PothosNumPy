// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

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

static std::string getNumPyVersion()
{
    auto pythonEnv = Pothos::ProxyEnvironment::make("python");
    auto numpy = pythonEnv->findProxy("numpy");
    return numpy.get<std::string>("__version__");
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
        "/numpy/info/config_info",
        Pothos::Callable(getNumPyConfigInfoJSONString));

    Pothos::PluginRegistry::addCall(
        "/numpy/info/finfo",
        Pothos::Callable(getNumPyFloatInfo));

    Pothos::PluginRegistry::addCall(
        "/numpy/info/iinfo",
        Pothos::Callable(getNumPyIntInfo));

    Pothos::PluginRegistry::addCall(
        "/numpy/info/version",
        Pothos::Callable(getNumPyVersion));
}
