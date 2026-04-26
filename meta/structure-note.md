# Current Structure Note

This repository keeps editable textbook sources separate from the published
GitHub Pages output.

## Source and output

- `src/book/chapter-01.md` is the chapter index: its first `#` heading becomes
  the main page title, and its numbered list defines lesson order.
- `src/book/chapter-01/NN-*.md` stores the lesson source pages. Filenames must
  start with numeric prefixes.
- `src/templates/page.html` is the shared HTML shell.
- `src/styles/book.css` is the source stylesheet.
- `docs/` is published output. Do not hand-edit generated HTML or
  `docs/assets/book.css`.
- `docs/backup/index.html` is the manual reference copy.

## Generated pages

- `docs/index.html` is a compact generated overview page.
- Standalone lesson pages are generated from source markdown basenames, for
  example `src/book/chapter-01/01-example.md` becomes `docs/01-example.html`.
- Each standalone page has one page-level `H1` plus top and bottom navigation
  with previous, index, and next links.
- Chapter 1 currently publishes nine standalone lesson pages.

## Build and assets

- Run manual Windows builds with `scripts\build-book.cmd`.
- `scripts/build-book.py` reads markdown headings for visible labels, resolves
  page order by numeric source prefixes, and renders supported markdown blocks.
- Supported lesson blocks include `:::details`, `:::note`, `:::warning`, nested
  block pairs, and explicit heading anchors such as `## Title {#anchor-id}`.
- Published syntax highlighting assets live in `docs/assets/highlightjs/`.
- Teaching assets live in `docs/static/`; asset credits are in
  `docs/static/credits.md`.

## Maintenance

- `meta/contract.md` is the authoritative build and content contract.
- `meta/design-system.md` is the authoritative design constraint note.
- `meta/poc-analysis.md` is historical archive only.
- `prompts.md` stores short reusable prompts, including the periodic metadata
  cleanup prompt used by `scripts/audit-meta.py`.
