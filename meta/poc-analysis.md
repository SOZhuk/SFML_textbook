# POC Analysis: Minimal-Change Content Generation Workflow

## Usage note

This file is historical context only. Do not read it during routine implementation or planning unless the task specifically concerns the original PoC, early architecture decisions, or repository history. For current rules, use `meta/contract.md`, `meta/structure-note.md`, and `meta/design-system.md`.

## Current repository structure

- `docs/index.html` is the only tracked site artifact in the repository.
- There is no visible source tree yet for chapters, templates, shared CSS, or build scripts.
- The current GitHub Pages output is therefore a single-page static textbook, not a multi-file content pipeline.

## How the book is currently organized

- The textbook content lives directly inside `docs/index.html`.
- The page is structured as one long sequence of `<section class="card">` blocks after a hero section.
- Each chapter is already expressed as a numbered section with headings, paragraphs, lists, notes, and code blocks.
- This means the content is semantically chunked, even though it is physically stored in one file.

## Shared assets and repeated HTML structure

### Shared assets

- All styling is embedded in the `<style>` block inside `docs/index.html`.
- There are no external CSS, JS, image, or template assets in the current repo.
- Links to external resources are hard-coded directly in the page.

### Repeated structure

- The page repeats the same layout pattern many times:
  - a `section.card`
  - an `h2` chapter title
  - narrative paragraphs
  - optional `ul` / `ol` lists
  - optional `pre > code` examples
  - optional `.note` callouts
- The visual language is consistent and simple, which is good for templating later.
- The current repetition suggests the content would benefit from chapter partials or source fragments, but not a full framework.

### Navigation pattern

- There is no visible multi-page navigation in the current HTML.
- The page reads as a scrollable linear lesson, which is compatible with a source-driven build that simply concatenates chapters.
- Because there is no existing nav system to preserve, the safest first step is to keep the single-page reading model.

## Minimal PoC workflow

### Goal

- Keep `docs/` as the published output for GitHub Pages.
- Store source content outside `docs/`.
- Generate only one chapter semi-automatically first.
- Avoid introducing a heavy framework or rewriting the whole project.

### Suggested directory structure

```text
.
├─ docs/                      # published GitHub Pages output
│  ├─ index.html
│  └─ assets/                 # optional future static assets
├─ src/                       # editable source content
│  ├─ book/
│  │  ├─ chapter-01.md        # source chapter content
│  │  └─ chapter-02.md
│  ├─ templates/
│  │  └─ page.html            # minimal HTML shell
│  └─ styles/
│     └─ book.css             # extracted shared CSS
├─ scripts/
│  └─ build-book.ps1          # simple generator for docs/
└─ meta/
   └─ poc-analysis.md         # this file
```

### PoC build shape

- Keep the current output model: one generated `docs/index.html`.
- Move chapter text into `src/book/chapter-01.md` first, leaving the rest in the current HTML until the process is proven.
- Use a small PowerShell script to:
  - read a template HTML shell
  - inject shared CSS
  - convert one markdown chapter into HTML
  - write the result to `docs/index.html`

### Semi-automatic chapter flow

- Start with one chapter only, for example `chapter-01.md`.
- Manually preserve the existing HTML shell and styling initially.
- Generate the chapter body into the page body from source text.
- If markdown conversion is too limited at first, allow the PoC to use a very small custom format or pre-authored HTML fragments.

## Recommended workflow options

### Option A: Markdown + simple template

- Source chapters in Markdown.
- Use a minimal local script to convert Markdown to HTML and wrap it in the existing layout.
- Best when you want easy editing and low tooling overhead.

### Option B: HTML fragments + template

- Source chapters remain small HTML partials outside `docs/`.
- A build script assembles them into the final page.
- Best when preserving exact legacy HTML behavior matters more than authoring convenience.

### Recommendation

- Start with Option B if compatibility with the legacy textbook is the top priority.
- Use Option A only if Markdown conversion quality is acceptable and the chapter content stays simple.

## Risks and assumptions

### Assumptions

- The project is meant to stay static and work on GitHub Pages without server-side rendering.
- The current `docs/index.html` is representative of the intended visual style.
- Future content will still be mostly chapter-based, not database-driven or interactive.

### Risks

- Encoding risk: the current file appears to have text encoding issues in the terminal view, so the build workflow must explicitly preserve UTF-8.
- Compatibility risk: automatic Markdown conversion may not preserve legacy HTML details exactly.
- Maintenance risk: if authors keep editing `docs/index.html` directly, source and output will drift.
- Scope risk: introducing too much automation too early could complicate a simple textbook site.

## Concrete next step for the PoC

- Add a `src/` tree with one chapter source and one template.
- Add a very small build script that writes to `docs/index.html`.
- Keep the published site identical or nearly identical after generation.
- Once one chapter works end-to-end, expand the same pattern to the remaining chapters.

## Status update

- The repository now has the initial source scaffold in `src/`, `scripts/`, and `docs/assets/`.
- The original page was preserved at `docs/backup/index.html` for manual comparison.
- `docs/index.html` is the overview page.
- `docs/setup.html` and `docs/project.html` are standalone chapter pages.
- The first chapter text is split into source parts under `src/book/chapter-01/`.
- `scripts/build-book.py` now generates the overview page plus standalone pages in the order defined by the numbered list in `src/book/chapter-01.md`.
- Index and standalone page labels are now sourced from markdown `#` headings (content-driven titles, no hardcoded UI text in build script).
- Standalone HTML filenames are now derived from source markdown basenames and must start with numbers.
- `highlight.js` now lives in `docs/assets/highlightjs/` and is loaded by the generated pages.
- `prompts.md` holds safe prefix prompts for future vague instructions.
- `meta/contract.md` records the current page and asset rules.
- Standalone pages now keep one page-level `H1` and omit duplicate content-level `H1`.
- Navigation pills were tightened for compact horizontal usage.

## Final status note

This document is kept as historical PoC analysis. The current implementation has moved beyond the initial one-page proof of concept: the book is now a generated multi-page static textbook with source markdown in `src/book/`, a shared template in `src/templates/page.html`, shared source styles in `src/styles/book.css`, generated output in `docs/`, and a two-theme Noir/Graphite design system documented separately in `meta/design-system.md`.
