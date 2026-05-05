---
title: Live Source Snapshot (main)
summary: Point-in-time fenced dump of live trees/src C++ sources from main, excluding deadcode and smd/conceptmap.
source_of_truth: git HEAD on branch main
scope:
  include:
    - trees/src/**/*.hpp
    - trees/src/**/*.h
    - trees/src/**/*.cpp
  exclude:
    - trees/src/deadcode/**
    - trees/src/smd/conceptmap/**
    - trees/src/**/CMakeLists.txt
update_policy:
  when_to_update:
    - Any time live files under trees/src are added, removed, renamed, or materially changed on main.
    - Before using this file as a review/reference baseline.
  how_to_update:
    - Regenerate from main using the command block in the "Regeneration" section below.
    - Replace this file atomically with regenerated output.
notes:
  - Section headers are canonical paths without the leading src/ prefix.
  - File contents are copied from git (HEAD), not the working tree.
---

# Live Source Snapshot (main)

Generated from main at commit e5a625d1.

Includes files under trees/src that are live in current targets and examples, excluding deadcode and smd/conceptmap.
Canonical names below omit the leading src/ prefix.

