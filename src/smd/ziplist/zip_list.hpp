// src/smd/ziplist/zip_list.hpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_ZIPLIST_ZIP_LIST
#define INCLUDED_SMD_ZIPLIST_ZIP_LIST

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace smd {

template <class T>
struct zip_list {
    using value_type = T;

    // Invariant: when repeated has a value, this zip_list models an infinite
    // repetition of that value and data is ignored.
    std::vector<T> data;
    std::optional<T> repeated{};

    static auto repeat(T value) -> zip_list {
        return zip_list{{}, std::move(value)};
    }

    auto is_repeating() const -> bool { return repeated.has_value(); }

    auto finite_size() const -> std::size_t { return data.size(); }

    friend auto operator==(const zip_list &left, const zip_list &right)
        -> bool {
        if (left.is_repeating() || right.is_repeating()) {
            return left.repeated == right.repeated;
        }
        return left.data == right.data;
    }
};

} // namespace smd

#endif
