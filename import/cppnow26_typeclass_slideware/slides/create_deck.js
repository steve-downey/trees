const pptxgen = require('pptxgenjs');
const {
  warnIfSlideHasOverlaps,
  warnIfSlideElementsOutOfBounds,
} = require('/home/oai/skills/slides/pptxgenjs_helpers');

const fs = require('fs');
const path = require('path');

const root = path.resolve(__dirname, '..');
const out = path.resolve(root, '..', 'foldable_applicative_traversable_plan.pptx');

function readBlock(file, uuid) {
  const text = fs.readFileSync(path.resolve(root, file), 'utf8');
  const start = `// ${uuid}`;
  const end = `// ${uuid} end`;
  const i = text.indexOf(start);
  const j = text.indexOf(end);
  if (i < 0 || j < 0 || j < i) throw new Error(`missing block ${uuid}`);
  return text.slice(i + start.length, j).trim().split('\n').map(s => s.replace(/^    /, '')).join('\n');
}

const snippets = {
  length: readBlock('src/smd/typeclass/examples/foldable_examples.cpp', '9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa'),
  invoke: readBlock('src/smd/typeclass/examples/applicative_examples.cpp', '3f0c8d0e-9a6b-4a3e-9c2a-0c1e9c3d4f11'),
  traverse: readBlock('src/smd/typeclass/examples/traversable_examples.cpp', '5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2'),
  bad: readBlock('src/smd/typeclass/examples/applicative_bad.cpp', 'd2e7a1c9-0f3b-4b2e-9d55-1a8e7c4b2f90'),
};

const pptx = new pptxgen();
pptx.layout = 'LAYOUT_WIDE';
pptx.author = 'Steve Downey / OpenAI draft';
pptx.subject = 'Foldable, Applicative, Traversable concept-map implementation plan';
pptx.title = 'Foldable, Applicative, Traversable for Trees in C++';
pptx.company = 'Bloomberg';
pptx.lang = 'en-US';
pptx.theme = {
  headFontFace: 'Aptos Display',
  bodyFontFace: 'Aptos',
  lang: 'en-US',
};
pptx.defineLayout({ name: 'WIDE', width: 13.333, height: 7.5 });
pptx.layout = 'WIDE';

const C = {
  bg: 'FAFAF7',
  ink: '171717',
  sub: '4A4A4A',
  rule: 'D3D0C8',
  codeBg: '1E1E1E',
  code: 'E8E8E8',
  accent: '6B4EFF',
};

function base(slide, title) {
  slide.background = { color: C.bg };
  slide.addText(title, { x: 0.65, y: 0.35, w: 12.0, h: 0.42, fontFace: 'Aptos Display', fontSize: 24, bold: true, color: C.ink, margin: 0 });
  slide.addShape(pptx.ShapeType.line, { x: 0.65, y: 0.92, w: 12.0, h: 0, line: { color: C.rule, width: 1 } });
}

function addBullets(slide, items, x, y, w, h, size=17) {
  const runs = [];
  items.forEach((item) => {
    runs.push({ text: item, options: { bullet: { indent: 12 }, hanging: 4, breakLine: true } });
  });
  slide.addText(runs, { x, y, w, h, fontFace: 'Aptos', fontSize: size, color: C.ink, fit: 'shrink', margin: 0.05, breakLine: false });
}

function addText(slide, text, x, y, w, h, size=17, color=C.sub) {
  slide.addText(text, { x, y, w, h, fontFace: 'Aptos', fontSize: size, color, fit: 'shrink', margin: 0.05 });
}

function addCode(slide, code, x, y, w, h, size=13) {
  slide.addText(code, { x, y, w, h, fontFace: 'Consolas', fontSize: size, color: C.code, fill: { color: C.codeBg }, margin: 0.14, breakLine: false, fit: 'shrink' });
}

function check(slide) {
  warnIfSlideHasOverlaps(slide, pptx, { muteContainment: true, ignoreLines: true });
  warnIfSlideElementsOutOfBounds(slide, pptx);
}

let slide;

slide = pptx.addSlide();
slide.background = { color: C.bg };
slide.addText('Foldable, Applicative, Traversable', { x: 0.75, y: 1.45, w: 11.8, h: 0.7, fontFace: 'Aptos Display', fontSize: 39, bold: true, color: C.ink, margin: 0 });
slide.addText('typeclass-style interfaces for trees in C++', { x: 0.78, y: 2.25, w: 10.8, h: 0.4, fontSize: 22, color: C.sub, margin: 0 });
slide.addText('concept_map dispatch | law-first APIs | transcluded code that compiles', { x: 0.78, y: 3.15, w: 10.9, h: 0.34, fontSize: 17, color: C.accent, margin: 0 });
slide.addText('draft implementation plan', { x: 0.78, y: 6.55, w: 4.2, h: 0.3, fontSize: 13, color: C.sub, margin: 0 });
check(slide);

slide = pptx.addSlide();
base(slide, 'The first payoff: generic length on a tree');
addText(slide, 'The call site should be boring. The tree opts into Foldable; the algorithm asks for the generic summary interface.', 0.75, 1.15, 11.8, 0.6, 18);
addCode(slide, snippets.length, 0.9, 2.2, 6.0, 1.25, 17);
addBullets(slide, ['one generic algorithm', 'no tree-specific overload at the call site', 'lawful summary interface, not ad hoc traversal'], 7.4, 2.25, 4.9, 2.0, 18);
check(slide);

slide = pptx.addSlide();
base(slide, 'Not a virtual interface');
addBullets(slide, ['not inheritance-based polymorphism', 'not type-erased polymorphism', 'not just ADL customization points', 'compile-time selection of named operations'], 0.9, 1.35, 5.3, 2.7, 19);
addText(slide, 'A typeclass object is a structured record of operations with defaults and laws. Dispatch is static; the interface remains open.', 6.85, 1.45, 5.5, 1.6, 20, C.ink);
addText(slide, 'Wrong model: vtable. Better model: visible, named, law-oriented operations selected by type.', 6.85, 3.45, 5.25, 1.1, 18, C.sub);
check(slide);

slide = pptx.addSlide();
base(slide, 'Three API layers');
addCode(slide, 't = push_left(t, x);\nauto v = view_right(t);\nauto m = measure(t);', 0.8, 1.25, 3.8, 1.5, 13);
addText(slide, 'domain API', 0.8, 2.9, 3.8, 0.35, 17, C.accent);
addCode(slide, 'auto n = std::ranges::distance(t);\nauto r = t | std::views::filter(p);', 4.85, 1.25, 3.8, 1.5, 13);
addText(slide, 'ranges API', 4.85, 2.9, 3.8, 0.35, 17, C.accent);
addCode(slide, 'auto n = smd::length(t);\nauto y = smd::invoke(f, tx, ty);\nauto z = smd::traverse(g, t);', 8.9, 1.25, 3.8, 1.5, 13);
addText(slide, 'typeclass API', 8.9, 2.9, 3.8, 0.35, 17, C.accent);
addText(slide, 'The layers are complementary: domain operations, iteration interop, and lawful generic structure.', 1.0, 4.55, 11.2, 0.6, 20, C.ink);
check(slide);

slide = pptx.addSlide();
base(slide, 'Why not just ranges?');
addBullets(slide, ['ranges answer: how do I iterate this?', 'Foldable answers: how do I summarize this lawfully?', 'ranges are operational', 'Foldable is algebraic'], 0.9, 1.25, 5.4, 2.65, 20);
addCode(slide, 'auto n1 = std::ranges::fold_left(r, 0, std::plus<>{});\nauto n2 = smd::fold_map(count_one, tree);', 6.65, 1.55, 5.8, 1.55, 13);
addText(slide, 'A Foldable instance can be implemented using a range. The abstraction is still different.', 6.65, 3.55, 5.65, 0.85, 19, C.sub);
check(slide);

slide = pptx.addSlide();
base(slide, 'Foldable: fold_map is the center');
addCode(slide, 'template <class FUNCTION, class STRUCTURE>\nconstexpr decltype(auto) fold_map(FUNCTION&& f, STRUCTURE&& x);', 0.95, 1.35, 6.2, 1.15, 14);
addBullets(slide, ['semantic primitive: fold_map', 'derived: length, to_vector, fold_left, fold_right', 'monoids make summaries composable', 'tree order is part of the instance contract'], 7.6, 1.35, 4.8, 2.6, 18);
addText(slide, 'Counting, collecting, measuring, and searching are all summaries.', 1.0, 4.1, 11.2, 0.7, 20, C.ink);
check(slide);

slide = pptx.addSlide();
base(slide, 'The public Applicative API is invoke');
addText(slide, 'Applicative should read like ordinary function application lifted into structure.', 0.9, 1.15, 11.4, 0.5, 19, C.ink);
addCode(slide, snippets.invoke, 0.95, 2.05, 7.0, 3.55, 12);
addBullets(slide, ['pure function', 'structured arguments', 'structured result', 'public API avoids exposing ap as the headline'], 8.45, 2.15, 3.9, 2.4, 17);
check(slide);

slide = pptx.addSlide();
base(slide, 'Tree Applicative is a design choice');
addBullets(slide, ['trees admit multiple plausible semantics', 'primary instance: shape-preserving zip', 'alternate semantics should be alternate maps', 'do not hide flattening or expansion behind the primary instance'], 0.9, 1.25, 5.6, 3.1, 18);
addText(slide, 'Primary for this proof: same shape in, same shape out.', 7.0, 1.45, 5.2, 0.8, 23, C.ink);
addCode(slide, 'auto z = smd::invoke(f, tx, ty);\n// matching tree shapes -> tree result\n// mismatched tree shapes -> no result', 7.0, 2.65, 5.25, 1.55, 14);
check(slide);

slide = pptx.addSlide();
base(slide, 'Bad tree Applicative example');
addText(slide, 'Compiles is not the same as a good primary instance.', 0.9, 1.15, 11.5, 0.5, 21, C.ink);
addCode(slide, snippets.bad, 0.95, 2.0, 7.2, 1.55, 14);
addBullets(slide, ['may be useful as another operation', 'violates the shape-preserving story', 'belongs behind a different explicit map'], 8.55, 2.0, 3.8, 2.0, 17);
check(slide);

slide = pptx.addSlide();
base(slide, 'Traversable: shape plus positions');
addBullets(slide, ['a traversable value behaves like shape + ordered positions', 'traverse visits positions in order', 'effects happen in that order', 'the same shape is rebuilt'], 0.9, 1.25, 5.7, 3.1, 19);
addCode(slide, snippets.traverse, 6.85, 1.35, 5.55, 3.4, 11);
check(slide);

slide = pptx.addSlide();
base(slide, 'Physical source rules');
addBullets(slide, ['classic include guards, not pragma once', 'canonical angle-bracket includes', 'no routine forward declarations', 'adaptation headers separate from datatypes', 'tests include the header under test twice'], 0.9, 1.25, 5.85, 3.5, 17);
addCode(slide, '#ifndef INCLUDE_SMD_TREE_FIX_TREE_HPP\n#define INCLUDE_SMD_TREE_FIX_TREE_HPP\n\n#include <smd/tree/fix_tree.hpp>\n\n#endif  // INCLUDE_SMD_TREE_FIX_TREE_HPP', 7.05, 1.3, 5.25, 2.4, 13);
addText(slide, 'Slides hide this boilerplate. The repository keeps it.', 7.05, 4.15, 5.2, 0.55, 18, C.sub);
check(slide);

slide = pptx.addSlide();
base(slide, 'Package contents');
addBullets(slide, ['compiling example files with UUID transclusion anchors', 'minimal fix_tree and typeclass scaffold', 'optional Applicative test instance using beman::optional::optional', 'org file with transclusion commands', 'Copilot / Claude rules for finishing the implementation'], 0.9, 1.25, 6.2, 3.3, 18);
addText(slide, 'The artifact is slideware, but the source snippets are real code.', 7.25, 1.5, 4.9, 1.0, 23, C.ink);
check(slide);

pptx.writeFile({ fileName: out });
