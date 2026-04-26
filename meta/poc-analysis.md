# PoC Analysis Archive

Status: historical. Do not use this file for routine implementation rules.

The original proof of concept moved the textbook from a single hand-edited
`docs/index.html` page toward a static source pipeline. Its lasting decisions
were:

- keep GitHub Pages output under `docs/`;
- keep editable lesson sources under `src/book/`;
- generate pages with a small local script instead of a framework;
- preserve a manual reference copy at `docs/backup/index.html`;
- document active build and design rules in `meta/`.

The active implementation has moved beyond the PoC. Current rules live in:

- `meta/contract.md` for content, page, asset, build, and maintenance rules;
- `meta/structure-note.md` for the current repository shape;
- `meta/design-system.md` for visual and interaction constraints.

Read this archive only when the task specifically concerns early architecture
history or why the project chose a minimal static generation workflow.
