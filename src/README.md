# Source Workspace

`src/` is the editable source for generated book pages.

- `book/chapter-01.md` defines the chapter title and lesson order.
- `book/chapter-01/` stores lesson markdown files named `NN-*.md`.
- `templates/page.html` stores the shared page shell.
- `styles/book.css` stores the shared design system and page styles.

The published site in `docs/` is generated output. Do not edit generated HTML
or `docs/assets/book.css` directly.

## Build

On Windows, run:

```cmd
scripts\build-book.cmd
```

For current rules, see `../meta/contract.md`, `../meta/structure-note.md`, and
`../meta/design-system.md`.
