// Copyright (c) 2019-2021 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#include <Pothos/Callable.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Managed.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/Logger.h>

#include <cstdlib>
#include <limits>

//
// Wrappers around NumPy numeric info classes to expose nicer function names
//

// numpy.iinfo
template <typename T>
class IntInfo
{
    public:
        IntInfo(): _iinfo()
        {
            auto env = Pothos::ProxyEnvironment::make("python");
            auto pothosBuffer = env->findProxy("Pothos.Buffer");
            auto numpy = env->findProxy("numpy");

            _iinfo = numpy.call(
                         "iinfo",
                         pothosBuffer.call(
                             "dtype_to_numpy",
                             this->dtype()));
        }

        virtual ~IntInfo() = default;

        Pothos::DType dtype() const
        {
            static const Pothos::DType dtype(typeid(T));
            return dtype;
        }

        size_t getBits() const
        {
            return _iinfo.get<size_t>("bits");
        }

        T getMinValue() const
        {
            return _iinfo.get<T>("min");
        }

        T getMaxValue() const
        {
            return _iinfo.get<T>("max");
        }

    private:
        Pothos::Proxy _iinfo;
};

// numpy.finfo
template <typename T>
class FloatInfo
{
    public:
        FloatInfo(): _finfo()
        {
            auto env = Pothos::ProxyEnvironment::make("python");
            auto pothosBuffer = env->findProxy("Pothos.Buffer");
            auto numpy = env->findProxy("numpy");

            _finfo = numpy.call(
                         "finfo",
                         pothosBuffer.call(
                             "dtype_to_numpy",
                             this->dtype()));
        }

        virtual ~FloatInfo() = default;

        Pothos::DType dtype() const
        {
            static const Pothos::DType dtype(typeid(T));
            return dtype;
        }

        size_t getBits() const
        {
            return _finfo.get<size_t>("bits");
        }

        T getEpsilon() const
        {
            return _finfo.get<T>("eps");
        }

        T getNegativeEpsilon() const
        {
            return _finfo.get<T>("epsneg");
        }

        size_t getExponentBits() const
        {
            return _finfo.get<size_t>("iexp");
        }

        ssize_t getEpsilonExponent() const
        {
            return _finfo.get<ssize_t>("machep");
        }

        T getMinValue() const
        {
            return _finfo.get<T>("min");
        }

        ssize_t getMinExponent() const
        {
            return _finfo.get<ssize_t>("minexp");
        }

        T getMaxValue() const
        {
            return _finfo.get<T>("max");
        }

        size_t getMaxExponent() const
        {
            return _finfo.get<size_t>("maxexp");
        }

        ssize_t getNegativeEpsilonExponent() const
        {
            return _finfo.get<ssize_t>("negep");
        }

        size_t getMantissaBits() const
        {
            return _finfo.get<size_t>("nmant");
        }

        size_t getPrecision() const
        {
            return _finfo.get<size_t>("precision");
        }

        T getResolution() const
        {
            return _finfo.get<T>("resolution");
        }

        T getMinPositiveValue() const
        {
            return _finfo.get<T>("tiny");
        }

    private:
        Pothos::Proxy _finfo;
};

//
// Registering managed classes
//

template <typename T>
static Pothos::ManagedClass getIInfoManagedClass()
{
    using Class = IntInfo<T>;

    return Pothos::ManagedClass()
               .registerConstructor<Class>()
               .registerMethod(POTHOS_FCN_TUPLE(Class, dtype))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getBits))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getMinValue))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getMaxValue));
}

template <typename T>
static Pothos::ManagedClass getFInfoManagedClass()
{
    using Class = FloatInfo<T>;

    return Pothos::ManagedClass()
               .registerConstructor<Class>()
               .registerMethod(POTHOS_FCN_TUPLE(Class, dtype))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getBits))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getEpsilon))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getNegativeEpsilon))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getExponentBits))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getEpsilonExponent))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getMinValue))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getMinExponent))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getMaxValue))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getMaxExponent))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getNegativeEpsilonExponent))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getMantissaBits))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getPrecision))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getResolution))
               .registerMethod(POTHOS_FCN_TUPLE(Class, getMinPositiveValue));
}

pothos_static_block(registerNumericInfoClasses)
{
    getIInfoManagedClass<std::int8_t>().commit("NumPy/IntInfoInt8");
    getIInfoManagedClass<std::int16_t>().commit("NumPy/IntInfoInt16");
    getIInfoManagedClass<std::int32_t>().commit("NumPy/IntInfoInt32");
    getIInfoManagedClass<std::int64_t>().commit("NumPy/IntInfoInt64");
    getIInfoManagedClass<std::uint8_t>().commit("NumPy/IntInfoUInt8");
    getIInfoManagedClass<std::uint16_t>().commit("NumPy/IntInfoUInt16");
    getIInfoManagedClass<std::uint32_t>().commit("NumPy/IntInfoUInt32");
    getIInfoManagedClass<std::uint64_t>().commit("NumPy/IntInfoUInt64");

    getFInfoManagedClass<float>().commit("NumPy/FloatInfoFloat32");
    getFInfoManagedClass<double>().commit("NumPy/FloatInfoFloat64");
}
