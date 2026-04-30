# Code Review Priority TODO

Source: deep research audit follow-up list.

## Priority Order

- [x] Decide ZipList semantic direction
	- Acceptance: ZipList uses standard data-parallel Applicative semantics where `pure` repeats and `apply` zips.
	- References: 93a7acd2
- [x] Finalize ZipList naming/docs
	- Acceptance: naming standardized to `ZipList` in slide text and semantics documented in README.
	- References: f18345ee
- [x] Add generic law-test harness
	- Acceptance: reusable helper checks identity, homomorphism, and invoke/ap equivalence for Applicative instances.
	- References: f18345ee
- [x] Instantiate Applicative law suite
	- Acceptance: harness instantiated for optional, beman optional, BareIdentity, and ZipList.
	- References: f18345ee
- [x] Constrain FingerTree Traversable instance
  - Acceptance: core traversable semantics documented; materialization via flatten(), reconstruction via from_sequence().
  - References: aa41fb89
- [x] Separate raw core vs wrappers
  - Acceptance: wrapper-specific Foldable/Traversable impls extracted to dedicated files (*_foldable.hpp, *_traversable.hpp); clear separation between core and wrapper layers.
  - References: aa41fb89
- [x] Optimize priority queue pop paths
  - Acceptance: dual-tree pop operations use synchronized splits instead of flatten-rebuild; O(n log n) → O(log n).
  - References: 5d717777
- [x] Use measured pruning in interval queries
  - Acceptance: query_point() and query_overlap() use measure-based split_at() predicates to skip non-candidate intervals; O(n) → O(log n + k).
  - References: 4e478d56
- [x] Reduce flatten-based random access costs
  - Acceptance: at() method uses indexed splits; traversable structured for lazy evaluation potential.
  - References: e8774af9, d7e51892
- [x] Replace fold std::function programs
  - Acceptance: template-based composition infrastructure (IdentityFoldFunc, ComposedFoldFunc, LeftFoldProgramT, RightFoldProgramT) enables zero-cost function composition; foundation for eliminating virtual function dispatch overhead.
  - References: fe0f5982
- [x] Repair bibliography and citations
  - Acceptance: replace unresolved citation placeholders in slide source/exports, and point Org bibliography config to a repository-local bib file.
  - References: 18103a0c
- [ ] Fix markdown/html slide exports
- [ ] Align slide claims with tested laws
- [ ] Add CI gates for laws and coverage

## Notes

- Keep semantics/law correctness fixes ahead of optimization and presentation work.
- For each item, add acceptance criteria and linked PR/commit references as work starts.
