#include <Pothos/Callable.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

static Pothos::Object FactoryFunc(
    const Pothos::Object *args,
    const size_t numArgs,
    const std::string& pythonName)
{
    // Create Python environment.
    auto env = Pothos::ProxyEnvironment::make("python");

    // Convert arguments for proxy.
    std::vector<Pothos::Proxy> proxyArgs(numArgs);
    for (size_t i = 0; i < numArgs; i++)
    {
        proxyArgs[i] = env->makeProxy(args[i]);
    }

    // Load this Pothos module's Python module.
    auto mod = env->findProxy("PothosNumPy");

    // Call into the factory to return the block.
    auto block = mod.getHandle()->call(pythonName, proxyArgs.data(), proxyArgs.size());
    return Pothos::Object(block);
}

static const std::vector<Pothos::BlockRegistry> blockRegistries =
{
%for factory in factories:
    ${factory},
%endfor
};

pothos_static_block(register_pothos_numpy_docs)
{
%for doc in docs:
    ${doc}
%endfor
}
