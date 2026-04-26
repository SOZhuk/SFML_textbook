# Source Workspace

This directory is the source of truth for generated book pages.

- `book/chapter-01.md` stores the chapter index title and page order.
- `book/chapter-01/` stores lesson markdown files (`NN-*.md`).
- `templates/page.html` stores the shared HTML shell, including the theme toggle.
- `styles/book.css` stores the shared design system and page styles.

The published site in `docs/` is generated output. Do not edit generated HTML or `docs/assets/book.css` directly.

## Build

On Windows, run the build through the command wrapper:

```cmd
scripts\build-book.cmd
```

The wrapper launches `scripts/build-book.ps1` with a process-local execution policy bypass, so it does not require changing the system-wide PowerShell policy.

## Current design

The book uses the shared Noir Amber / Graphite School theme system from `styles/book.css`. The theme defaults from the browser preference and can be toggled from the icon button defined in `templates/page.html`.

For project rules and design constraints, see:

- `../meta/contract.md`
- `../meta/structure-note.md`
- `../meta/design-system.md`
