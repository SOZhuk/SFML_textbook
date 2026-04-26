# Design System Note

## Themes

- Supported themes are Noir Amber (`data-theme="noir"`) and Graphite School
  (`data-theme="graphite"`).
- The first load follows `prefers-color-scheme` unless
  `localStorage["book-theme"]` contains a manual choice.
- The shared template exposes one icon-only day/night toggle and keeps its
  `aria-label`, `title`, and `aria-pressed` state in sync.

## Source of truth

- Edit tokens and component styles in `src/styles/book.css`.
- Keep theme toggle markup and JavaScript in `src/templates/page.html`.
- Do not add design controls or workflow notes to lesson markdown.

## Shape and readability

- The visual language uses asymmetric `border-radius` values, not clipped
  corners.
- Shape tokens include `--radius-panel`, `--radius-control`, `--radius-code`,
  `--radius-warning`, and `--radius-marker`.
- Student lessons are code-heavy, so contrast, stable layout, and readable code
  blocks outrank decoration.
- Code blocks should remain dark, bordered, horizontally scrollable, and clear
  in both themes.

## Interaction and warnings

- Links, index items, navigation controls, details summaries, and the theme
  toggle need visible hover and focus states.
- Avoid hover motion on large reading panels.
- `:::warning` renders as `.note.note-warning` and must remain stronger than
  regular notes or details, using the dedicated warning color variables and a
  prominent marker treatment.
