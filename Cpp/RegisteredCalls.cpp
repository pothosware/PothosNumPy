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

static Pothos::BufferChunk getNumPyWindow(
    const std::string& windowType,
    size_t windowSize)
{
    auto pythonEnv = Pothos::ProxyEnvironment::make("python");
    auto numpy = pythonEnv->findProxy("numpy");

    return numpy.call<Pothos::BufferChunk>(windowType, windowSize);
}

static Pothos::BufferChunk getNumPyKaiser(size_t windowSize, double beta)
{
    auto pythonEnv = Pothos::ProxyEnvironment::make("python");
    auto numpy = pythonEnv->findProxy("numpy");

    return numpy.call<Pothos::BufferChunk>("kaiser", windowSize, beta);
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

    const std::vector<std::string> windowTypes =
    {
        "bartlett",
        "blackman",
        "hamming",
        "hanning"
    };
    for(const auto& windowType: windowTypes)
    {
        std::string pluginName = "/numpy/window/" + windowType;
        Pothos::PluginRegistry::addCall(
            std::string("/numpy/window/" + windowType),
            Pothos::Callable(getNumPyWindow)
                .bind<std::string>(std::string(windowType), 0));
    }

    // Add Kaiser separately because of the different function signature.
    Pothos::PluginRegistry::addCall(
        "/numpy/window/kaiser",
        Pothos::Callable(getNumPyKaiser));
}
