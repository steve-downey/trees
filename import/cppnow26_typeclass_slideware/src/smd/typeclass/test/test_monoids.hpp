#ifndef INCLUDE_SMD_TYPECLASS_TEST_TEST_MONOIDS_HPP
#define INCLUDE_SMD_TYPECLASS_TEST_TEST_MONOIDS_HPP

#include <smd/typeclass/monoid.hpp>

#include <vector>

namespace smd::typeclass::test {

using smd::typeclass::Count;

template <class VALUE_TYPE>
using Vector = std::vector<VALUE_TYPE>;

}  // close namespace smd::typeclass::test

#endif  // INCLUDE_SMD_TYPECLASS_TEST_TEST_MONOIDS_HPP
