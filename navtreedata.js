/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "beman::optional", "index.html", [
    [ "codestyle", "md_docs_codestyle.html", null ],
    [ "CODING_RULES", "md_docs_CODING_RULES.html", [
      [ "Coding Rules", "md_docs_CODING_RULES.html#autotoc_md0", [
        [ "Semantic Defaults", "md_docs_CODING_RULES.html#autotoc_md1", null ],
        [ "Project Layout", "md_docs_CODING_RULES.html#autotoc_md2", null ],
        [ "File Prolog and Includes", "md_docs_CODING_RULES.html#autotoc_md3", null ],
        [ "CMake and Build Graph", "md_docs_CODING_RULES.html#autotoc_md4", null ],
        [ "C++ Structure", "md_docs_CODING_RULES.html#autotoc_md5", null ],
        [ "Language and Tooling", "md_docs_CODING_RULES.html#autotoc_md6", null ],
        [ "Typeclass Design", "md_docs_CODING_RULES.html#autotoc_md7", null ],
        [ "Foldable Rules", "md_docs_CODING_RULES.html#autotoc_md8", null ],
        [ "Applicative Rules", "md_docs_CODING_RULES.html#autotoc_md9", null ],
        [ "Traversable Rules", "md_docs_CODING_RULES.html#autotoc_md10", null ],
        [ "Test Rules", "md_docs_CODING_RULES.html#autotoc_md11", null ],
        [ "Slide and Transclusion Rules", "md_docs_CODING_RULES.html#autotoc_md12", null ],
        [ "Prose and Documentation Formatting", "md_docs_CODING_RULES.html#autotoc_md13", null ]
      ] ]
    ] ],
    [ "FingerTree5 Allocator Design", "md_docs_finger_tree5_allocator_design.html", [
      [ "Context", "md_docs_finger_tree5_allocator_design.html#autotoc_md15", null ],
      [ "The Lakos Coherency Rule", "md_docs_finger_tree5_allocator_design.html#autotoc_md17", null ],
      [ "Design Decisions", "md_docs_finger_tree5_allocator_design.html#autotoc_md19", [
        [ "1. Allocator as a 4th template parameter", "md_docs_finger_tree5_allocator_design.html#autotoc_md20", null ],
        [ "2. Which allocations use the custom allocator", "md_docs_finger_tree5_allocator_design.html#autotoc_md21", null ],
        [ "3. Coherency enforcement at the public API surface", "md_docs_finger_tree5_allocator_design.html#autotoc_md22", null ],
        [ "4. Detecting allocator equality", "md_docs_finger_tree5_allocator_design.html#autotoc_md23", null ],
        [ "5. The <tt>uses_allocator</tt> extended constructor", "md_docs_finger_tree5_allocator_design.html#autotoc_md24", null ],
        [ "6. <tt>propagate_on_container_*</tt> traits", "md_docs_finger_tree5_allocator_design.html#autotoc_md25", null ],
        [ "7. The <tt>append</tt> fast path", "md_docs_finger_tree5_allocator_design.html#autotoc_md26", null ],
        [ "8. Spine shell allocation — <tt>allocate_spine</tt>", "md_docs_finger_tree5_allocator_design.html#autotoc_md27", null ]
      ] ],
      [ "Known Limitations", "md_docs_finger_tree5_allocator_design.html#autotoc_md29", [
        [ "Static <tt>leaf()</tt> and <tt>from_sequence()</tt> factories use <tt>ALLOCATOR{}</tt>", "md_docs_finger_tree5_allocator_design.html#autotoc_md30", null ]
      ] ],
      [ "PMR <tt>smd::tree::pmr::FingerTree5</tt>", "md_docs_finger_tree5_allocator_design.html#autotoc_md32", null ]
    ] ],
    [ "FT11 Lazy Queue Removal Implementation Notes", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html", [
      [ "Overview", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md34", null ],
      [ "Problem Summary", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md35", null ],
      [ "Why Cross-Measure Split Fails", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md36", [
        [ "Root Cause", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md37", null ],
        [ "Example Failure Scenario", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md38", null ]
      ] ],
      [ "Solution Implemented: Deterministic Rebuild Path", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md39", [
        [ "Strategy", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md40", null ],
        [ "Advantages", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md41", null ],
        [ "Trade-offs", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md42", null ],
        [ "Why This Is a Valid Interim Solution", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md43", null ]
      ] ],
      [ "Testing & Validation", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md44", [
        [ "Stress Regression Test", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md45", null ],
        [ "Validation Results", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md46", null ]
      ] ],
      [ "Memory Safety Notes for Future Work", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md47", [
        [ "OOM Prevention", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md48", null ],
        [ "ASAN Configuration", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md49", null ],
        [ "Debugging Crashes Without gdb Loops", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md50", null ]
      ] ],
      [ "Recommendations for Future Implementers", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md51", [
        [ "Next Steps for Lazy Removal", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md52", null ],
        [ "Code Review Checklist", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md53", null ],
        [ "Documentation", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md54", null ]
      ] ],
      [ "References", "md_docs_FT11_LAZY_QUEUE_REMOVAL_NOTES.html#autotoc_md55", null ]
    ] ],
    [ "Follow-up plan for FingerTree5 (handoff to a fresh Claude session)", "md_docs_ft5_followup.html", [
      [ "Orientation (read this first)", "md_docs_ft5_followup.html#autotoc_md57", null ],
      [ "Item 1 — Bidirectional iterator + <tt>std::ranges::view_interface</tt>", "md_docs_ft5_followup.html#autotoc_md59", [
        [ "Goal", "md_docs_ft5_followup.html#autotoc_md60", null ],
        [ "Files", "md_docs_ft5_followup.html#autotoc_md61", null ],
        [ "Type & state", "md_docs_ft5_followup.html#autotoc_md62", null ],
        [ "Algorithms", "md_docs_ft5_followup.html#autotoc_md63", null ],
        [ "Gotchas", "md_docs_ft5_followup.html#autotoc_md64", null ],
        [ "Tests", "md_docs_ft5_followup.html#autotoc_md65", null ],
        [ "Commit shape", "md_docs_ft5_followup.html#autotoc_md66", null ]
      ] ],
      [ "Item 2 — <tt>reversed()</tt> + <tt>DualMonoid<M></tt>", "md_docs_ft5_followup.html#autotoc_md68", [
        [ "Goal", "md_docs_ft5_followup.html#autotoc_md69", null ],
        [ "Files", "md_docs_ft5_followup.html#autotoc_md70", null ],
        [ "<tt>DualMonoid<M></tt> design", "md_docs_ft5_followup.html#autotoc_md71", null ],
        [ "<tt>reversed()</tt> design — decision required", "md_docs_ft5_followup.html#autotoc_md72", null ],
        [ "Gotchas", "md_docs_ft5_followup.html#autotoc_md73", null ],
        [ "Tests", "md_docs_ft5_followup.html#autotoc_md74", null ],
        [ "Commit shape", "md_docs_ft5_followup.html#autotoc_md75", null ]
      ] ],
      [ "Item 3 — Parameterize wrappers on tree type", "md_docs_ft5_followup.html#autotoc_md77", [
        [ "Goal", "md_docs_ft5_followup.html#autotoc_md78", null ],
        [ "Approach", "md_docs_ft5_followup.html#autotoc_md79", null ],
        [ "Files to modify", "md_docs_ft5_followup.html#autotoc_md80", null ],
        [ "Sequencing — do <tt>random_access</tt> first", "md_docs_ft5_followup.html#autotoc_md81", null ],
        [ "Gotchas", "md_docs_ft5_followup.html#autotoc_md82", null ],
        [ "Tests", "md_docs_ft5_followup.html#autotoc_md83", null ],
        [ "Commit shape", "md_docs_ft5_followup.html#autotoc_md84", null ]
      ] ],
      [ "Conventions to honor throughout", "md_docs_ft5_followup.html#autotoc_md86", null ],
      [ "Suggested overall sequence", "md_docs_ft5_followup.html#autotoc_md88", null ]
    ] ],
    [ "index", "md_docs_index.html", [
      [ "Overview", "md_docs_index.html#autotoc_md89", null ],
      [ "Active Source Files", "md_docs_index.html#autotoc_md90", [
        [ "smd/fixpoint/box.hpp", "md_docs_index.html#autotoc_md91", null ],
        [ "smd/fixpoint/cata.hpp", "md_docs_index.html#autotoc_md92", null ],
        [ "smd/fixpoint/fix.hpp", "md_docs_index.html#autotoc_md93", null ],
        [ "smd/fixpoint/overloaded.hpp", "md_docs_index.html#autotoc_md94", null ],
        [ "smd/ranges/range<sub>applicative.hpp</sub>", "md_docs_index.html#autotoc_md95", null ],
        [ "smd/ranges/range<sub>foldable.hpp</sub>", "md_docs_index.html#autotoc_md96", null ],
        [ "smd/ranges/range<sub>functor.hpp</sub>", "md_docs_index.html#autotoc_md97", null ],
        [ "smd/ranges/range<sub>list.hpp</sub>", "md_docs_index.html#autotoc_md98", null ],
        [ "smd/ranges/range<sub>traversable.hpp</sub>", "md_docs_index.html#autotoc_md99", null ],
        [ "smd/thunk/delay.hpp", "md_docs_index.html#autotoc_md100", null ],
        [ "smd/thunk/memoize.hpp", "md_docs_index.html#autotoc_md101", null ],
        [ "smd/tree/binary<sub>tree</sub><sub>applicative.hpp</sub>", "md_docs_index.html#autotoc_md102", null ],
        [ "smd/tree/binary<sub>tree</sub><sub>foldable.hpp</sub>", "md_docs_index.html#autotoc_md103", null ],
        [ "smd/tree/binary<sub>tree.hpp</sub>", "md_docs_index.html#autotoc_md104", null ],
        [ "smd/tree/binary<sub>tree</sub><sub>traversable.hpp</sub>", "md_docs_index.html#autotoc_md105", null ],
        [ "smd/tree/finger<sub>tree</sub><sub>foldable.hpp</sub>", "md_docs_index.html#autotoc_md106", null ],
        [ "smd/tree/finger<sub>tree.hpp</sub>", "md_docs_index.html#autotoc_md107", null ],
        [ "smd/tree/finger<sub>tree</sub><sub>interval</sub><sub>index.hpp</sub>", "md_docs_index.html#autotoc_md108", null ],
        [ "smd/tree/finger<sub>tree</sub><sub>priority</sub><sub>queue.hpp</sub>", "md_docs_index.html#autotoc_md109", null ],
        [ "smd/tree/finger<sub>tree</sub><sub>random</sub><sub>access.hpp</sub>", "md_docs_index.html#autotoc_md110", null ],
        [ "smd/tree/finger<sub>tree</sub><sub>rope.hpp</sub>", "md_docs_index.html#autotoc_md111", null ],
        [ "smd/tree/finger<sub>tree</sub><sub>traversable.hpp</sub>", "md_docs_index.html#autotoc_md112", null ],
        [ "smd/tree/finger<sub>tree</sub><sub>wrappers.hpp</sub>", "md_docs_index.html#autotoc_md113", null ],
        [ "smd/tree/fixpoint<sub>tree</sub><sub>foldable.hpp</sub>", "md_docs_index.html#autotoc_md114", null ],
        [ "smd/tree/fixpoint<sub>tree.hpp</sub>", "md_docs_index.html#autotoc_md115", null ],
        [ "smd/tree/fixpoint<sub>tree</sub><sub>traversable.hpp</sub>", "md_docs_index.html#autotoc_md116", null ],
        [ "smd/tree/fringe<sub>tree</sub><sub>applicative.hpp</sub>", "md_docs_index.html#autotoc_md117", null ],
        [ "smd/tree/fringe<sub>tree</sub><sub>foldable.hpp</sub>", "md_docs_index.html#autotoc_md118", null ],
        [ "smd/tree/fringe<sub>tree.hpp</sub>", "md_docs_index.html#autotoc_md119", null ],
        [ "smd/tree/fringe<sub>tree</sub><sub>traversable.hpp</sub>", "md_docs_index.html#autotoc_md120", null ],
        [ "smd/typeclass/applicative.hpp", "md_docs_index.html#autotoc_md121", null ],
        [ "smd/typeclass/examples/applicative<sub>bad.cpp</sub>", "md_docs_index.html#autotoc_md122", null ],
        [ "smd/typeclass/examples/applicative<sub>examples.cpp</sub>", "md_docs_index.html#autotoc_md123", null ],
        [ "smd/typeclass/examples/foldable<sub>examples.cpp</sub>", "md_docs_index.html#autotoc_md124", null ],
        [ "smd/typeclass/examples/lookup<sub>modes</sub><sub>examples.cpp</sub>", "md_docs_index.html#autotoc_md125", null ],
        [ "smd/typeclass/examples/traversable<sub>examples.cpp</sub>", "md_docs_index.html#autotoc_md126", null ],
        [ "smd/typeclass/foldable.hpp", "md_docs_index.html#autotoc_md127", null ],
        [ "smd/typeclass/functor.hpp", "md_docs_index.html#autotoc_md128", null ],
        [ "smd/typeclass/monad.hpp", "md_docs_index.html#autotoc_md129", null ],
        [ "smd/typeclass/monoid.hpp", "md_docs_index.html#autotoc_md130", null ],
        [ "smd/typeclass/traversable.hpp", "md_docs_index.html#autotoc_md131", null ],
        [ "smd/typeclass/typeclass<sub>base.hpp</sub>", "md_docs_index.html#autotoc_md132", null ],
        [ "smd/ziplist/zip<sub>list</sub><sub>applicative.hpp</sub>", "md_docs_index.html#autotoc_md133", null ],
        [ "smd/ziplist/zip<sub>list.hpp</sub>", "md_docs_index.html#autotoc_md134", null ]
      ] ],
      [ "Regeneration", "md_docs_index.html#autotoc_md135", null ]
    ] ],
    [ "Measure-Based Splitting in FingerTree", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html", [
      [ "Overview", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md284", null ],
      [ "Methods for Measure-Based Splitting", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md285", [
        [ "1. <tt>split_at(PREDICATE&& predicate)</tt> — Generic Predicate-Based Split", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md286", null ],
        [ "2. <tt>split_at_measure(const Tag& threshold)</tt> — Convenience Measure Threshold", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md287", null ],
        [ "3. <tt>split(PREDICATE&& predicate)</tt> — Triple Split with Pivot", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md288", null ]
      ] ],
      [ "Comparison with <tt>split_at_index()</tt>", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md289", null ],
      [ "How Measure-Based Splitting Works Internally", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md290", [
        [ "The Algorithm: <tt>split_segment()</tt>", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md291", null ]
      ] ],
      [ "Code Examples", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md292", [
        [ "Example 1: Count-Based Split (Default Measure)", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md293", null ],
        [ "Example 2: Weighted Measure Split", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md294", null ],
        [ "Example 3: Interval Pruning (Practical Use Case)", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md295", null ],
        [ "Example 4: Custom Predicate for Complex Conditions", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md296", null ],
        [ "Example 5: Using <tt>split()</tt> to Extract Pivot", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md297", null ]
      ] ],
      [ "Performance Characteristics", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md298", [
        [ "Time Complexity", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md299", null ],
        [ "Space Complexity", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md300", null ],
        [ "Optimization: Measure-Aware Pruning", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md301", null ]
      ] ],
      [ "Related Concepts", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md302", [
        [ "Monoid Operations", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md303", null ],
        [ "Measure Policy", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md304", null ]
      ] ],
      [ "Summary", "md_docs_MEASURE_BASED_SPLITTING_GUIDE.html#autotoc_md305", null ]
    ] ],
    [ "Checklist: Beman Reference Layout", "md_docs_notes_beman_extraction_checklist.html", [
      [ "Purpose", "md_docs_notes_beman_extraction_checklist.html#autotoc_md307", null ],
      [ "Repository Bootstrap", "md_docs_notes_beman_extraction_checklist.html#autotoc_md308", null ],
      [ "Public Naming Contract", "md_docs_notes_beman_extraction_checklist.html#autotoc_md309", null ],
      [ "Algorithm Surface", "md_docs_notes_beman_extraction_checklist.html#autotoc_md310", null ],
      [ "Tree and Container Surface", "md_docs_notes_beman_extraction_checklist.html#autotoc_md311", null ],
      [ "Recursive Algorithm Source Material", "md_docs_notes_beman_extraction_checklist.html#autotoc_md312", null ],
      [ "<tt>detail/</tt> Support Layout", "md_docs_notes_beman_extraction_checklist.html#autotoc_md313", null ],
      [ "Adapter Consolidation", "md_docs_notes_beman_extraction_checklist.html#autotoc_md314", null ],
      [ "Tests and Examples", "md_docs_notes_beman_extraction_checklist.html#autotoc_md315", null ],
      [ "File-by-File Reference Map", "md_docs_notes_beman_extraction_checklist.html#autotoc_md316", [
        [ "Public API and support headers", "md_docs_notes_beman_extraction_checklist.html#autotoc_md317", null ],
        [ "<tt>detail/</tt> headers", "md_docs_notes_beman_extraction_checklist.html#autotoc_md318", null ],
        [ "Tests that move with the component", "md_docs_notes_beman_extraction_checklist.html#autotoc_md319", null ],
        [ "Examples that move with the component", "md_docs_notes_beman_extraction_checklist.html#autotoc_md320", null ],
        [ "Keep in <tt>trees</tt>", "md_docs_notes_beman_extraction_checklist.html#autotoc_md321", null ]
      ] ],
      [ "Completion Criteria for the Reference Layout", "md_docs_notes_beman_extraction_checklist.html#autotoc_md322", null ]
    ] ],
    [ "Pre-Migration Gaps: Work Required Before Beman Extraction", "md_docs_notes_beman_extraction_gaps.html", [
      [ "Gap 1: No history preservation strategy", "md_docs_notes_beman_extraction_gaps.html#autotoc_md325", null ],
      [ "Gap 2: Incomplete file mapping — missing components", "md_docs_notes_beman_extraction_gaps.html#autotoc_md327", [
        [ "2a. PMR typedef header", "md_docs_notes_beman_extraction_gaps.html#autotoc_md328", null ],
        [ "2b. Standalone PMR allocation probe", "md_docs_notes_beman_extraction_gaps.html#autotoc_md329", null ],
        [ "2c. Allocator design document", "md_docs_notes_beman_extraction_gaps.html#autotoc_md330", null ],
        [ "2d. The <tt>src/smd/fixpoint/</tt> directory", "md_docs_notes_beman_extraction_gaps.html#autotoc_md331", null ],
        [ "2e. Benchmark suite", "md_docs_notes_beman_extraction_gaps.html#autotoc_md332", null ]
      ] ],
      [ "Gap 3: No mechanical rewrite specification", "md_docs_notes_beman_extraction_gaps.html#autotoc_md334", null ],
      [ "Gap 4: <tt>recursive_fold.hpp</tt> and <tt>recursive_build.hpp</tt> need interface design", "md_docs_notes_beman_extraction_gaps.html#autotoc_md336", null ],
      [ "Gap 5: <tt>UnitMeasure5</tt> and other versioned measure type names", "md_docs_notes_beman_extraction_gaps.html#autotoc_md338", null ],
      [ "Gap 6: Two-repo synchronization policy", "md_docs_notes_beman_extraction_gaps.html#autotoc_md340", null ],
      [ "Gap 7: The <tt>*_ft5.t.cpp</tt> merge decision", "md_docs_notes_beman_extraction_gaps.html#autotoc_md342", null ],
      [ "Summary: Pre-extraction checklist", "md_docs_notes_beman_extraction_gaps.html#autotoc_md344", null ]
    ] ],
    [ "Plan: Integrate the Proposal Set into One Beman Project", "md_docs_notes_beman_integration_plan.html", [
      [ "Purpose", "md_docs_notes_beman_integration_plan.html#autotoc_md346", null ],
      [ "Naming Comes First", "md_docs_notes_beman_integration_plan.html#autotoc_md347", null ],
      [ "Naming Rules for the Future Beman Library", "md_docs_notes_beman_integration_plan.html#autotoc_md348", [
        [ "1. The library name must carry the conceptual grouping", "md_docs_notes_beman_integration_plan.html#autotoc_md349", null ],
        [ "2. Public headers should be flat by default", "md_docs_notes_beman_integration_plan.html#autotoc_md350", null ],
        [ "3. Filenames should name the facility, not the framework story", "md_docs_notes_beman_integration_plan.html#autotoc_md351", null ],
        [ "4. Avoid duplicate nouns across directory and filename", "md_docs_notes_beman_integration_plan.html#autotoc_md352", null ],
        [ "5. Prefer a slightly longer filename over another directory level", "md_docs_notes_beman_integration_plan.html#autotoc_md353", null ],
        [ "6. Internal organization does not need to mirror public organization", "md_docs_notes_beman_integration_plan.html#autotoc_md354", null ]
      ] ],
      [ "Consequences for Repository Naming", "md_docs_notes_beman_integration_plan.html#autotoc_md355", null ],
      [ "Consequences for Namespace Naming", "md_docs_notes_beman_integration_plan.html#autotoc_md356", null ],
      [ "Consequences for Extraction", "md_docs_notes_beman_integration_plan.html#autotoc_md357", null ],
      [ "Documentation Language Guidance", "md_docs_notes_beman_integration_plan.html#autotoc_md358", null ],
      [ "Naming Evaluation Rubric", "md_docs_notes_beman_integration_plan.html#autotoc_md359", [
        [ "1. Does the include path stutter?", "md_docs_notes_beman_integration_plan.html#autotoc_md360", null ],
        [ "2. Does the library name carry enough semantic weight?", "md_docs_notes_beman_integration_plan.html#autotoc_md361", null ],
        [ "3. Does the library name overconstrain the paper set?", "md_docs_notes_beman_integration_plan.html#autotoc_md362", null ],
        [ "4. Can the public headers remain flat?", "md_docs_notes_beman_integration_plan.html#autotoc_md363", null ],
        [ "5. Does the namespace read cleanly in examples?", "md_docs_notes_beman_integration_plan.html#autotoc_md364", null ],
        [ "6. Does the CMake target name read cleanly?", "md_docs_notes_beman_integration_plan.html#autotoc_md365", null ],
        [ "7. Does the name still work if one surface becomes more important?", "md_docs_notes_beman_integration_plan.html#autotoc_md366", null ]
      ] ],
      [ "Immediate Naming Task", "md_docs_notes_beman_integration_plan.html#autotoc_md367", null ],
      [ "Current Candidate Short Names", "md_docs_notes_beman_integration_plan.html#autotoc_md368", [
        [ "Candidate: <tt>structure</tt>", "md_docs_notes_beman_integration_plan.html#autotoc_md369", null ],
        [ "Candidate: <tt>shape</tt>", "md_docs_notes_beman_integration_plan.html#autotoc_md370", null ],
        [ "Candidate: <tt>functional</tt>", "md_docs_notes_beman_integration_plan.html#autotoc_md371", null ],
        [ "Candidate: <tt>context</tt>", "md_docs_notes_beman_integration_plan.html#autotoc_md372", null ],
        [ "Candidate: <tt>tree</tt>", "md_docs_notes_beman_integration_plan.html#autotoc_md373", null ],
        [ "Candidate: <tt>transpose</tt>", "md_docs_notes_beman_integration_plan.html#autotoc_md374", null ]
      ] ],
      [ "Current Naming Recommendation", "md_docs_notes_beman_integration_plan.html#autotoc_md375", null ],
      [ "Current Public Header Map", "md_docs_notes_beman_integration_plan.html#autotoc_md376", [
        [ "Likely top-level public algorithm headers", "md_docs_notes_beman_integration_plan.html#autotoc_md377", null ],
        [ "Likely later or advanced algorithm headers", "md_docs_notes_beman_integration_plan.html#autotoc_md378", null ],
        [ "Headers that should stay internal", "md_docs_notes_beman_integration_plan.html#autotoc_md379", null ],
        [ "Finger-tree public headers", "md_docs_notes_beman_integration_plan.html#autotoc_md380", null ],
        [ "Finger-tree headers that should become internal", "md_docs_notes_beman_integration_plan.html#autotoc_md381", null ],
        [ "Recursive-tree public headers", "md_docs_notes_beman_integration_plan.html#autotoc_md382", null ],
        [ "Recursive-tree adapter headers that should not remain public", "md_docs_notes_beman_integration_plan.html#autotoc_md383", null ]
      ] ],
      [ "Proposed Public Include Tree", "md_docs_notes_beman_integration_plan.html#autotoc_md384", [
        [ "Reference core shape", "md_docs_notes_beman_integration_plan.html#autotoc_md385", null ],
        [ "Reference recursive-algorithm additions", "md_docs_notes_beman_integration_plan.html#autotoc_md386", null ],
        [ "Headers that should stay internal from the start", "md_docs_notes_beman_integration_plan.html#autotoc_md387", null ]
      ] ],
      [ "Suggested Source-to-Header Translation", "md_docs_notes_beman_integration_plan.html#autotoc_md388", [
        [ "Algorithm surface", "md_docs_notes_beman_integration_plan.html#autotoc_md389", null ],
        [ "Persistent finger-tree surface", "md_docs_notes_beman_integration_plan.html#autotoc_md390", null ],
        [ "Recursive-tree surface", "md_docs_notes_beman_integration_plan.html#autotoc_md391", null ]
      ] ],
      [ "Export Policy for the Reference Implementation", "md_docs_notes_beman_integration_plan.html#autotoc_md392", null ],
      [ "Classification of Current <tt>src/smd/...</tt> Headers", "md_docs_notes_beman_integration_plan.html#autotoc_md393", [
        [ "<tt>src/smd/typeclass/</tt>", "md_docs_notes_beman_integration_plan.html#autotoc_md394", [
          [ "Public facade candidates", "md_docs_notes_beman_integration_plan.html#autotoc_md395", null ],
          [ "Public support candidates", "md_docs_notes_beman_integration_plan.html#autotoc_md396", null ],
          [ "<tt>detail/</tt> support candidates", "md_docs_notes_beman_integration_plan.html#autotoc_md397", null ]
        ] ],
        [ "<tt>src/smd/tree/</tt>", "md_docs_notes_beman_integration_plan.html#autotoc_md398", [
          [ "Public facade candidates", "md_docs_notes_beman_integration_plan.html#autotoc_md399", null ],
          [ "Additional public and source-material candidates", "md_docs_notes_beman_integration_plan.html#autotoc_md400", null ],
          [ "<tt>detail/</tt> support candidates", "md_docs_notes_beman_integration_plan.html#autotoc_md401", null ],
          [ "Headers that should usually not remain separate public headers", "md_docs_notes_beman_integration_plan.html#autotoc_md402", null ],
          [ "Headers that should stay in <tt>trees</tt>", "md_docs_notes_beman_integration_plan.html#autotoc_md403", null ]
        ] ]
      ] ],
      [ "Rule for <tt>detail/</tt> Headers", "md_docs_notes_beman_integration_plan.html#autotoc_md404", null ],
      [ "Public Naming Consequences for Current Headers", "md_docs_notes_beman_integration_plan.html#autotoc_md405", [
        [ "Names that should lose version suffixes", "md_docs_notes_beman_integration_plan.html#autotoc_md406", null ],
        [ "Names that should lose implementation nouns", "md_docs_notes_beman_integration_plan.html#autotoc_md407", null ],
        [ "Names that should lose adapter suffixes", "md_docs_notes_beman_integration_plan.html#autotoc_md408", null ]
      ] ],
      [ "Copier-Based Bootstrap Note", "md_docs_notes_beman_integration_plan.html#autotoc_md409", null ],
      [ "Context", "md_docs_notes_beman_integration_plan.html#autotoc_md410", null ],
      [ "Core Planning Decision", "md_docs_notes_beman_integration_plan.html#autotoc_md411", null ],
      [ "Primary Repository Questions", "md_docs_notes_beman_integration_plan.html#autotoc_md412", [
        [ "1. Library identity", "md_docs_notes_beman_integration_plan.html#autotoc_md413", null ],
        [ "2. Scope of one repository", "md_docs_notes_beman_integration_plan.html#autotoc_md414", null ],
        [ "3. Paper tracking in README and papers/", "md_docs_notes_beman_integration_plan.html#autotoc_md415", null ]
      ] ],
      [ "Structural Migration Plan", "md_docs_notes_beman_integration_plan.html#autotoc_md416", [
        [ "Current <tt>trees</tt> style", "md_docs_notes_beman_integration_plan.html#autotoc_md417", null ],
        [ "Target Beman style", "md_docs_notes_beman_integration_plan.html#autotoc_md418", null ],
        [ "Mapping guidance", "md_docs_notes_beman_integration_plan.html#autotoc_md419", [
          [ "Public headers", "md_docs_notes_beman_integration_plan.html#autotoc_md420", null ],
          [ "Non-public implementation headers and sources", "md_docs_notes_beman_integration_plan.html#autotoc_md421", null ],
          [ "Tests", "md_docs_notes_beman_integration_plan.html#autotoc_md422", null ],
          [ "Examples", "md_docs_notes_beman_integration_plan.html#autotoc_md423", null ],
          [ "Documentation", "md_docs_notes_beman_integration_plan.html#autotoc_md424", null ],
          [ "Paper sources", "md_docs_notes_beman_integration_plan.html#autotoc_md425", null ]
        ] ]
      ] ],
      [ "Code Extraction Categories", "md_docs_notes_beman_integration_plan.html#autotoc_md426", [
        [ "Category A: production-candidate code", "md_docs_notes_beman_integration_plan.html#autotoc_md427", null ],
        [ "Category B: support and proving-ground code", "md_docs_notes_beman_integration_plan.html#autotoc_md428", null ],
        [ "Category C: pedagogical and explanatory material", "md_docs_notes_beman_integration_plan.html#autotoc_md429", null ]
      ] ],
      [ "Beman-Specific Work Items", "md_docs_notes_beman_integration_plan.html#autotoc_md430", [
        [ "1. Choose the library name and namespace", "md_docs_notes_beman_integration_plan.html#autotoc_md431", null ],
        [ "2. Re-layout the source tree", "md_docs_notes_beman_integration_plan.html#autotoc_md432", null ],
        [ "3. Normalize file naming and test naming", "md_docs_notes_beman_integration_plan.html#autotoc_md433", null ],
        [ "4. Establish top-level Beman repo files", "md_docs_notes_beman_integration_plan.html#autotoc_md434", null ],
        [ "5. Adapt build system shape", "md_docs_notes_beman_integration_plan.html#autotoc_md435", null ],
        [ "6. Handle feature-conditional code the Beman way", "md_docs_notes_beman_integration_plan.html#autotoc_md436", null ],
        [ "7. Split usage docs from design docs", "md_docs_notes_beman_integration_plan.html#autotoc_md437", null ],
        [ "8. Add Beman-style examples and integration documentation", "md_docs_notes_beman_integration_plan.html#autotoc_md438", null ],
        [ "9. Papers directory and coordinated paper tracking", "md_docs_notes_beman_integration_plan.html#autotoc_md439", null ],
        [ "10. Decide maturity and release posture", "md_docs_notes_beman_integration_plan.html#autotoc_md440", null ]
      ] ],
      [ "Comparison: Local <tt>trees</tt> Style vs Beman Style", "md_docs_notes_beman_integration_plan.html#autotoc_md441", [
        [ "Layout", "md_docs_notes_beman_integration_plan.html#autotoc_md442", null ],
        [ "Public include identity", "md_docs_notes_beman_integration_plan.html#autotoc_md443", null ],
        [ "Tests", "md_docs_notes_beman_integration_plan.html#autotoc_md444", null ],
        [ "Papers", "md_docs_notes_beman_integration_plan.html#autotoc_md445", null ]
      ] ],
      [ "Recommended Integration Strategy", "md_docs_notes_beman_integration_plan.html#autotoc_md446", [
        [ "Repository framing", "md_docs_notes_beman_integration_plan.html#autotoc_md447", null ],
        [ "Core reference surface", "md_docs_notes_beman_integration_plan.html#autotoc_md448", null ],
        [ "Recursive and container expansion", "md_docs_notes_beman_integration_plan.html#autotoc_md449", null ],
        [ "Documentation split and stabilization", "md_docs_notes_beman_integration_plan.html#autotoc_md450", null ]
      ] ],
      [ "What Should Stay in <tt>trees</tt>", "md_docs_notes_beman_integration_plan.html#autotoc_md451", null ],
      [ "Cross-Reference Guidance", "md_docs_notes_beman_integration_plan.html#autotoc_md452", null ],
      [ "Summary", "md_docs_notes_beman_integration_plan.html#autotoc_md453", null ]
    ] ],
    [ "Updated Beman Migration Plan (v2)", "md_docs_notes_beman_migration_plan_v2.html", [
      [ "Strategic decisions unchanged from v1", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md456", null ],
      [ "Phase 0: Pre-extraction work (must complete before any file moves)", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md458", [
        [ "0.1 Commit to a history-preservation mechanism", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md459", null ],
        [ "0.2 Write and test the mechanical rewrite script", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md460", null ],
        [ "0.3 Sketch the <tt>recursive_fold.hpp</tt> and <tt>recursive_build.hpp</tt> interfaces", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md461", null ],
        [ "0.4 Decide stable non-versioned names for measure policy types", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md462", null ],
        [ "0.5 Decide the PMR header surface", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md463", null ],
        [ "0.6 Decide the <tt>*_ft5.t.cpp</tt> merge strategy", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md464", null ],
        [ "0.7 Write the two-repo synchronization policy", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md465", null ]
      ] ],
      [ "Complete file mapping", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md467", [
        [ "Algorithm / typeclass surface", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md468", null ],
        [ "Fixpoint module (previously unaddressed)", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md469", null ],
        [ "Finger-tree core", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md470", null ],
        [ "Wrapper surface", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md471", null ],
        [ "Recursive-algorithm surface (design task, not file copy)", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md472", null ],
        [ "Supporting tree types", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md473", null ],
        [ "Tests", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md474", null ],
        [ "Examples", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md475", null ],
        [ "Documentation", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md476", null ],
        [ "Benchmarks (evidence, not production tests)", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md477", null ],
        [ "Stay in <tt>trees</tt>", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md478", null ]
      ] ],
      [ "Adapter consolidation rule (unchanged from v1, restated for completeness)", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md480", null ],
      [ "Extraction sequence", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md482", [
        [ "Phase 0: Pre-extraction decisions (no file moves)", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md483", null ],
        [ "Phase 1: Bootstrap the Beman repo", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md484", null ],
        [ "Phase 2: Import the typeclass / algorithm surface", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md485", null ],
        [ "Phase 3: Import the fixpoint module (<tt>detail/</tt>)", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md486", null ],
        [ "Phase 4: Import the finger-tree core", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md487", null ],
        [ "Phase 5: Import the wrapper surface", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md488", null ],
        [ "Phase 6: Import supporting tree types", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md489", null ],
        [ "Phase 7: Import the recursive-algorithm surface", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md490", null ],
        [ "Phase 8: Add benchmarks", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md491", null ],
        [ "Phase 9: Final compliance checks", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md492", null ]
      ] ],
      [ "<tt>detail/</tt> namespace and header rule (unchanged from v1)", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md494", null ],
      [ "Comparison to <tt>beman-integration-plan.md</tt>", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md496", null ],
      [ "Summary", "md_docs_notes_beman_migration_plan_v2.html#autotoc_md498", null ]
    ] ],
    [ "Plan: Make FingerTree5 a C++ Container + AllocatorAware + PMR", "md_docs_notes_container_compliance_plan.html", [
      [ "Context", "md_docs_notes_container_compliance_plan.html#autotoc_md500", null ],
      [ "Current state (gap audit)", "md_docs_notes_container_compliance_plan.html#autotoc_md502", [
        [ "Type aliases present", "md_docs_notes_container_compliance_plan.html#autotoc_md503", null ],
        [ "Type aliases MISSING (Container requires)", "md_docs_notes_container_compliance_plan.html#autotoc_md504", null ],
        [ "Members present", "md_docs_notes_container_compliance_plan.html#autotoc_md505", null ],
        [ "Members MISSING", "md_docs_notes_container_compliance_plan.html#autotoc_md506", null ],
        [ "Allocation pattern", "md_docs_notes_container_compliance_plan.html#autotoc_md507", null ]
      ] ],
      [ "Phase 1 — Container named requirements", "md_docs_notes_container_compliance_plan.html#autotoc_md509", [
        [ "File modified", "md_docs_notes_container_compliance_plan.html#autotoc_md510", null ],
        [ "Changes", "md_docs_notes_container_compliance_plan.html#autotoc_md511", null ],
        [ "Tests to add (in finger_tree5.t.cpp)", "md_docs_notes_container_compliance_plan.html#autotoc_md512", null ]
      ] ],
      [ "Phase 2 — ReversibleContainer", "md_docs_notes_container_compliance_plan.html#autotoc_md514", [
        [ "File modified", "md_docs_notes_container_compliance_plan.html#autotoc_md515", null ],
        [ "Changes", "md_docs_notes_container_compliance_plan.html#autotoc_md516", null ],
        [ "Tests to add", "md_docs_notes_container_compliance_plan.html#autotoc_md517", null ]
      ] ],
      [ "Phase 3 — AllocatorAwareContainer", "md_docs_notes_container_compliance_plan.html#autotoc_md519", [
        [ "Design decisions", "md_docs_notes_container_compliance_plan.html#autotoc_md520", null ],
        [ "Files modified", "md_docs_notes_container_compliance_plan.html#autotoc_md521", null ],
        [ "Public API additions", "md_docs_notes_container_compliance_plan.html#autotoc_md522", null ],
        [ "Tests to add", "md_docs_notes_container_compliance_plan.html#autotoc_md523", null ]
      ] ],
      [ "Phase 4 — PMR typedef", "md_docs_notes_container_compliance_plan.html#autotoc_md525", [
        [ "New file", "md_docs_notes_container_compliance_plan.html#autotoc_md526", null ],
        [ "Content", "md_docs_notes_container_compliance_plan.html#autotoc_md527", null ],
        [ "Tests", "md_docs_notes_container_compliance_plan.html#autotoc_md528", null ]
      ] ],
      [ "Phase 5 — Convenience aliases for familiarity (optional)", "md_docs_notes_container_compliance_plan.html#autotoc_md530", null ],
      [ "Verification", "md_docs_notes_container_compliance_plan.html#autotoc_md532", null ],
      [ "Commit sequence", "md_docs_notes_container_compliance_plan.html#autotoc_md534", null ],
      [ "Risk: Allocator parameter changes the type signature", "md_docs_notes_container_compliance_plan.html#autotoc_md536", null ]
    ] ],
    [ "Current-State Audit: 2026-05-02", "md_docs_notes_current_state_audit_2026_05_02.html", [
      [ "Findings Summary", "md_docs_notes_current_state_audit_2026_05_02.html#autotoc_md539", [
        [ "1. [CORRECTNESS] <tt>split_at_index</tt> fast-path contract violation (MEDIUM)", "md_docs_notes_current_state_audit_2026_05_02.html#autotoc_md540", null ],
        [ "2. [REPO-HEALTH] Orphan dead test file masking broken include (MEDIUM)", "md_docs_notes_current_state_audit_2026_05_02.html#autotoc_md542", null ],
        [ "3. [REPO-HEALTH] Broad formatting churn in vendored Catch2 (LOW-TO-MEDIUM)", "md_docs_notes_current_state_audit_2026_05_02.html#autotoc_md544", null ],
        [ "4. [MINOR] Benign CMake and C++ formatting changes", "md_docs_notes_current_state_audit_2026_05_02.html#autotoc_md546", null ],
        [ "5. [MINOR] Test-helper alternate-core declaration reorder (LOW)", "md_docs_notes_current_state_audit_2026_05_02.html#autotoc_md548", null ]
      ] ],
      [ "Summary of Action Items", "md_docs_notes_current_state_audit_2026_05_02.html#autotoc_md550", null ],
      [ "Validation", "md_docs_notes_current_state_audit_2026_05_02.html#autotoc_md552", null ]
    ] ],
    [ "FingerTree2/3/4/5 Benchmark Results — 2026-05-15", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html", [
      [ "Environment", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md555", null ],
      [ "How to reproduce", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md556", [
        [ "Build", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md557", null ],
        [ "Run all non-crashing benchmarks", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md558", null ],
        [ "Run the isolated crash-case tests", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md559", null ],
        [ "Measure compile-time template expansion (DWARF proxy)", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md560", null ]
      ] ],
      [ "Results", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md562", [
        [ "1. Build — <tt>snoc</tt> N elements", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md564", null ],
        [ "2. Traverse — <tt>flatten</tt> of a pre-built N-element tree", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md566", null ],
        [ "3. Split at midpoint — O(log N) structural operation", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md568", null ],
        [ "4. Append — O(log N) structural operation", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md570", null ],
        [ "5. Persistent fan-out — 100 <tt>cons</tt> calls from the same base snapshot", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md572", null ],
        [ "6. Persistent drain — flatten 100 derived trees from same base", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md574", null ],
        [ "7. Compile-time template expansion — DWARF <tt>DW_TAG_class_type</tt> count", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md576", null ]
      ] ],
      [ "Summary of findings", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md578", [
        [ "Correctness failures at scale", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md579", null ],
        [ "Which tree to use", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md580", null ],
        [ "FT3's amortisation advantage is real but scoped", "md_docs_notes_finger_tree_benchmark_results_2026_05_15.html#autotoc_md581", null ]
      ] ]
    ] ],
    [ "Code Review: finger_tree5.hpp + finger_tree5_iterator.hpp — 2026-05-16", "md_docs_notes_ft5_code_review_2026_05_16.html", [
      [ "1. Redundant <tt>overloaded</tt> deduction guide (C++20 supersedes)", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md584", null ],
      [ "2. <tt>inline</tt> on free function templates is redundant", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md586", null ],
      [ "3. <tt>operator!=</tt> is generated from <tt>operator==</tt> in C++20", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md588", null ],
      [ "4. <tt>tag_id()</tt> and <tt>tag_op()</tt> should be <tt>constexpr</tt>", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md590", null ],
      [ "5. <tt>make_leaf</tt> takes <tt>T value</tt> by value — potential double-move", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md592", null ],
      [ "6. <tt>nodes_from</tt> takes <tt>std::vector</tt> by value but could take a span", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md594", null ],
      [ "7. <tt>digit_to_vec</tt> unnecessarily allocates — could return <tt>std::span</tt>", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md596", null ],
      [ "8. <tt>app3</tt> takes <tt>middle</tt> by value but immediately iterates it", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md598", null ],
      [ "9. Raw <tt>nullptr</tt> used where <tt>SpinePtr{}</tt> would be more type-safe", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md600", null ],
      [ "10. <tt>if (!spine || spine->is_empty())</tt> duplicates a null check", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md602", null ],
      [ "11. <tt>view_l</tt> / <tt>view_r</tt> copy the value out of the Leaf", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md604", null ],
      [ "12. <tt>from_sequence</tt> is O(N) via repeated <tt>snoc</tt> — could be O(N) bottom-up", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md606", null ],
      [ "13. Iterator: <tt>make_begin</tt> copies the tree into a <tt>shared_ptr</tt>", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md608", null ],
      [ "14. Missing <tt>[[nodiscard]]</tt> on pure-functional operations", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md610", null ],
      [ "15. <tt>SpineFrame::Section</tt> enum should be an <tt>enum class</tt>", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md612", null ],
      [ "16. <tt>assert(false && \"...\")</tt> is not <tt>[[noreturn]]</tt>-friendly", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md614", null ],
      [ "17. Repetitive digit construction in <tt>digit_to_tree</tt>", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md616", null ],
      [ "Summary priority", "md_docs_notes_ft5_code_review_2026_05_16.html#autotoc_md618", null ]
    ] ],
    [ "Performance Analysis & Optimization Plan for FingerTree5", "md_docs_notes_ft5_performance_plan.html", [
      [ "Measured type sizes (gcc-16, x86-64)", "md_docs_notes_ft5_performance_plan.html#autotoc_md620", null ],
      [ "Identified hot spots and pessimizations", "md_docs_notes_ft5_performance_plan.html#autotoc_md622", [
        [ "H1. <tt>make_shared</tt> on every internal node — the dominant cost", "md_docs_notes_ft5_performance_plan.html#autotoc_md623", null ],
        [ "H2. <tt>std::visit</tt> dispatch — variant overhead on the hot path", "md_docs_notes_ft5_performance_plan.html#autotoc_md625", null ],
        [ "H3. Digit copy in <tt>digit_with_pushed_front</tt> / <tt>digit_with_pushed_back</tt>", "md_docs_notes_ft5_performance_plan.html#autotoc_md627", null ],
        [ "H4. <tt>make_deep</tt> recomputes <tt>digit_measure</tt> every time", "md_docs_notes_ft5_performance_plan.html#autotoc_md629", null ],
        [ "H5. <tt>from_sequence</tt> — O(N) repeated snoc with high constant factor", "md_docs_notes_ft5_performance_plan.html#autotoc_md631", null ],
        [ "H6. <tt>app3</tt> creates intermediate <tt>std::vector</tt> allocations", "md_docs_notes_ft5_performance_plan.html#autotoc_md633", null ],
        [ "H7. <tt>flatten</tt> / <tt>for_each</tt> traverse via recursive <tt>std::visit</tt>", "md_docs_notes_ft5_performance_plan.html#autotoc_md635", null ],
        [ "H8. Iterator <tt>make_begin</tt> copies the tree into a <tt>make_shared</tt>", "md_docs_notes_ft5_performance_plan.html#autotoc_md637", null ],
        [ "H9. <tt>view_l</tt> / <tt>view_r</tt> copy the element value", "md_docs_notes_ft5_performance_plan.html#autotoc_md639", null ]
      ] ],
      [ "How to measure before making changes", "md_docs_notes_ft5_performance_plan.html#autotoc_md641", [
        [ "Tool 1: Counting allocator", "md_docs_notes_ft5_performance_plan.html#autotoc_md642", null ],
        [ "Tool 2: <tt>perf stat</tt> on benchmark binary", "md_docs_notes_ft5_performance_plan.html#autotoc_md643", null ],
        [ "Tool 3: <tt>perf record</tt> + flame graph", "md_docs_notes_ft5_performance_plan.html#autotoc_md644", null ],
        [ "Tool 4: Valgrind DHAT (heap profiler)", "md_docs_notes_ft5_performance_plan.html#autotoc_md645", null ],
        [ "Tool 5: Targeted micro-benchmarks", "md_docs_notes_ft5_performance_plan.html#autotoc_md646", null ]
      ] ],
      [ "Prioritized optimization roadmap", "md_docs_notes_ft5_performance_plan.html#autotoc_md648", null ],
      [ "Key principle: measure first, then optimize", "md_docs_notes_ft5_performance_plan.html#autotoc_md650", null ]
    ] ],
    [ "Re-Review: May 2 Finger Tree Merge Chunks", "md_docs_notes_re_review_2026_05_02_finger_tree_merge_chunks.html", [
      [ "Summary", "md_docs_notes_re_review_2026_05_02_finger_tree_merge_chunks.html#autotoc_md652", null ],
      [ "Findings (ordered by severity)", "md_docs_notes_re_review_2026_05_02_finger_tree_merge_chunks.html#autotoc_md653", null ],
      [ "1) High: <tt>split_at_index</tt> optimization can be wrong for custom <tt>size_t</tt> measure policies", "md_docs_notes_re_review_2026_05_02_finger_tree_merge_chunks.html#autotoc_md654", null ],
      [ "2) Medium: no regression test locks this corner case", "md_docs_notes_re_review_2026_05_02_finger_tree_merge_chunks.html#autotoc_md655", null ],
      [ "3) Low: review note wording overstates optimization scope and test count is stale", "md_docs_notes_re_review_2026_05_02_finger_tree_merge_chunks.html#autotoc_md656", null ],
      [ "Worker Patch Set", "md_docs_notes_re_review_2026_05_02_finger_tree_merge_chunks.html#autotoc_md658", [
        [ "Patch 1: Guard fast path by count measure semantics", "md_docs_notes_re_review_2026_05_02_finger_tree_merge_chunks.html#autotoc_md659", null ],
        [ "Patch 2: Add regression test for weighted <tt>size_t</tt> measure policy", "md_docs_notes_re_review_2026_05_02_finger_tree_merge_chunks.html#autotoc_md660", null ],
        [ "Patch 3: Update DONE-note wording and stale test count", "md_docs_notes_re_review_2026_05_02_finger_tree_merge_chunks.html#autotoc_md661", null ]
      ] ],
      [ "Validation checklist for worker agent", "md_docs_notes_re_review_2026_05_02_finger_tree_merge_chunks.html#autotoc_md663", null ],
      [ "Reviewer acceptance criteria", "md_docs_notes_re_review_2026_05_02_finger_tree_merge_chunks.html#autotoc_md665", null ]
    ] ],
    [ "Inverted Triangle Plan: From Design Thesis to Papers", "md_docs_notes_standardization_inverted_triangle_plan.html", [
      [ "Summary — read this first", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md667", null ],
      [ "Broad thesis", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md669", null ],
      [ "The three papers and the anchor", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md671", [
        [ "Paper A: Traversal, transposition, and bundled customization", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md672", null ],
        [ "Paper C: Persistent measured sequence", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md673", null ],
        [ "Paper D: Recursive tree algorithms", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md674", null ]
      ] ],
      [ "Current implementation readiness", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md676", [
        [ "Paper C: substantially complete", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md677", null ],
        [ "Paper A: blocked on the required example set", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md678", null ],
        [ "Paper D: still being shaped", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md679", null ]
      ] ],
      [ "Near-term decisions and current biases", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md681", null ],
      [ "Publication and coordination strategy", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md683", null ],
      [ "Architectural claims supporting the thesis", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md685", null ],
      [ "Review-control strategy", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md687", null ],
      [ "Paper boundaries and managed overlap", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md689", null ],
      [ "Detailed per-paper plans", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md691", [
        [ "Paper A plan", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md692", null ],
        [ "Paper C plan", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md693", null ],
        [ "Paper D plan", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md694", null ]
      ] ],
      [ "Current paper sketches", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md696", [
        [ "Paper A sketch", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md697", null ],
        [ "Paper C sketch", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md698", null ],
        [ "Paper D sketch", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md699", null ]
      ] ],
      [ "Acceptance-oriented implementation guidance", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md701", null ],
      [ "P3200 reservation", "md_docs_notes_standardization_inverted_triangle_plan.html#autotoc_md703", null ]
    ] ],
    [ "Plan: Benchmark Comparing FT5 Wrappers Against std Library Types", "md_docs_notes_std_comparison_benchmark_plan.html", [
      [ "Context", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md705", null ],
      [ "Output", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md706", null ],
      [ "Design principles", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md708", null ],
      [ "Category 1: RandomAccess vs std::vector / std::deque", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md710", [
        [ "Benchmarks", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md711", null ],
        [ "What to measure", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md712", null ],
        [ "Helper code", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md713", null ]
      ] ],
      [ "Category 2: Rope vs std::string", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md715", [
        [ "Benchmarks", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md716", null ],
        [ "What to measure", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md717", null ]
      ] ],
      [ "Category 3: PriorityQueue vs std::priority_queue", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md719", [
        [ "Benchmarks", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md720", null ],
        [ "What to measure", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md721", null ]
      ] ],
      [ "Category 4: IntervalIndex vs std::multimap (brute-force baseline)", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md723", [
        [ "Benchmarks", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md724", null ],
        [ "What to measure", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md725", null ]
      ] ],
      [ "Category 5: Persistence (the cross-cutting advantage)", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md727", [
        [ "Benchmarks", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md728", null ],
        [ "What to measure", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md729", null ]
      ] ],
      [ "Category 6: Sequential iteration (where std wins)", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md731", [
        [ "Expected results", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md732", null ]
      ] ],
      [ "Implementation structure", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md734", null ],
      [ "CMakeLists change", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md735", null ],
      [ "Running", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md736", null ],
      [ "Known gaps / TODOs for implementer", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md737", null ],
      [ "Verification", "md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md738", null ]
    ] ],
    [ "Structured Comparison: FingerTree5 Wrappers vs Standard Library Types", "md_docs_notes_std_comparison_finger_tree_wrappers.html", [
      [ "What FingerTree5 provides", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md741", null ],
      [ "1. FingerTreeRandomAccess vs std::vector / std::deque", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md743", [
        [ "Operation complexity", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md744", null ],
        [ "Where FingerTree5 wins", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md745", null ],
        [ "Where std types win", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md746", null ]
      ] ],
      [ "2. FingerTreeRope vs std::string", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md748", [
        [ "Operation complexity", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md749", null ],
        [ "Where FingerTree5 wins", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md750", null ],
        [ "Where std::string wins", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md751", null ],
        [ "Verdict", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md752", null ]
      ] ],
      [ "3. FingerTreePriorityQueue vs std::priority_queue", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md754", [
        [ "Operation complexity", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md755", null ],
        [ "Where FingerTree5 wins", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md756", null ],
        [ "Where std::priority_queue wins", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md757", null ],
        [ "Verdict", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md758", null ]
      ] ],
      [ "4. FingerTreeIntervalIndex — no std equivalent", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md760", null ],
      [ "5. Cross-cutting properties", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md762", [
        [ "5a. Persistence and structural sharing", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md763", null ],
        [ "5b. Iterator invalidation", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md764", null ],
        [ "5c. Concurrency safety", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md765", null ],
        [ "5d. Memory layout and cache performance", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md766", null ]
      ] ],
      [ "6. Summary decision matrix", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md768", null ],
      [ "7. What FingerTree5 enables that has no std equivalent", "md_docs_notes_std_comparison_finger_tree_wrappers.html#autotoc_md770", null ]
    ] ],
    [ "polymorphic recursion in fingertree spine", "md_docs_polymorphic_recursion_in_fingertree_spine.html", null ],
    [ "Finger Tree Compile Regression Notes", "md_docs_reference_finger_tree_compile_regression_notes.html", [
      [ "Problem Observed", "md_docs_reference_finger_tree_compile_regression_notes.html#autotoc_md998", null ],
      [ "Probe Findings", "md_docs_reference_finger_tree_compile_regression_notes.html#autotoc_md999", null ],
      [ "Temporary Fallback", "md_docs_reference_finger_tree_compile_regression_notes.html#autotoc_md1000", null ],
      [ "Guidance For The Next Structural Pass", "md_docs_reference_finger_tree_compile_regression_notes.html#autotoc_md1001", null ],
      [ "Practical Rule", "md_docs_reference_finger_tree_compile_regression_notes.html#autotoc_md1002", null ]
    ] ],
    [ "Understanding-F-Algebras", "md_docs_reference_Understanding_F_Algebras.html", null ],
    [ "polymorphic recursion in fingertree spine", "md_docs_RustLikeFingerTree.html", [
      [ "Search's response:", "md_docs_RustLikeFingerTree.html#autotoc_md1014", null ],
      [ "Polymorphic Recursion in the Spine", "md_docs_RustLikeFingerTree.html#autotoc_md1015", null ],
      [ "Architectural Specification: Non-Regular Finger Tree Text Rope", "md_docs_RustLikeFingerTree.html#autotoc_md1016", [
        [ "1. Executive Summary & Core Guarantees", "md_docs_RustLikeFingerTree.html#autotoc_md1019", null ],
        [ "2. Core Engine & Memory Topology", "md_docs_RustLikeFingerTree.html#autotoc_md1021", [
          [ "Design Blueprint for a Type-Flattened, Thread-Safe, Async-Capable Persistent Sequence", "md_docs_RustLikeFingerTree.html#autotoc_md1017", null ],
          [ "2.1 The Generalized Node Layout", "md_docs_RustLikeFingerTree.html#autotoc_md1022", null ]
        ] ]
      ] ]
    ] ],
    [ "Session Notes: 2026-05-02/03", "md_docs_session_2026_05_02_session_notes.html", [
      [ "Work Completed", "md_docs_session_2026_05_02_session_notes.html#autotoc_md1024", [
        [ "Build & Infrastructure", "md_docs_session_2026_05_02_session_notes.html#autotoc_md1025", null ],
        [ "Presentation Accuracy (Feynman Audit)", "md_docs_session_2026_05_02_session_notes.html#autotoc_md1026", null ],
        [ "Code Changes", "md_docs_session_2026_05_02_session_notes.html#autotoc_md1027", null ],
        [ "Documentation Cleanup", "md_docs_session_2026_05_02_session_notes.html#autotoc_md1028", null ]
      ] ],
      [ "Key Findings", "md_docs_session_2026_05_02_session_notes.html#autotoc_md1029", [
        [ "Code is already O(log n)", "md_docs_session_2026_05_02_session_notes.html#autotoc_md1030", null ],
        [ "Haskell cross-check", "md_docs_session_2026_05_02_session_notes.html#autotoc_md1031", null ],
        [ "Compile regression note", "md_docs_session_2026_05_02_session_notes.html#autotoc_md1032", null ]
      ] ],
      [ "Current State", "md_docs_session_2026_05_02_session_notes.html#autotoc_md1033", null ],
      [ "Open Items (very minor)", "md_docs_session_2026_05_02_session_notes.html#autotoc_md1034", null ]
    ] ],
    [ "typeclass-object-pattern", "md_docs_typeclass_object_pattern.html", [
      [ "Typeclass Object Pattern in This Repository", "md_docs_typeclass_object_pattern.html#autotoc_md1035", [
        [ "Why this exists", "md_docs_typeclass_object_pattern.html#autotoc_md1036", null ],
        [ "Two surfaces in this repo", "md_docs_typeclass_object_pattern.html#autotoc_md1037", null ],
        [ "Lookup modes (important)", "md_docs_typeclass_object_pattern.html#autotoc_md1038", null ],
        [ "Core mechanics", "md_docs_typeclass_object_pattern.html#autotoc_md1039", [
          [ "Concept side", "md_docs_typeclass_object_pattern.html#autotoc_md1040", null ],
          [ "Type side", "md_docs_typeclass_object_pattern.html#autotoc_md1041", null ],
          [ "Call side", "md_docs_typeclass_object_pattern.html#autotoc_md1042", null ]
        ] ],
        [ "How to add a new instance", "md_docs_typeclass_object_pattern.html#autotoc_md1043", null ],
        [ "How to add a new concept", "md_docs_typeclass_object_pattern.html#autotoc_md1044", null ],
        [ "Testing and build wiring expectations", "md_docs_typeclass_object_pattern.html#autotoc_md1045", null ],
        [ "Algorithm objects: Inheriting from typeclass instances", "md_docs_typeclass_object_pattern.html#autotoc_md1046", [
          [ "Pattern", "md_docs_typeclass_object_pattern.html#autotoc_md1047", null ],
          [ "Multi-typeclass composition", "md_docs_typeclass_object_pattern.html#autotoc_md1048", null ],
          [ "Key points", "md_docs_typeclass_object_pattern.html#autotoc_md1049", null ]
        ] ],
        [ "Applicative: Derived invoke via terminating partial application", "md_docs_typeclass_object_pattern.html#autotoc_md1050", null ],
        [ "Traps and corrections from tree-instance implementation", "md_docs_typeclass_object_pattern.html#autotoc_md1051", null ],
        [ "Notes for future cleanup", "md_docs_typeclass_object_pattern.html#autotoc_md1052", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"md_docs_notes_beman_integration_plan.html#autotoc_md394",
"md_docs_notes_std_comparison_benchmark_plan.html#autotoc_md729"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';