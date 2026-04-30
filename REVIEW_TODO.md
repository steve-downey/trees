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
- [ ] Constrain FingerTree Traversable instance
  - Acceptance: core traversable semantics documented; materialization via flatten(), reconstruction via from_sequence().
  - References: [commit TBD]
- [ ] Separate raw core vs wrappers
  - Acceptance: wrapper-specific Foldable/Traversable impls extracted to dedicated files (*_foldable.hpp, *_traversable.hpp); clear separation between core and wrapper layers.
  - References: [commit TBD]
- [ ] Optimize priority queue pop paths
- [ ] Use measured pruning in interval queries
- [ ] Reduce flatten-based random access costs
- [ ] Replace fold std::function programs
- [ ] Repair bibliography and citations
- [ ] Fix markdown/html slide exports
- [ ] Align slide claims with tested laws
- [ ] Add CI gates for laws and coverage

## Notes

- Keep semantics/law correctness fixes ahead of optimization and presentation work.
- For each item, add acceptance criteria and linked PR/commit references as work starts.
