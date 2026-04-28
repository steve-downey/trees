# Slide-transcluded code listing

## Foldable length example

```cpp
auto n = smd::length(tree);
```

## Applicative invoke example

```cpp
using beman::optional::optional;

optional<int> ax = 1;
optional<int> ay = 2;

auto sum = smd::invoke(
    [](int a, int b) { return a + b; },
    ax,
    ay);
```

## Traversable relabel example

```cpp
using beman::optional::optional;

auto relabelled = smd::traverse(
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
