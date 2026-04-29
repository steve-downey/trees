#ifndef INCLUDE_SMD_TREE_FINGER_TREE_HPP
#define INCLUDE_SMD_TREE_FINGER_TREE_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace smd::tree {

template <typename T>
struct One {
  T a;
};

template <typename T>
struct Two {
  T a;
  T b;
};

template <typename T>
struct Three {
  T a;
  T b;
  T c;
};

template <typename T>
using Digit = std::variant<One<T>, Two<T>, Three<T>>;

template <typename T>
struct Node2 {
  T a;
  T b;
};

template <typename T>
struct Node3 {
  T a;
  T b;
  T c;
};

template <typename T>
using Node = std::variant<Node2<T>, Node3<T>>;

template <typename T>
inline auto digit_to_vector(const Digit<T>& d) -> std::vector<T>
{
  return std::visit(
    [](const auto& node) -> std::vector<T> {
      using N = std::decay_t<decltype(node)>;
      if constexpr (std::is_same_v<N, One<T>>) {
        return {node.a};
      } else if constexpr (std::is_same_v<N, Two<T>>) {
        return {node.a, node.b};
      } else {
        return {node.a, node.b, node.c};
      }
    },
    d);
}

template <typename T>
inline auto node_to_vector(const Node<T>& n) -> std::vector<T>
{
  return std::visit(
    [](const auto& node) -> std::vector<T> {
      using N = std::decay_t<decltype(node)>;
      if constexpr (std::is_same_v<N, Node2<T>>) {
        return {node.a, node.b};
      } else {
        return {node.a, node.b, node.c};
      }
    },
    n);
}

template <typename T>
class FingerTree {
  struct Empty {};

  struct Single {
    T d_value;
  };

  struct Deep {
    std::size_t d_measure;
    Digit<T> d_prefix;
    std::vector<Node<T>> d_middle;
    Digit<T> d_suffix;
  };

  std::variant<Empty, Single, Deep> d_data;

  static auto digit_size(const Digit<T>& digit) -> std::size_t
  {
    return std::visit(
      [](const auto& node) -> std::size_t {
        using N = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<N, One<T>>) {
          return 1U;
        }
        if constexpr (std::is_same_v<N, Two<T>>) {
          return 2U;
        }
        return 3U;
      },
      digit);
  }

  static auto node_size(const Node<T>& node) -> std::size_t
  {
    return std::visit(
      [](const auto& n) -> std::size_t {
        using N = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<N, Node2<T>>) {
          return 2U;
        }
        return 3U;
      },
      node);
  }

  static auto middle_size(const std::vector<Node<T>>& middle) -> std::size_t
  {
    std::size_t total = 0;
    for (const auto& n : middle) {
      total += node_size(n);
    }
    return total;
  }

  static auto to_tuples(std::vector<T> values) -> std::vector<Node<T>>
  {
    std::vector<Node<T>> nodes;
    if (values.size() < 2) {
      return nodes;
    }

    std::size_t i = 0;
    std::size_t remaining = values.size();

    // Avoid leaving a single trailing element by pre-splitting 4 -> 2 + 2.
    if (remaining % 3 == 1 && remaining >= 4) {
      nodes.push_back(Node2<T>{std::move(values[i]), std::move(values[i + 1])});
      i += 2;
      remaining -= 2;
      nodes.push_back(Node2<T>{std::move(values[i]), std::move(values[i + 1])});
      i += 2;
      remaining -= 2;
    } else if (remaining % 3 == 2) {
      nodes.push_back(Node2<T>{std::move(values[i]), std::move(values[i + 1])});
      i += 2;
      remaining -= 2;
    }

    while (remaining >= 3) {
      nodes.push_back(
        Node3<T>{std::move(values[i]), std::move(values[i + 1]), std::move(values[i + 2])});
      i += 3;
      remaining -= 3;
    }

    if (remaining == 2) {
      nodes.push_back(Node2<T>{std::move(values[i]), std::move(values[i + 1])});
    }

    return nodes;
  }

  auto make_single(T value) const -> FingerTree
  {
    return FingerTree(Single{std::move(value)});
  }

  auto make_deep(Digit<T> prefix, std::vector<Node<T>> middle, Digit<T> suffix) const
    -> FingerTree
  {
    const auto total = digit_size(prefix) + middle_size(middle) + digit_size(suffix);
    return FingerTree(
      Deep{total, std::move(prefix), std::move(middle), std::move(suffix)});
  }

 public:
  struct View {
    T d_value;
    FingerTree d_rest;
  };

  static auto empty() -> FingerTree { return FingerTree(Empty{}); }

  static auto leaf(T value) -> FingerTree
  {
    FingerTree builder(Empty{});
    return builder.make_single(std::move(value));
  }

  auto cons(T x) const -> FingerTree
  {
    return std::visit(
      [this, &x](const auto& node) -> FingerTree {
        using N = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<N, Empty>) {
          return make_single(std::move(x));
        } else if constexpr (std::is_same_v<N, Single>) {
          return make_deep(
            One<T>{std::move(x)}, std::vector<Node<T>>{}, One<T>{node.d_value});
        } else {
          return std::visit(
            [this, &x, &node](const auto& prefix) -> FingerTree {
              using P = std::decay_t<decltype(prefix)>;
              if constexpr (std::is_same_v<P, One<T>>) {
                return make_deep(
                  Two<T>{std::move(x), prefix.a}, node.d_middle, node.d_suffix);
              } else if constexpr (std::is_same_v<P, Two<T>>) {
                return make_deep(
                  Three<T>{std::move(x), prefix.a, prefix.b}, node.d_middle, node.d_suffix);
              } else {
                auto new_middle = node.d_middle;
                new_middle.insert(new_middle.begin(), Node2<T>{prefix.b, prefix.c});
                return make_deep(
                  Two<T>{std::move(x), prefix.a}, std::move(new_middle), node.d_suffix);
              }
            },
            node.d_prefix);
        }
      },
      d_data);
  }

  auto snoc(T x) const -> FingerTree
  {
    return std::visit(
      [this, &x](const auto& node) -> FingerTree {
        using N = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<N, Empty>) {
          return make_single(std::move(x));
        } else if constexpr (std::is_same_v<N, Single>) {
          return make_deep(
            One<T>{node.d_value}, std::vector<Node<T>>{}, One<T>{std::move(x)});
        } else {
          return std::visit(
            [this, &x, &node](const auto& suffix) -> FingerTree {
              using S = std::decay_t<decltype(suffix)>;
              if constexpr (std::is_same_v<S, One<T>>) {
                return make_deep(
                  node.d_prefix, node.d_middle, Two<T>{suffix.a, std::move(x)});
              } else if constexpr (std::is_same_v<S, Two<T>>) {
                return make_deep(
                  node.d_prefix, node.d_middle, Three<T>{suffix.a, suffix.b, std::move(x)});
              } else {
                auto new_middle = node.d_middle;
                new_middle.push_back(Node2<T>{suffix.b, suffix.c});
                return make_deep(
                  node.d_prefix, std::move(new_middle), Two<T>{suffix.a, std::move(x)});
              }
            },
            node.d_suffix);
        }
      },
      d_data);
  }

  auto append(const FingerTree& right) const -> FingerTree
  {
    return std::visit(
      [this, &right](const auto& left_node) -> FingerTree {
        using L = std::decay_t<decltype(left_node)>;
        if constexpr (std::is_same_v<L, Empty>) {
          return right;
        } else if constexpr (std::is_same_v<L, Single>) {
          return right.cons(left_node.d_value);
        } else {
          return std::visit(
            [this, &left_node](const auto& right_node) -> FingerTree {
              using R = std::decay_t<decltype(right_node)>;
              if constexpr (std::is_same_v<R, Empty>) {
                return FingerTree(left_node);
              } else if constexpr (std::is_same_v<R, Single>) {
                return this->snoc(right_node.d_value);
              } else {
                auto bridge = digit_to_vector(left_node.d_suffix);
                auto right_prefix = digit_to_vector(right_node.d_prefix);
                bridge.insert(
                  bridge.end(),
                  std::make_move_iterator(right_prefix.begin()),
                  std::make_move_iterator(right_prefix.end()));

                auto bridge_nodes = to_tuples(std::move(bridge));

                auto new_middle = left_node.d_middle;
                new_middle.insert(
                  new_middle.end(),
                  std::make_move_iterator(bridge_nodes.begin()),
                  std::make_move_iterator(bridge_nodes.end()));
                new_middle.insert(new_middle.end(),
                                  right_node.d_middle.begin(),
                                  right_node.d_middle.end());

                return make_deep(left_node.d_prefix,
                                 std::move(new_middle),
                                 right_node.d_suffix);
              }
            },
            right.d_data);
        }
      },
      d_data);
  }

  static auto branch(const FingerTree& left, const FingerTree& right) -> FingerTree
  {
    return left.append(right);
  }

  static auto prepend(T value, const FingerTree& tree) -> FingerTree
  {
    return tree.cons(std::move(value));
  }

  static auto append(const FingerTree& tree, T value) -> FingerTree
  {
    return tree.snoc(std::move(value));
  }

  static auto concat(const FingerTree& left, const FingerTree& right) -> FingerTree
  {
    return left.append(right);
  }

  auto is_empty() const -> bool { return std::holds_alternative<Empty>(d_data); }
  auto is_leaf() const -> bool { return std::holds_alternative<Single>(d_data); }
  auto is_branch() const -> bool { return std::holds_alternative<Deep>(d_data); }

  auto measure() const -> std::size_t
  {
    return std::visit(
      [](const auto& node) -> std::size_t {
        using N = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<N, Empty>) {
          return 0U;
        } else if constexpr (std::is_same_v<N, Single>) {
          return 1U;
        } else {
          return node.d_measure;
        }
      },
      d_data);
  }

  auto breadth() const -> std::size_t { return measure(); }

  auto depth() const -> std::size_t
  {
    return std::visit(
      [](const auto& node) -> std::size_t {
        using N = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<N, Empty>) {
          return 0U;
        } else if constexpr (std::is_same_v<N, Single>) {
          return 1U;
        } else {
          return node.d_middle.empty() ? 1U : 2U;
        }
      },
      d_data);
  }

  auto value() const -> const T&
  {
    assert(is_leaf());
    return std::get<Single>(d_data).d_value;
  }

  auto flatten() const -> std::vector<T>
  {
    return std::visit(
      [](const auto& node) -> std::vector<T> {
        using N = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<N, Empty>) {
          return {};
        } else if constexpr (std::is_same_v<N, Single>) {
          return {node.d_value};
        } else {
          auto result = digit_to_vector(node.d_prefix);
          for (const auto& middle_node : node.d_middle) {
            auto expanded = node_to_vector(middle_node);
            result.insert(result.end(), expanded.begin(), expanded.end());
          }
          auto suffix = digit_to_vector(node.d_suffix);
          result.insert(result.end(), suffix.begin(), suffix.end());
          return result;
        }
      },
      d_data);
  }

  static auto from_sequence(std::vector<T> values) -> FingerTree
  {
    if (values.empty()) {
      return empty();
    }
    if (values.size() == 1) {
      return leaf(std::move(values.front()));
    }

    const auto n = values.size();
    const std::size_t prefix_sz = std::min<std::size_t>(3, std::max<std::size_t>(1, n / 2));
    const std::size_t suffix_sz = std::min<std::size_t>(3, n - prefix_sz);

    std::vector<T> prefix_v(values.begin(), values.begin() + prefix_sz);
    std::vector<T> suffix_v(values.end() - suffix_sz, values.end());

    std::vector<T> middle_v;
    if (prefix_sz + suffix_sz < n) {
      middle_v.assign(values.begin() + prefix_sz, values.end() - suffix_sz);
    }

    std::vector<Node<T>> middle_nodes;
    for (std::size_t i = 0; i < middle_v.size();) {
      const auto remaining = middle_v.size() - i;
      if (remaining >= 3) {
        middle_nodes.push_back(Node3<T>{
          std::move(middle_v[i]), std::move(middle_v[i + 1]), std::move(middle_v[i + 2])});
        i += 3;
      } else if (remaining == 2) {
        middle_nodes.push_back(Node2<T>{std::move(middle_v[i]), std::move(middle_v[i + 1])});
        i += 2;
      } else {
        suffix_v.insert(suffix_v.begin(), std::move(middle_v[i]));
        i += 1;
      }
    }

    if (suffix_v.size() == 4) {
      middle_nodes.push_back(Node2<T>{std::move(suffix_v[0]), std::move(suffix_v[1])});
      suffix_v.erase(suffix_v.begin(), suffix_v.begin() + 2);
    }

    Digit<T> prefix;
    if (prefix_v.size() == 1) {
      prefix = One<T>{std::move(prefix_v[0])};
    } else if (prefix_v.size() == 2) {
      prefix = Two<T>{std::move(prefix_v[0]), std::move(prefix_v[1])};
    } else {
      prefix = Three<T>{std::move(prefix_v[0]), std::move(prefix_v[1]), std::move(prefix_v[2])};
    }

    Digit<T> suffix;
    if (suffix_v.size() == 1) {
      suffix = One<T>{std::move(suffix_v[0])};
    } else if (suffix_v.size() == 2) {
      suffix = Two<T>{std::move(suffix_v[0]), std::move(suffix_v[1])};
    } else {
      suffix = Three<T>{std::move(suffix_v[0]), std::move(suffix_v[1]), std::move(suffix_v[2])};
    }

    FingerTree builder(Empty{});
    return builder.make_deep(std::move(prefix), std::move(middle_nodes), std::move(suffix));
  }

  auto view_l() const -> std::optional<View>
  {
    auto values = flatten();
    if (values.empty()) {
      return std::nullopt;
    }
    auto first = std::move(values.front());
    values.erase(values.begin());
    return View{std::move(first), from_sequence(std::move(values))};
  }

  auto view_r() const -> std::optional<View>
  {
    auto values = flatten();
    if (values.empty()) {
      return std::nullopt;
    }
    auto last_value = std::move(values.back());
    values.pop_back();
    return View{std::move(last_value), from_sequence(std::move(values))};
  }

  auto head() const -> T
  {
    auto v = view_l();
    assert(v.has_value());
    return std::move(v->d_value);
  }

  auto tail() const -> FingerTree
  {
    auto v = view_l();
    return v.has_value() ? std::move(v->d_rest) : empty();
  }

  auto last() const -> T
  {
    auto v = view_r();
    assert(v.has_value());
    return std::move(v->d_value);
  }

  auto init() const -> FingerTree
  {
    auto v = view_r();
    return v.has_value() ? std::move(v->d_rest) : empty();
  }

 private:
  explicit FingerTree(Empty e)
      : d_data(std::move(e))
  {
  }

  explicit FingerTree(Single s)
      : d_data(std::move(s))
  {
  }

  explicit FingerTree(Deep d)
      : d_data(std::move(d))
  {
  }
};

}  // namespace smd::tree

#endif
