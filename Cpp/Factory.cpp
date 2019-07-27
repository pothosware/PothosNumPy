// Copyright (c) 2014-2016 Josh Blum
//                    2019 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Callable.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>

#include <functional>

static Pothos::Object FactoryFunc(
    const Pothos::Object *args,
    const size_t numArgs,
    const std::string& pythonName)
{
    //create python environment
    auto env = Pothos::ProxyEnvironment::make("python");

    //convert arguments into proxy environment
    std::vector<Pothos::Proxy> proxyArgs(numArgs);
    for (size_t i = 0; i < numArgs; i++)
    {
        proxyArgs[i] = env->makeProxy(args[i]);
    }

    //locate the module
    auto mod = env->findProxy("PothosNumPy");

    //call into the factory
    auto block = mod.getHandle()->call(pythonName, proxyArgs.data(), proxyArgs.size());
    return Pothos::Object(block);
}

#define BLOCK_REGISTRY(path,name) \
    Pothos::BlockRegistry( \
        path, \
        Pothos::Callable(&FactoryFunc).bind<std::string>(name, 2))

static const std::vector<Pothos::BlockRegistry> blockRegistries =
{
    BLOCK_REGISTRY("/numpy/sin", "Sin"),
    BLOCK_REGISTRY("/numpy/cos", "Cos"),
    BLOCK_REGISTRY("/numpy/tan", "Tan"),
    BLOCK_REGISTRY("/numpy/arcsin", "ArcSin"),
    BLOCK_REGISTRY("/numpy/arccos", "ArcCos"),
    BLOCK_REGISTRY("/numpy/arctan", "ArcTan"),
    BLOCK_REGISTRY("/numpy/hypot", "Hypot"),
    BLOCK_REGISTRY("/numpy/arctan2", "ArcTan2"),
    BLOCK_REGISTRY("/numpy/degrees", "Degrees"),
    BLOCK_REGISTRY("/numpy/radians", "Radians"),
    BLOCK_REGISTRY("/numpy/sinh", "Sinh"),
    BLOCK_REGISTRY("/numpy/cosh", "Cosh"),
    BLOCK_REGISTRY("/numpy/tanh", "Tanh"),
    BLOCK_REGISTRY("/numpy/arcsinh", "ArcSinh"),
    BLOCK_REGISTRY("/numpy/arccosh", "ArcCosh"),
    BLOCK_REGISTRY("/numpy/arctanh", "ArcTanh"),
    BLOCK_REGISTRY("/numpy/floor", "Floor"),
    BLOCK_REGISTRY("/numpy/ceil", "Ceil"),
    BLOCK_REGISTRY("/numpy/trunc", "Trunc"),
    BLOCK_REGISTRY("/numpy/exp", "Exp"),
    BLOCK_REGISTRY("/numpy/expm1", "ExpM1"),
    BLOCK_REGISTRY("/numpy/exp2", "Exp2"),
    BLOCK_REGISTRY("/numpy/log", "Log"),
    BLOCK_REGISTRY("/numpy/log10", "Log10"),
    BLOCK_REGISTRY("/numpy/log2", "Log2"),
    BLOCK_REGISTRY("/numpy/log1p", "Log1P"),
    BLOCK_REGISTRY("/numpy/logaddexp", "LogAddExp"),
    BLOCK_REGISTRY("/numpy/logaddexp2", "LogAddExp2"),
    BLOCK_REGISTRY("/numpy/sinc", "NumpySinc"),
    BLOCK_REGISTRY("/numpy/reciprocal", "Reciprocal"),
    BLOCK_REGISTRY("/numpy/positive", "Positive"),
    BLOCK_REGISTRY("/numpy/negative", "Negative"),
    BLOCK_REGISTRY("/numpy/conjugate", "Conjugate"),
    BLOCK_REGISTRY("/numpy/sqrt", "Sqrt"),
    BLOCK_REGISTRY("/numpy/cbrt", "Cbrt"),
    BLOCK_REGISTRY("/numpy/square", "Square"),
    BLOCK_REGISTRY("/numpy/absolute", "Absolute"),
    BLOCK_REGISTRY("/numpy/invert", "Invert"),
    BLOCK_REGISTRY("/numpy/load", "Load"),
    BLOCK_REGISTRY("/numpy/save", "Save"),
    BLOCK_REGISTRY("/numpy/savez", "SaveZ"),
    BLOCK_REGISTRY("/numpy/savez_compressed", "SaveZCompressed"),
    BLOCK_REGISTRY("/numpy/ones", "Ones"),
    BLOCK_REGISTRY("/numpy/zeros", "Zeros"),
    BLOCK_REGISTRY("/numpy/full", "Full"),
    BLOCK_REGISTRY("/numpy/arange", "ARange"),
    BLOCK_REGISTRY("/numpy/linspace", "LinSpace"),
    BLOCK_REGISTRY("/numpy/logspace", "LogSpace"),
    BLOCK_REGISTRY("/numpy/geomspace", "GeomSpace"),
    BLOCK_REGISTRY("/numpy/flip", "Flip"),
    BLOCK_REGISTRY("/numpy/roll", "Roll")
};
