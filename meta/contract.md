# Build Contract

## Content rules

- Keep lesson text in the source chapter parts.
- Do not place workflow notes, repository notes, or build planning inside lesson content.
- Keep all user-facing headings and page labels in markdown content.
- Lesson markdown may use callout blocks with `:::note Title` ... `:::` and `:::warning Title` ... `:::`. The generator renders them as styled note panels.
- Lesson markdown may nest `:::details`, `:::note`, and `:::warning` blocks inside one another when a troubleshooting block needs a collapsible subsection.
- Warning callouts must stay visually stronger than regular notes and details blocks, with high contrast in every supported theme.
- Lesson headings may use explicit anchors with `## Heading {#anchor-id}`. Internal links such as `[text](#anchor-id)` stay on the same page.
- Keep the published output in `docs/`.
- Preserve `docs/backup/index.html` as the manual reference copy.

## Page rules

- `docs/index.html` is the main overview page.
- Index page `<h1>` and `<title>` are taken from the first `# ...` heading in `src/book/chapter-01.md` (or the source path passed to the build script).
- Page order is taken from the numbered list in `src/book/chapter-01.md`.
- For each list item number `N`, the generator resolves source file `src/book/chapter-01/NN-*.md` (or `N-*.md`).
- Standalone page filenames must match source markdown basenames:
  `chapter-01/01-example.md` -> `docs/01-example.html`.
- Every standalone page filename must start with a numeric prefix.
- Standalone page title labels and navigation labels are taken from each page markdown first `# ...` heading.
- Recommended page heading format is `# N. Title` for consistent numbering in index and navigation.
- The main page is a compact vertical index with links only.
- Each standalone page must include top and bottom navigation with previous, index, and next links generated from the markdown index order.

## Asset rules

- Keep syntax highlighting assets in `docs/assets/highlightjs/`.
- Keep source styles in `src/styles/book.css`.
- Keep shared generated book styles in `docs/assets/book.css`; do not edit this generated copy directly.
- Keep global page shell behavior in `src/templates/page.html`, including the theme toggle.
- Do not depend on `temp/` at runtime.

## Design and theme rules

- The book supports two themes through `data-theme="noir"` and `data-theme="graphite"`.
- The initial theme follows `prefers-color-scheme` unless `localStorage["book-theme"]` contains a manual override.
- The theme toggle is a single icon button in the shared template. Do not place theme controls in lesson markdown.
- The shared CSS uses variables for colors, warning treatment, and asymmetric shape radii. Preserve readability for code-heavy student lessons.

## Build rules

- For manual Windows builds, prefer `scripts\build-book.cmd`; it wraps the PowerShell script with a process-local execution policy bypass.

## Maintenance rules

- Update `meta/` after each iteration.
- Keep structural notes out of lesson markdown.
- Keep markdown documentation in English.
