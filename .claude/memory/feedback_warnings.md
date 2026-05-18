---
name: Treat warnings as errors
description: Fix all compiler warnings; do not dismiss them as pre-existing or minor
type: feedback
originSessionId: febb364f-d8bd-42ed-a1af-9c37e8675e7e
---
Fix every warning encountered during compilation, whether pre-existing or newly introduced.  Dismissing a warning as "just pre-existing" or "minor" is wrong.

**Why:** Although -Werror is not enabled in this project, warnings are bugs with a slow fuse.  "If you're explaining, you're already losing."

**How to apply:** When a warning appears in compile output, fix it before committing, even if it predates the current change.  Only acceptable exceptions are demonstrably false positives — the known example is -Wmaybe-uninitialized, which fires spuriously enough that it is turned off in the project.  All other warnings should be fixed, not rationalized.
