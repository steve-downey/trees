#ifndef INCLUDE_SMD_TREE_FINGER_TREE_INTERVAL_INDEX_TRAVERSABLE_HPP
#define INCLUDE_SMD_TREE_FINGER_TREE_INTERVAL_INDEX_TRAVERSABLE_HPP

#include <smd/tree/finger_tree_interval_index.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexTraversableImpl {
  template <class F>
  auto traverse(this auto&&,
                F&& function,
                const smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE>& index)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const PAYLOAD_TYPE&>>;
    const auto& applicative = smd::applicative_typeclass<Context>;
    using U = smd::applicative_value_t<Context>;

    auto accumulated = applicative.pure(std::vector<smd::tree::Interval<U>>{});

    for (const auto& entry : index.entries()) {
      auto lifted = std::invoke(function, entry.d_payload);
      accumulated = applicative.invoke(
        [start = entry.d_start, end = entry.d_end](std::vector<smd::tree::Interval<U>> values,
                                                    U payload) {
          values.push_back(
            smd::tree::Interval<U>{start, end, std::move(payload)});
          return values;
        },
        std::move(accumulated),
        std::move(lifted));
    }

    return applicative.invoke(
      [](std::vector<smd::tree::Interval<U>> values) {
        return smd::tree::FingerTreeIntervalIndex<U>::from_intervals(
          std::move(values));
      },
      std::move(accumulated));
  }
};

template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexTraversableMap
  : Traversable<FingerTreeIntervalIndexTraversableImpl<PAYLOAD_TYPE>> {
  using FingerTreeIntervalIndexTraversableImpl<PAYLOAD_TYPE>::traverse;
};

template <class PAYLOAD_TYPE>
inline constexpr auto traversable_typeclass<
  smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE>> =
  FingerTreeIntervalIndexTraversableMap<PAYLOAD_TYPE>{};

}  // namespace smd

#endif
