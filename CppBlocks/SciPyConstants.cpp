// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include <Pothos/Exception.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Managed.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <iostream>
#include <string>
#include <typeinfo>

struct SciPyPhysicalConstant
{
    double value;
    std::string unit;
    double uncertainty;
};
static const std::string ManagedClassProxyPath = "scipy/PhysicalConstants";

static Pothos::ProxyMap getAllSciPyConstants()
{
    auto pythonEnv = Pothos::ProxyEnvironment::make("python");
    auto module = pythonEnv->findProxy("PothosSciPy");

    return module.call<Pothos::ProxyMap>("getAllSciPyConstants");
}

static Pothos::Proxy getSciPyPhysicalConstant(const std::string& name)
{
    auto pythonEnv = Pothos::ProxyEnvironment::make("python");
    auto module = pythonEnv->findProxy("scipy.constants");

    auto physicalConstantsDict = module.get("physical_constants");
    if(!physicalConstantsDict.call<bool>("__contains__", name))
    {
        throw Pothos::NotFoundException(
                  "Invalid SciPy physical constant",
                  name);
    }

    auto physicalConstantsTuple = physicalConstantsDict.call<Pothos::ProxyVector>("get", name);

    // Make sure the output is what we expect before attempting to call into it.
    if(physicalConstantsTuple.size() != 3)
    {
        throw Pothos::RuntimeException(
                  "The tuple returned by SciPy is of an unexpected length",
                  std::to_string(physicalConstantsTuple.size()));
    }
    if(!physicalConstantsTuple[0].toObject().canConvert(typeid(double)))
    {
        throw Pothos::RuntimeException(
                  "The first element of the tuple returned by SciPy is of an unexpected type",
                  physicalConstantsTuple[0].getClassName());
    }
    if(physicalConstantsTuple[1].toObject().type() != typeid(std::string))
    {
        throw Pothos::RuntimeException(
                  "The second element of the tuple returned by SciPy is of an unexpected type",
                  physicalConstantsTuple[1].getClassName());
    }
    if(!physicalConstantsTuple[2].toObject().canConvert(typeid(double)))
    {
        throw Pothos::RuntimeException(
                  "The third element of the tuple returned by SciPy is of an unexpected type",
                  physicalConstantsTuple[2].getClassName());
    }

    auto managedEnv = Pothos::ProxyEnvironment::make("managed");
    auto cls = managedEnv->findProxy(ManagedClassProxyPath);

    auto managedConstants = cls();
    managedConstants.set("Value", physicalConstantsTuple[0].convert<double>());
    managedConstants.set("Unit", physicalConstantsTuple[1].toObject().extract<std::string>());
    managedConstants.set("Uncertainty", physicalConstantsTuple[2].convert<double>());

    return managedConstants;
}

pothos_static_block(registerSciPyConstants)
{
    Pothos::ManagedClass()
        .registerClass<SciPyPhysicalConstant>()
        .registerConstructor<SciPyPhysicalConstant>()
        .template registerField<SciPyPhysicalConstant, double>("Value", &SciPyPhysicalConstant::value)
        .template registerField<SciPyPhysicalConstant, std::string>("Unit", &SciPyPhysicalConstant::unit)
        .template registerField<SciPyPhysicalConstant, double>("Uncertainty", &SciPyPhysicalConstant::uncertainty)
        .commit(ManagedClassProxyPath);

    Pothos::PluginRegistry::addCall(
        "/scipy/constants/constants_dict",
        &getAllSciPyConstants);
    Pothos::PluginRegistry::addCall(
        "/scipy/constants/get_physical_constant",
        &getSciPyPhysicalConstant);
}
