#ifndef INCLUDE_SMD_TYPECLASS_FUNCTOR_HPP
#define INCLUDE_SMD_TYPECLASS_FUNCTOR_HPP

#include <smd/typeclass/typeclass_base.hpp>

#include <beman/optional/optional.hpp>

#include <concepts>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

// Functor pattern invariants:
// - Generic entry point is fmap(F, T) and dispatches via functor_typeclass<T>.
// - Instances provide an object with fmap(F, T) and specialize
//   functor_typeclass<Concrete>.
// - replace is derived from fmap and should not need per-type overrides.
// - Keep lookup explicit through concept-map objects, not ADL overloads.

template <class T>
inline constexpr auto functor_typeclass = std::false_type{};

template <class FUNCTOR_MAP, class T, class F>
auto fmap(const FUNCTOR_MAP& functor_map, F&& function, T&& value)
{
    return functor_map.fmap(std::forward<F>(function),
                            std::forward<T>(value));
}

template <class T, class F>
auto fmap(F&& function, T&& value)
{
    using ValueType = remove_cvref_t<T>;
    return fmap(functor_typeclass<ValueType>,
                std::forward<F>(function),
                std::forward<T>(value));
}

template <class T, class U>
auto replace(T&& value, U&& replacement)
{
    return fmap(
        [replacement = std::forward<U>(replacement)](const auto&) mutable {
            return replacement;
        },
        std::forward<T>(value));
}

template <class VALUE_TYPE>
struct OptionalFunctorMap {
    template <class F>
    auto fmap(F&& function, const std::optional<VALUE_TYPE>& value) const
    {
        using Result = std::invoke_result_t<F, const VALUE_TYPE&>;
        if (!value) {
            return std::optional<remove_cvref_t<Result> >{};
        }
        return std::optional<remove_cvref_t<Result> >{
            std::invoke(std::forward<F>(function), *value)};
    }
};

template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>, std::optional<VALUE_TYPE> >)
struct BemanOptionalFunctorMap {
    template <class F>
    auto fmap(F&& function, const beman::optional::optional<VALUE_TYPE>& value) const
    {
        using Result = std::invoke_result_t<F, const VALUE_TYPE&>;
        if (!value) {
            return beman::optional::optional<remove_cvref_t<Result> >{};
        }
        return beman::optional::optional<remove_cvref_t<Result> >{
            std::invoke(std::forward<F>(function), *value)};
    }
};

template <class VALUE_TYPE>
struct VectorFunctorMap {
    template <class F>
    auto fmap(F&& function, const std::vector<VALUE_TYPE>& values) const
    {
        using Result = std::invoke_result_t<F, const VALUE_TYPE&>;
        std::vector<remove_cvref_t<Result> > output;
        output.reserve(values.size());

        for (const auto& value : values) {
            output.push_back(std::invoke(std::forward<F>(function), value));
        }

        return output;
    }
};

template <class VALUE_TYPE>
inline constexpr auto functor_typeclass<std::optional<VALUE_TYPE> > =
    OptionalFunctorMap<VALUE_TYPE>{};

template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>, std::optional<VALUE_TYPE> >)
inline constexpr auto functor_typeclass<beman::optional::optional<VALUE_TYPE> > =
    BemanOptionalFunctorMap<VALUE_TYPE>{};

template <class VALUE_TYPE>
inline constexpr auto functor_typeclass<std::vector<VALUE_TYPE> > =
    VectorFunctorMap<VALUE_TYPE>{};

}  // close namespace smd

#endif  // INCLUDE_SMD_TYPECLASS_FUNCTOR_HPP
