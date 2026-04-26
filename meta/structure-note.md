# Current Structure Note

The repository now includes a minimal source scaffold alongside the published `docs/` site.

## Added paths

- `src/book/chapter-01.md`
- `src/templates/page.html`
- `src/styles/book.css`
- `scripts/build-book.ps1`
- `src/README.md`
- `docs/assets/.gitkeep`

## Intent

- Keep `docs/` as the published output.
- Prepare a source tree for chapter-based content generation.
- Leave existing HTML content untouched until the build workflow is ready.

## Current state

- `docs/backup/index.html` preserves the original page for reference.
- `docs/index.html` is the generated main overview page.
- `docs/` is generated from `src/`; do not edit generated HTML or `docs/assets/book.css` by hand.
- Standalone lesson pages are generated with numbered names based on source markdown basenames:
  `01-nalashtuvannia-seredovyshcha.html`, `02-pershyi-proiekt-sfml.html`,
  `03-vstup-i-zahalna-informatsiia.html`, `04-struktura-i-tekhnichni-detali.html`,
  `05-obiekty-renderinhu.html`, `06-navchalnyi-proekt-bazova-kulka.html`,
  `07-navchalnyi-proekt-pole-kolir-i-keruvannia.html`,
  `08-navchalnyi-proekt-kilka-kulok.html`,
  `09-dodatkovi-idei-indykatory-spraity-i-fon.html`.
- `src/book/chapter-01/` contains the chapter source parts.
- `scripts/build-book.py` regenerates the overview page and the standalone pages.
- `scripts/build-book.cmd` is the recommended Windows entry point for manual builds because it runs `scripts/build-book.ps1` with a process-local PowerShell execution policy bypass.
- `scripts/build-book.py` now reads index and page headings from markdown `# ...` lines, so user-visible titles are content-driven.
- `scripts/build-book.py` now takes page order from the numbered list in `src/book/chapter-01.md` and resolves files by numeric filename prefix in `src/book/chapter-01/`.
- `docs/assets/highlightjs/` holds the published highlight.js assets used by the generated pages.
- Shared page styles are sourced from `src/styles/book.css` and delivered as generated `docs/assets/book.css`.
- `prompts.md` contains short prefix prompts for safer future instructions.
- `meta/contract.md` defines the current content and page rules.
- Standalone pages now render a single `H1` only (no duplicated heading inside the content card).
- Standalone page navigation is compact, horizontally scrollable, and styled with aggressive asymmetric control radii.
- Ordered-list rendering now strips markdown numeric prefixes from list item text to avoid doubled numbering in HTML.
- Lesson markdown supports collapsible blocks with `:::details Title` ... `:::`; the generator renders them as HTML `<details>` blocks.
- Lesson markdown supports callout blocks with `:::note Title` ... `:::` and `:::warning Title` ... `:::`; the generator renders them as styled `<aside>` panels using `.note` variants.
- Lesson markdown supports explicit heading anchors with `## Heading {#anchor-id}` and same-page links to those anchors.
- Chapter 1 now has nine standalone lesson pages. The former large page 6 was split into pages 6-9:
  `06-navchalnyi-proekt-bazova-kulka.md`,
  `07-navchalnyi-proekt-pole-kolir-i-keruvannia.md`,
  `08-navchalnyi-proekt-kilka-kulok.md`,
  `09-dodatkovi-idei-indykatory-spraity-i-fon.md`.
- Pages 6-9 are step-by-step student worksheets with incremental snippets, intermediate results, and extension tasks. Public downloadable teaching assets live in `docs/static/`, including simple sprites with opaque green key-color backgrounds, a star background, a Noto Sans font, and five `docs/static/animation/octopus*.png` frames with normalized green key color.
- Main index links are now asymmetric menu controls with accent borders, strong hover states, and focus-visible styling.
- The current design system has two themes: Noir Amber and Graphite School.
- The shared template includes one day/night icon theme toggle. It defaults from browser `prefers-color-scheme` and stores manual choices in `localStorage`.
- Shape is controlled through CSS variables such as `--radius-panel`, `--radius-control`, `--radius-code`, `--radius-warning`, and `--radius-marker`.
- Warning blocks are intentionally more prominent than regular notes/details and use dedicated warning color variables per theme.
