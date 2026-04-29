#include <smd/typeclass/examples/examples.hpp>

#include <smd/typeclass/applicative.hpp>

#include <beman/optional/optional.hpp>

namespace smd::typeclass::examples {

auto applicative_invoke_example() -> beman::optional::optional<int>
{
    using beman::optional::optional;
    const auto& applicative = smd::applicative_typeclass<optional<int> >;

    optional<int> ax = 1;
    optional<int> ay = 2;

    // 3f0c8d0e-9a6b-4a3e-9c2a-0c1e9c3d4f11
    auto sum = applicative.invoke(
        [](int a, int b) { return a + b; },
        ax,
        ay);
    // 3f0c8d0e-9a6b-4a3e-9c2a-0c1e9c3d4f11 end

    return sum;
}

}  // close namespace smd::typeclass::examples
