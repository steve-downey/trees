#include <smd/typeclass/examples/examples.hpp>

#include <smd/typeclass/functor.hpp>

#include <optional>
#include <type_traits>
#include <utility>

namespace smd::typeclass::examples {

struct OptionalFunctorObject {
    template <class F, class T>
    auto fmap(F&& function, const std::optional<T>& value) const
    {
        return smd::fmap(smd::functor_typeclass<std::optional<T> >,
                         std::forward<F>(function),
                         value);
    }
};

template <class CONTEXT>
inline constexpr auto functor_typeclass = std::false_type{};

template <class T>
inline constexpr auto functor_typeclass<std::optional<T> > =
    OptionalFunctorObject{};

template <class CONTEXT, const auto& FUNCTOR = functor_typeclass<CONTEXT> >
auto fmap_plus_one_nttp(const CONTEXT& value)
{
    return FUNCTOR.fmap([](int x) { return x + 1; }, value);
}

auto explicit_object_lookup_example() -> std::optional<int>
{
    OptionalFunctorObject functor;
    std::optional<int> value{41};

    return functor.fmap([](int x) { return x + 1; }, value);
}

auto nttp_object_lookup_example() -> std::optional<int>
{
    std::optional<int> value{9};
    return fmap_plus_one_nttp(value);
}

}  // close namespace smd::typeclass::examples
