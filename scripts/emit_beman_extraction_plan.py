#!/usr/bin/env python3
"""Emit the current reference Beman layout map.

This script is intentionally simple.
It is a review aid for the reference implementation shape, not an automated mover.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass


@dataclass(frozen=True)
class Mapping:
    source: str
    category: str
    target: str
    note: str


MAPPINGS = [
    Mapping(
        "src/smd/typeclass/traversable.hpp",
        "public-api",
        "include/beman/structure/traverse.hpp",
        "Rename toward the operation users reach for.",
    ),
    Mapping(
        "src/smd/typeclass/traversable.hpp",
        "public-api",
        "include/beman/structure/transpose.hpp",
        "Expose the structure/context flipping operation with a verb-first name.",
    ),
    Mapping(
        "src/smd/typeclass/foldable.hpp",
        "public-api",
        "include/beman/structure/fold.hpp",
        "Working preferred public filename.",
    ),
    Mapping(
        "src/smd/typeclass/monoid.hpp",
        "public-support",
        "include/beman/structure/monoid.hpp",
        "Public support concept for fold and traverse-oriented facilities.",
    ),
    Mapping(
        "src/smd/typeclass/applicative.hpp",
        "public-support",
        "include/beman/structure/apply.hpp",
        "Verb-first support header for the reference implementation.",
    ),
    Mapping(
        "src/smd/typeclass/functor.hpp",
        "public-support",
        "include/beman/structure/functor.hpp",
        "Companion support surface in the full reference layout.",
    ),
    Mapping(
        "src/smd/typeclass/monad.hpp",
        "public-support",
        "include/beman/structure/monad.hpp",
        "Companion support surface in the full reference layout.",
    ),
    Mapping(
        "src/smd/typeclass/dual_monoid.hpp",
        "public-support",
        "include/beman/structure/dual_monoid.hpp",
        "Advanced support header that still belongs in the reference map.",
    ),
    Mapping(
        "src/smd/typeclass/typeclass_base.hpp",
        "detail",
        "include/beman/structure/detail/typeclass_base.hpp",
        "Foundational machinery, not a front-door header.",
    ),
    Mapping(
        "src/smd/tree/finger_tree5.hpp",
        "public-api",
        "include/beman/structure/finger_tree.hpp",
        "Drop version suffix from public API.",
    ),
    Mapping(
        "src/smd/tree/finger_tree_random_access.hpp",
        "public-api",
        "include/beman/structure/random_access.hpp",
        "Capability-facing public name.",
    ),
    Mapping(
        "src/smd/tree/finger_tree_rope.hpp",
        "public-api",
        "include/beman/structure/rope.hpp",
        "Capability-facing public name.",
    ),
    Mapping(
        "src/smd/tree/finger_tree_priority_queue.hpp",
        "public-api",
        "include/beman/structure/priority_queue.hpp",
        "Capability-facing public name.",
    ),
    Mapping(
        "src/smd/tree/finger_tree_interval_index.hpp",
        "public-api",
        "include/beman/structure/interval_index.hpp",
        "Capability-facing public name.",
    ),
    Mapping(
        "src/smd/tree/binary_tree.hpp",
        "public-api",
        "include/beman/structure/binary_tree.hpp",
        "Supporting tree surface in the full reference layout.",
    ),
    Mapping(
        "src/smd/tree/fringe_tree.hpp",
        "public-api",
        "include/beman/structure/fringe_tree.hpp",
        "Supporting tree surface in the full reference layout.",
    ),
    Mapping(
        "src/smd/tree/finger_tree_wrappers.hpp",
        "public-convenience",
        "include/beman/structure/finger_tree_wrappers.hpp",
        "Optional umbrella include that still belongs in the reference map.",
    ),
    Mapping(
        "src/smd/tree/fixpoint_tree.hpp",
        "source-material",
        "include/beman/structure/recursive_fold.hpp",
        "Primary source material for the recursive fold surface.",
    ),
    Mapping(
        "src/smd/tree/fixpoint_tree.hpp",
        "source-material",
        "include/beman/structure/recursive_build.hpp",
        "Primary source material for the recursive build surface.",
    ),
    Mapping(
        "src/smd/tree/finger_tree5_iterator.hpp",
        "detail",
        "include/beman/structure/detail/finger_tree_iterator.hpp",
        "Large representation-coupled support header.",
    ),
    Mapping(
        "src/smd/typeclass/traversable.t.cpp",
        "test",
        "tests/beman/structure/traverse.test.cpp",
        "Traverse behavior test for the reference surface.",
    ),
    Mapping(
        "src/smd/typeclass/traversable.t.cpp",
        "test",
        "tests/beman/structure/transpose.test.cpp",
        "Explicit coverage for the public transpose operation.",
    ),
    Mapping(
        "src/smd/typeclass/applicative.t.cpp",
        "test",
        "tests/beman/structure/apply.test.cpp",
        "Apply behavior test for the reference support surface.",
    ),
    Mapping(
        "src/smd/typeclass/foldable.t.cpp",
        "test",
        "tests/beman/structure/fold.test.cpp",
        "Fold behavior test for the reference surface.",
    ),
    Mapping(
        "src/smd/typeclass/monoid.t.cpp",
        "test",
        "tests/beman/structure/monoid.test.cpp",
        "Monoid behavior test for the reference support surface.",
    ),
    Mapping(
        "src/smd/typeclass/functor.t.cpp",
        "test",
        "tests/beman/structure/functor.test.cpp",
        "Functor behavior test for the reference support surface.",
    ),
    Mapping(
        "src/smd/typeclass/monad.t.cpp",
        "test",
        "tests/beman/structure/monad.test.cpp",
        "Monad behavior test for the reference support surface.",
    ),
    Mapping(
        "src/smd/typeclass/dual_monoid.t.cpp",
        "test",
        "tests/beman/structure/dual_monoid.test.cpp",
        "Dual-monoid behavior test for the reference support surface.",
    ),
    Mapping(
        "src/smd/typeclass/typeclass_base.t.cpp",
        "test",
        "tests/beman/structure/detail/typeclass_base.test.cpp",
        "Detail support still requires tests.",
    ),
    Mapping(
        "src/smd/tree/finger_tree5.t.cpp",
        "test",
        "tests/beman/structure/finger_tree.test.cpp",
        "Primary finger-tree behavior test.",
    ),
    Mapping(
        "src/smd/tree/finger_tree5_iterator.t.cpp",
        "test",
        "tests/beman/structure/detail/finger_tree_iterator.test.cpp",
        "Detail iterator support still requires tests.",
    ),
    Mapping(
        "src/smd/tree/finger_tree_random_access.t.cpp",
        "test",
        "tests/beman/structure/random_access.test.cpp",
        "Capability wrapper test.",
    ),
    Mapping(
        "src/smd/tree/finger_tree_rope.t.cpp",
        "test",
        "tests/beman/structure/rope.test.cpp",
        "Capability wrapper test.",
    ),
    Mapping(
        "src/smd/tree/finger_tree_priority_queue.t.cpp",
        "test",
        "tests/beman/structure/priority_queue.test.cpp",
        "Capability wrapper test.",
    ),
    Mapping(
        "src/smd/tree/finger_tree_interval_index.t.cpp",
        "test",
        "tests/beman/structure/interval_index.test.cpp",
        "Capability wrapper test.",
    ),
    Mapping(
        "src/smd/tree/binary_tree.t.cpp",
        "test",
        "tests/beman/structure/binary_tree.test.cpp",
        "Supporting tree behavior test.",
    ),
    Mapping(
        "src/smd/tree/fringe_tree.t.cpp",
        "test",
        "tests/beman/structure/fringe_tree.test.cpp",
        "Supporting tree behavior test.",
    ),
    Mapping(
        "src/smd/tree/finger_tree_wrappers.t.cpp",
        "test",
        "tests/beman/structure/finger_tree_wrappers.test.cpp",
        "Umbrella include and wrapper smoke test.",
    ),
    Mapping(
        "src/smd/typeclass/examples/applicative_examples.cpp",
        "example",
        "examples/apply_example.cpp",
        "Reference example for the verb-first apply support surface.",
    ),
    Mapping(
        "src/smd/typeclass/examples/traversable_examples.cpp",
        "example",
        "examples/traverse_example.cpp",
        "Reference example for the public traverse surface.",
    ),
    Mapping(
        "src/smd/typeclass/examples/traversable_examples.cpp",
        "example",
        "examples/transpose_example.cpp",
        "Expose the structure/context flipping operation under its public-facing name.",
    ),
    Mapping(
        "src/smd/typeclass/examples/foldable_examples.cpp",
        "example",
        "examples/fold_example.cpp",
        "Reference example for the public fold surface.",
    ),
    Mapping(
        "src/examples/fixpoint_tree_example.cpp",
        "source-material",
        "examples/recursive_fold_example.cpp",
        "Seed example for the recursive fold surface once the split is finalized.",
    ),
    Mapping(
        "src/smd/typeclass/examples/blog_fixpoint_examples.cpp",
        "stay-in-trees",
        "",
        "Blog and pedagogy asset.",
    ),
    Mapping(
        "src/smd/typeclass/examples/blog_typeclass_examples.cpp",
        "stay-in-trees",
        "",
        "Blog and pedagogy asset.",
    ),
    Mapping(
        "src/smd/typeclass/examples/lookup_modes_examples.cpp",
        "stay-in-trees",
        "",
        "Pedagogy and design explanation asset.",
    ),
    Mapping(
        "src/examples/cpo_example.cpp",
        "stay-in-trees",
        "",
        "Local demonstration asset.",
    ),
    Mapping(
        "src/examples/map_example.cpp",
        "stay-in-trees",
        "",
        "Local demonstration asset.",
    ),
    Mapping(
        "src/examples/main.cpp",
        "stay-in-trees",
        "",
        "Local demo harness, not a Beman example.",
    ),
    Mapping(
        "src/smd/tree/finger_tree2.hpp",
        "stay-in-trees",
        "",
        "Implementation-history and comparison artifact.",
    ),
    Mapping(
        "src/smd/tree/finger_tree3.hpp",
        "stay-in-trees",
        "",
        "Implementation-history and comparison artifact.",
    ),
    Mapping(
        "src/smd/tree/finger_tree4.hpp",
        "stay-in-trees",
        "",
        "Implementation-history and comparison artifact.",
    ),
    Mapping(
        "src/smd/tree/deadcode/",
        "stay-in-trees",
        "",
        "Pedagogy and archaeology only.",
    ),
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Emit the reference Beman layout mapping."
    )
    parser.add_argument(
        "--format",
        choices=("table", "tsv"),
        default="tsv",
        help="Output format. Default: tsv",
    )
    return parser.parse_args()


def emit_tsv(rows: list[tuple[str, str, str, str]]) -> None:
    print("\t".join(("CATEGORY", "SOURCE", "TARGET", "NOTE")))
    for row in rows:
        print("\t".join(row))


def emit_table(rows: list[tuple[str, str, str, str]]) -> None:
    headers = (
        "CATEGORY",
        "SOURCE",
        "TARGET",
        "NOTE",
    )
    widths = [len(h) for h in headers]
    for row in rows:
        widths = [max(width, len(value)) for width, value in zip(widths, row)]

    def render_row(values: tuple[str, str, str, str]) -> str:
        return " | ".join(
            value.ljust(width) for value, width in zip(values, widths)
        )

    print(render_row(headers))
    print("-+-".join("-" * width for width in widths))
    for row in rows:
        print(render_row(row))


def main() -> None:
    args = parse_args()
    rows = [(m.category, m.source, m.target or "-", m.note) for m in MAPPINGS]
    if args.format == "table":
        emit_table(rows)
        return
    emit_tsv(rows)


if __name__ == "__main__":
    main()
