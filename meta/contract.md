# Build Contract

## Content rules

- Keep lesson text only in `src/book/**`.
- Keep workflow, repository, and build notes out of lesson markdown.
- Keep user-facing headings and page labels in markdown content.
- Lesson markdown may use `:::details`, `:::note`, and `:::warning` blocks,
  including nested combinations when a troubleshooting block needs it.
- Lesson headings may use explicit anchors with `## Heading {#anchor-id}`.
  Same-page links such as `[text](#anchor-id)` must remain local.
- Preserve `docs/backup/index.html` as the manual reference copy.

## Page rules

- `docs/index.html` is the generated overview page.
- The overview title comes from the first `# ...` heading in
  `src/book/chapter-01.md` or the source path passed to the build script.
- Page order comes from the numbered list in the chapter index markdown.
- For list item `N`, the generator resolves `src/book/chapter-01/NN-*.md`
  or `N-*.md`.
- Standalone page filenames must match source markdown basenames and start with
  a numeric prefix.
- Standalone page and navigation labels come from each page's first `# ...`
  heading.
- Each standalone page includes generated top and bottom navigation with
  previous, index, and next links.

## Asset and build rules

- Keep generated output in `docs/`.
- Keep syntax highlighting assets in `docs/assets/highlightjs/`.
- Edit shared styles in `src/styles/book.css`; treat `docs/assets/book.css` as
  generated output.
- Keep global shell behavior in `src/templates/page.html`.
- Do not depend on `temp/` at runtime.
- For manual Windows builds, use `scripts\build-book.cmd`.

## Maintenance rules

- Keep `meta/` current after repository structure, build, or design changes.
- Keep service documentation compact and in English.
- Do not edit `src/book/**`, generated HTML, generated CSS, or license text
  during metadata cleanup tasks.
