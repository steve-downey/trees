#ifndef INCLUDE_SMD_TYPECLASS_TRAVERSABLE_HPP
#define INCLUDE_SMD_TYPECLASS_TRAVERSABLE_HPP

#include <smd/typeclass/functor.hpp>
#include <smd/typeclass/typeclass_base.hpp>

#include <type_traits>
#include <utility>

namespace smd {

struct traversable_tag {};

template <class T, class F>
auto traverse(F&& function, T&& value)
{
    using ValueType = remove_cvref_t<T>;
    return map<traversable_tag, ValueType>::traverse(std::forward<F>(function),
                                                     std::forward<T>(value));
}

template <class T>
auto sequence(T&& value)
{
    return traverse([](auto&& x) { return std::forward<decltype(x)>(x); },
                    std::forward<T>(value));
}

}  // close namespace smd

#endif
