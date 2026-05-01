# FT11 Strict Finger Tree Follow-Up

Goal: restore lazy middle-edge-driven queue removals and add stress coverage while staying memory-safe.

## Scope

- Restore non-rebuild split/concat removal path in `FingerTreePriorityQueue`.
- Add stress regression coverage for repeated shared-version operations.
- Validate in Debug and Asan (includes lsan in this repo toolchain setup).

## Recovery Checklist (if interrupted)

1. Check branch and status:
   - `git branch --show-current`
   - `git status --short`
2. Confirm changed files:
   - `git --no-pager diff --stat`
3. Rebuild low-memory Debug:
   - `make -j1 compile TOOLCHAIN=gcc-16 CONFIG=Debug`
4. Run targeted tree tests first:
   - `./.build/build-gcc-16/src/smd/tree/Debug/smd_tree_tests "FingerTreePriorityQueueTest - WrapperOperations" -r compact`
   - `./.build/build-gcc-16/src/smd/tree/Debug/smd_tree_tests "FingerTreePersistenceTest - RepeatedSplitPopAcrossSharedVersions" -r compact`
5. Rebuild Asan:
   - `make -j1 compile TOOLCHAIN=gcc-16 CONFIG=Asan`
6. Run targeted Asan+lsan tests:
   - `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:abort_on_error=1 LSAN_OPTIONS=exitcode=23:report_objects=1 ./.build/build-gcc-16/src/smd/tree/Asan/smd_tree_tests "FingerTreePriorityQueueTest - WrapperOperations" -r compact`
7. Run full suite only after targeted tests pass:
   - `make -j1 test TOOLCHAIN=gcc-16 CONFIG=Debug`
   - `make -j1 test TOOLCHAIN=gcc-16 CONFIG=Asan`

## OOM Safety Notes

- Do not use `gdb` backtrace loops unless explicitly needed.
- Do not set `ulimit -v` while running Asan (Asan shadow mapping fails).
- Prefer `-j1` for all build/test steps.

## Work Items

- [ ] Replace deterministic rebuild path in `FingerTreePriorityQueue::pop_min` and `pop_max` with split/concat removal path.
- [x] Keep duplicate-value behavior stable (remove one matching element only).
- [x] Add/retain stress regression that exercises repeated split/pop/append on shared versions.
- [x] Verify Debug tree tests pass.
- [x] Verify Asan tree tests pass with leak detection enabled.
- [ ] Commit only on fully green tests.

## Status

- Branch start: `phase-ft11-lazy-queue-restore`
- Current: stability-first checkpoint complete. split/concat queue removal attempt regressed with deterministic segfault in `FingerTreePriorityQueueTest - WrapperOperations`; queue pops are currently on deterministic rebuild removal to keep branch green while root-causeing strict lazy removal path.
