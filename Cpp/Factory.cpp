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
    auto mod = env->findProxy("PothosSciPy");

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
    BLOCK_REGISTRY("/scipy/numpy/sin", "NumPySin"),
    BLOCK_REGISTRY("/scipy/numpy/cos", "NumPyCos"),
    BLOCK_REGISTRY("/scipy/numpy/tan", "NumPyTan"),
    BLOCK_REGISTRY("/scipy/numpy/arcsin", "NumPyArcSin"),
    BLOCK_REGISTRY("/scipy/numpy/arccos", "NumPyArcCos"),
    BLOCK_REGISTRY("/scipy/numpy/arctan", "NumPyArcTan"),
    BLOCK_REGISTRY("/scipy/numpy/hypot", "NumPyHypot"),
    BLOCK_REGISTRY("/scipy/numpy/arctan2", "NumPyArcTan2"),
    BLOCK_REGISTRY("/scipy/numpy/degrees", "NumPyDegrees"),
    BLOCK_REGISTRY("/scipy/numpy/radians", "NumPyRadians"),
    BLOCK_REGISTRY("/scipy/numpy/sinh", "NumPySinh"),
    BLOCK_REGISTRY("/scipy/numpy/cosh", "NumPyCosh"),
    BLOCK_REGISTRY("/scipy/numpy/tanh", "NumPyTanh"),
    BLOCK_REGISTRY("/scipy/numpy/arcsinh", "NumPyArcSinh"),
    BLOCK_REGISTRY("/scipy/numpy/arccosh", "NumPyArcCosh"),
    BLOCK_REGISTRY("/scipy/numpy/arctanh", "NumPyArcTanh"),
    BLOCK_REGISTRY("/scipy/numpy/floor", "NumPyFloor"),
    BLOCK_REGISTRY("/scipy/numpy/ceil", "NumPyCeil"),
    BLOCK_REGISTRY("/scipy/numpy/trunc", "NumPyTrunc"),
    BLOCK_REGISTRY("/scipy/numpy/exp", "NumPyExp"),
    BLOCK_REGISTRY("/scipy/numpy/expm1", "NumPyExpM1"),
    BLOCK_REGISTRY("/scipy/numpy/exp2", "NumPyExp2"),
    BLOCK_REGISTRY("/scipy/numpy/log", "NumPyLog"),
    BLOCK_REGISTRY("/scipy/numpy/log10", "NumPyLog10"),
    BLOCK_REGISTRY("/scipy/numpy/log2", "NumPyLog2"),
    BLOCK_REGISTRY("/scipy/numpy/log1p", "NumPyLog1P"),
    BLOCK_REGISTRY("/scipy/numpy/logaddexp", "NumPyLogAddExp"),
    BLOCK_REGISTRY("/scipy/numpy/logaddexp2", "NumPyLogAddExp2"),
    BLOCK_REGISTRY("/scipy/numpy/sinc", "NumpySinc"),
    BLOCK_REGISTRY("/scipy/numpy/reciprocal", "NumPyReciprocal"),
    BLOCK_REGISTRY("/scipy/numpy/positive", "NumPyPositive"),
    BLOCK_REGISTRY("/scipy/numpy/negative", "NumPyNegative"),
    BLOCK_REGISTRY("/scipy/numpy/conjugate", "NumPyConjugate"),
    BLOCK_REGISTRY("/scipy/numpy/sqrt", "NumPySqrt"),
    BLOCK_REGISTRY("/scipy/numpy/cbrt", "NumPyCbrt"),
    BLOCK_REGISTRY("/scipy/numpy/square", "NumPySquare"),
    BLOCK_REGISTRY("/scipy/numpy/absolute", "NumPyAbsolute"),
    BLOCK_REGISTRY("/scipy/numpy/invert", "NumPyInvert"),
    BLOCK_REGISTRY("/scipy/numpy/load", "NumPyLoad"),
    BLOCK_REGISTRY("/scipy/numpy/save", "NumPySave"),
    BLOCK_REGISTRY("/scipy/numpy/savez", "NumPySaveZ"),
    BLOCK_REGISTRY("/scipy/numpy/savez_compressed", "NumPySaveZCompressed"),
    BLOCK_REGISTRY("/scipy/numpy/ones", "NumPyOnes"),
    BLOCK_REGISTRY("/scipy/numpy/zeros", "NumPyZeros"),
    BLOCK_REGISTRY("/scipy/numpy/full", "NumPyFull"),
    BLOCK_REGISTRY("/scipy/numpy/arange", "NumPyARange"),
    BLOCK_REGISTRY("/scipy/numpy/linspace", "NumPyLinSpace"),
    BLOCK_REGISTRY("/scipy/numpy/logspace", "NumPyLogSpace"),
    BLOCK_REGISTRY("/scipy/numpy/geomspace", "NumPyGeomSpace"),

    BLOCK_REGISTRY("/scipy/constants/lambda2nu", "SciPyLambda2Nu"),
    BLOCK_REGISTRY("/scipy/constants/nu2lambda", "SciPyNu2Lambda"),

    BLOCK_REGISTRY("/scipy/signal/bspline", "SciPySignalBSpline"),
    BLOCK_REGISTRY("/scipy/signal/cubic", "SciPySignalCubic"),
    BLOCK_REGISTRY("/scipy/signal/gauss_spline", "SciPySignalGaussSpline"),
    BLOCK_REGISTRY("/scipy/signal/quadratic", "SciPySignalQuadratic"),
};
