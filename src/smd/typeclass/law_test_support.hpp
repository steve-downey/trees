#ifndef INCLUDE_SMD_TYPECLASS_LAW_TEST_SUPPORT_HPP
#define INCLUDE_SMD_TYPECLASS_LAW_TEST_SUPPORT_HPP

#include <utility>

namespace smd::typeclass::test {

template <class LEFT, class RIGHT>
auto are_equal(LEFT&& left, RIGHT&& right) -> bool
{
    return std::forward<LEFT>(left) == std::forward<RIGHT>(right);
}

}  // close namespace smd::typeclass::test

#endif  // INCLUDE_SMD_TYPECLASS_LAW_TEST_SUPPORT_HPP
