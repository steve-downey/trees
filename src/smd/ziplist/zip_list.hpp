// src/smd/ziplist/zip_list.hpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_ZIPLIST_ZIP_LIST
#define INCLUDED_SMD_ZIPLIST_ZIP_LIST

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace smd {

/** List with positional (zip) applicative semantics that can represent an
 * infinite repetition of a single value.
 *
 * A zip_list is either finite (elements stored in @c data) or infinite
 * (a single @c repeated value that logically occupies every position). The
 * ZipList applicative applies functions positionally and truncates to the
 * shortest finite operand; pure(x) yields the infinite repetition of x so
 * that it acts as an identity for truncation.
 *
 * Invariant: when @c repeated has a value, @c data is ignored and the
 * zip_list models an infinite repetition of @c repeated.
 * @tparam T element type
 */
template <class T>
struct zip_list {
    using value_type = T;

    // Invariant: when repeated has a value, this zip_list models an infinite
    // repetition of that value and data is ignored.
    std::vector<T> data;
    std::optional<T> repeated{};

    /** Construct an infinite zip_list repeating @p value at every position. */
    static auto repeat(T value) -> zip_list {
        return zip_list{{}, std::move(value)};
    }

    /** True when this zip_list represents an infinite repetition. */
    auto is_repeating() const -> bool { return repeated.has_value(); }

    /** Number of elements in the finite representation; 0 for infinite lists.
     */
    auto finite_size() const -> std::size_t { return data.size(); }

    /** Equality: two infinite lists are equal iff they repeat the same value;
     * two finite lists use element-wise comparison; mixed always false.
     */
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
