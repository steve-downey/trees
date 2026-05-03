# Finger Tree Compile Regression Notes

This note exists so future work on `src/smd/tree/finger_tree.hpp` does not repeat the same compile-time failure mode.

## Problem Observed

A compatibility-oriented finger tree implementation regressed badly under both Clang 23 and GCC 16 when the implementation recursively instantiated `FingerTree<Node<T, Tag>>` and then odr-used operations that walk the structure through `flatten()`.

The important boundary was not header parse, `leaf()`, or custom measure tags by themselves.

The compile cliff began once a translation unit odr-used any of these operations:

- `flatten()`
- `measure()`
- `search()`
- `split()`
- `split_at()`
- `split_at_measure()`

These all shared the same recursive compile-time expansion path.

## Probe Findings

Using isolated probe translation units under an 8 GB virtual-memory cap:

- Header-only and `leaf()` probes compiled quickly on both Clang 23 and GCC 16.
- Weighted tag probes were also cheap until they touched the flatten-based path.
- `flatten()` alone was enough to trigger the pathological compile path.
- `measure()`, `search()`, and `split()` inherited the same cliff because they depended on `flatten()`.

Before the fallback rewrite:

- Clang 23 could finish the combined probe, but took about 28 minutes and about 6.3 GB RSS.
- GCC 16 exhausted the 8 GB cap after about 6 minutes and about 7.9 GB RSS.

## Temporary Fallback

A flat persistent-vector backend was able to preserve the public API surface while collapsing compile time back to seconds. That fallback is useful as an emergency compatibility option, but it is not a true finger tree and should not be treated as the long-term design.

## Guidance For The Next Structural Pass

If a true finger tree is required, avoid reintroducing unbounded recursive compile-time expansion through `FingerTree<Node<...>>` on hot inline paths.

In particular:

- treat `flatten()` as a compile-time hotspot, not just a runtime helper
- be careful with recursive `Node`/`Digit` instantiation visible in headers
- validate compile cost with tiny probe translation units before scaling back up to `finger_tree.t.cpp`
- check both Clang and GCC early, because GCC 16 failed much earlier on memory

## Practical Rule

Any candidate structural rewrite should be checked first with a minimal ladder of probes:

- header only
- `leaf()`
- `flatten()`
- `measure()`
- `search()`
- `split()`

If `flatten()` regresses again, the rest of the API will likely follow.
