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

struct functor_tag {};

template <class T, class F>
auto fmap(F&& function, T&& value)
{
    using ValueType = remove_cvref_t<T>;
    return map<functor_tag, ValueType>::fmap(std::forward<F>(function),
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
struct map<functor_tag, std::optional<VALUE_TYPE> > {
    template <class F>
    static auto fmap(F&& function, const std::optional<VALUE_TYPE>& value)
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
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                                                 std::optional<VALUE_TYPE> >)
struct map<functor_tag, beman::optional::optional<VALUE_TYPE> > {
    template <class F>
    static auto fmap(F&& function,
                     const beman::optional::optional<VALUE_TYPE>& value)
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
struct map<functor_tag, std::vector<VALUE_TYPE> > {
    template <class F>
    static auto fmap(F&& function, const std::vector<VALUE_TYPE>& values)
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

}  // close namespace smd

#endif  // INCLUDE_SMD_TYPECLASS_FUNCTOR_HPP
