# Slide-transcluded code listing

## Foldable length example

```cpp
const auto& foldable = smd::foldable_typeclass<IntTree>;
auto n = foldable.length(tree);
```

## Applicative invoke example

```cpp
using beman::optional::optional;
const auto& applicative = smd::applicative_typeclass<optional<int> >;

optional<int> ax = 1;
optional<int> ay = 2;
optional<int> az = 3;

auto sum = applicative.invoke(
    [](int a, int b, int c) { return a + b + c; },
    ax,
    ay,
    az);
```

## Traversable relabel example

```cpp
using beman::optional::optional;
const auto& traversable = smd::traversable_typeclass<IntTree>;

auto relabelled = traversable.traverse(
    [](int x) -> optional<int> {
        return x >= 0 ? optional<int>{x + 1} : optional<int>{};
    },
    tree);
```

## Bad Applicative example

```cpp
// Hypothetical: expands structure instead of preserving shape.
auto bad = cartesian_product(tx, ty);
```
