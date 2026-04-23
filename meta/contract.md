# Build Contract

## Content rules

- Keep lesson text in the source chapter parts.
- Do not place workflow notes, repository notes, or build planning inside lesson content.
- Keep all user-facing headings and page labels in markdown content.
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
- Keep shared book styles in `docs/assets/book.css`.
- Do not depend on `temp/` at runtime.

## Maintenance rules

- Update `meta/` after each iteration.
- Keep structural notes out of lesson markdown.
- Keep markdown documentation in English.
