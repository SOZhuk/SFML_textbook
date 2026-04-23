# Source Workspace

This directory is the authoring space for book content.

- `book/chapter-01.md` stores chapter index title and page order.
- `book/chapter-01/` stores page markdown files (`NN-*.md`).
- `templates/` stores the generated HTML shell.
- `styles/` stores shared CSS extracted from `docs/`.

The published site in `docs/` is generated output.

## Build

On Windows, run the build through the command wrapper:

```cmd
scripts\build-book.cmd
```

The wrapper launches `scripts/build-book.ps1` with a process-local execution policy bypass, so it does not require changing the system-wide PowerShell policy.
