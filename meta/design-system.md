# Design System Note

## Themes

- The book has two supported themes: Noir Amber (`data-theme="noir"`) and Graphite School (`data-theme="graphite"`).
- Noir Amber is the dark cinematic default for dark browser preference. Graphite School is the lighter classroom-friendly alternative for light browser preference.
- The first page load reads `prefers-color-scheme` unless `localStorage["book-theme"]` contains a saved manual choice.
- The shared template exposes one icon-only day/night toggle. It toggles between the two themes and updates `aria-label`, `title`, and `aria-pressed`.

## Source of truth

- Edit design tokens and component styling in `src/styles/book.css`.
- Treat `docs/assets/book.css` as generated output.
- Keep theme toggle markup and JavaScript in `src/templates/page.html`.
- Do not add design controls or workflow notes to lesson markdown.

## Shape language

- The current visual language uses aggressive asymmetric radii, not clipped corners.
- Asymmetry means uneven `border-radius` values such as `2px 9px` or `4px 14px`.
- Do not reintroduce `clip-path` for the primary panel/control shape unless there is a deliberate new design direction.
- Current shape tokens:
  - `--radius-panel`
  - `--radius-control`
  - `--radius-code`
  - `--radius-warning`
  - `--radius-marker`

## Readability rules

- The primary audience is students, and pages are code-heavy. Text contrast, stable layout, and readable code blocks are more important than decoration.
- Code blocks should remain dark, bordered, scrollable horizontally, and visually distinct in both themes.
- Inline code should stay compact and high contrast without pulling attention away from surrounding text.

## Interaction rules

- Links, index items, navigation controls, details summaries, and the theme toggle need clear hover and focus-visible states.
- Hover states may use strong amber/yellow color changes and small movement on interactive elements.
- Do not animate or move large reading panels on hover.

## Warning policy

- `:::warning` callouts render as `.note.note-warning` and must be more visually prominent than regular notes or details.
- Warning blocks use dedicated variables: `--warning-bg`, `--warning-border`, `--warning-text`, and `--warning-accent`.
- Keep the thick left/top border treatment and the `!` marker unless replacing it with an equally prominent warning treatment.
