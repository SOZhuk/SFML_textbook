from __future__ import annotations

import html
import re
import shutil
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / "docs"
SRC = ROOT / "src"


def inline(text: str) -> str:
    placeholders: list[str] = []

    def stash(value: str) -> str:
        placeholders.append(value)
        return f"@@P{len(placeholders) - 1}@@"

    text = re.sub(
        r"\[(.+?)\]\((.+?)\)",
        lambda m: stash(
            f'<a href="{html.escape(m.group(2), quote=True)}">{html.escape(m.group(1))}</a>'
            if m.group(2).startswith("#")
            else f'<a href="{html.escape(m.group(2), quote=True)}" target="_blank" rel="noreferrer">{html.escape(m.group(1))}</a>'
        ),
        text,
    )
    text = re.sub(r"\*\*(.+?)\*\*", lambda m: stash(f"<strong>{html.escape(m.group(1))}</strong>"), text)
    text = re.sub(r"_(.+?)_", lambda m: stash(f"<em>{html.escape(m.group(1))}</em>"), text)
    text = re.sub(r"`(.+?)`", lambda m: stash(f"<code>{html.escape(m.group(1))}</code>"), text)
    text = html.escape(text)
    for i, value in enumerate(placeholders):
        text = text.replace(f"@@P{i}@@", value)
    return text


def heading_html(level: int, text: str) -> str:
    match = re.match(r"^(.*?)\s+\{#([A-Za-z0-9_-]+)\}$", text)
    if match:
        title = match.group(1).strip()
        anchor = html.escape(match.group(2), quote=True)
        return f'<h{level} id="{anchor}">{inline(title)}</h{level}>'
    return f"<h{level}>{inline(text)}</h{level}>"


def render_markdown(source: str, drop_first_h1: bool = False) -> str:
    lines = source.splitlines()
    body: list[str] = []
    in_list: str | None = None
    in_pre = False
    pre_lines: list[str] = []
    pre_lang = "cpp"
    in_details = False
    details_title = ""
    details_lines: list[str] = []
    in_callout = False
    callout_kind = "note"
    callout_title = ""
    callout_lines: list[str] = []
    first_h1_dropped = False

    def flush_list() -> None:
        nonlocal in_list
        if in_list:
            body.append(f"</{in_list}>")
            in_list = None

    def flush_details() -> None:
        nonlocal in_details, details_title, details_lines
        if in_details:
            details_body = render_markdown("\n".join(details_lines))
            body.append(
                f'<details class="details-block"><summary>{inline(details_title)}</summary>'
                f'<div class="details-body">{details_body}</div></details>'
            )
            in_details = False
            details_title = ""
            details_lines = []

    def flush_callout() -> None:
        nonlocal in_callout, callout_kind, callout_title, callout_lines
        if in_callout:
            callout_body = render_markdown("\n".join(callout_lines))
            title = f'<strong class="note-title">{inline(callout_title)}</strong>' if callout_title else ""
            body.append(f'<aside class="note note-{callout_kind}">{title}{callout_body}</aside>')
            in_callout = False
            callout_kind = "note"
            callout_title = ""
            callout_lines = []

    for line in lines:
        raw = line.rstrip()
        check = raw.lstrip()

        if in_callout:
            if check == ":::":
                flush_callout()
            else:
                callout_lines.append(raw)
            continue

        if in_details:
            if check == ":::":
                flush_details()
            else:
                details_lines.append(raw)
            continue

        if in_pre:
            if check.startswith("```"):
                body.append(f'<pre><code class="language-{pre_lang}">{html.escape(chr(10).join(pre_lines))}</code></pre>')
                pre_lines = []
                in_pre = False
            else:
                pre_lines.append(raw)
            continue

        if check.startswith("```"):
            flush_list()
            in_pre = True
            pre_lines = []
            pre_lang = check[3:].strip() or "cpp"
            continue

        if check.startswith(":::details"):
            flush_list()
            in_details = True
            details_title = check[len(":::details") :].strip() or "Додаткові команди"
            details_lines = []
            continue

        callout_match = re.match(r"^:::(note|warning)\b(.*)$", check)
        if callout_match:
            flush_list()
            in_callout = True
            callout_kind = callout_match.group(1)
            callout_title = callout_match.group(2).strip()
            callout_lines = []
            continue

        if not check:
            flush_list()
            continue

        if check.startswith("# "):
            if drop_first_h1 and not first_h1_dropped:
                first_h1_dropped = True
                flush_list()
                continue
            flush_list()
            body.append(heading_html(1, check[2:].strip()))
        elif check.startswith("## "):
            flush_list()
            body.append(heading_html(2, check[3:].strip()))
        elif check.startswith("### "):
            flush_list()
            body.append(heading_html(3, check[4:].strip()))
        elif check.startswith("- "):
            if in_list != "ul":
                flush_list()
                body.append("<ul>")
                in_list = "ul"
            body.append(f"<li>{inline(check[2:].strip())}</li>")
        elif re.match(r"^\d+\.\s+", check):
            if in_list != "ol":
                flush_list()
                body.append("<ol>")
                in_list = "ol"
            body.append(f"<li>{inline(re.sub(r'^\d+\.\s+', '', check))}</li>")
        else:
            flush_list()
            body.append(f"<p>{inline(check)}</p>")

    flush_list()
    if in_pre:
        body.append(f'<pre><code class="language-{pre_lang}">{html.escape(chr(10).join(pre_lines))}</code></pre>')
    flush_details()
    flush_callout()
    return "\n".join(body)


def render_page(template: str, title: str, body: str) -> str:
    return template.replace("{{title}}", title).replace("{{body}}", body)


def wrap_assets(body: str) -> str:
    return body + '\n<script src="assets/highlightjs/highlight.min.js"></script>\n<script>hljs.highlightAll();</script>'


def page_output_name(page: dict) -> str:
    output = Path(page["source"]).with_suffix(".html").name
    if not re.match(r"^\d+", output):
        raise ValueError(f"Page output must start with a number: {output}")
    return output


def extract_first_h1(markdown: str) -> str | None:
    for line in markdown.splitlines():
        check = line.strip()
        if check.startswith("# "):
            return check[2:].strip()
    return None


def split_numbered_heading(heading: str | None) -> tuple[str | None, str | None]:
    if not heading:
        return None, None
    match = re.match(r"^(\d+)\.\s+(.+)$", heading)
    if match:
        return match.group(1), match.group(2).strip()
    return None, heading


def parse_index_page_order(markdown: str) -> list[dict]:
    order: list[dict] = []
    for line in markdown.splitlines():
        check = line.strip()
        match = re.match(r"^(\d+)\.\s+(.+)$", check)
        if match:
            order.append({"number": str(int(match.group(1))), "title": match.group(2).strip()})
    if not order:
        raise ValueError("No ordered list found in chapter index markdown.")
    return order


def find_page_source(chapter_dir: Path, number: str) -> Path:
    num = int(number)
    matches = sorted(chapter_dir.glob(f"{num:02d}-*.md"))
    if not matches:
        matches = sorted(chapter_dir.glob(f"{num}-*.md"))
    if not matches:
        raise FileNotFoundError(f"Missing source markdown for page number {number} in {chapter_dir}")
    return matches[0]


def build_page_models(chapter_dir: Path, page_order: list[dict]) -> list[dict]:
    models: list[dict] = []
    for idx, page_order_item in enumerate(page_order):
        source_path = find_page_source(chapter_dir, page_order_item["number"])
        source_text = source_path.read_text(encoding="utf-8")
        heading = extract_first_h1(source_text)
        h_number, h_title = split_numbered_heading(heading)

        filename_number_match = re.match(r"^(\d+)", source_path.name)
        filename_number = filename_number_match.group(1) if filename_number_match else None

        number = h_number or page_order_item["number"] or filename_number or str(idx + 1)
        title = h_title or page_order_item["title"] or source_path.stem
        source_rel = source_path.relative_to(SRC / "book").as_posix()

        models.append(
            {
                "number": number,
                "title": title,
                "source": source_rel,
                "source_text": source_text,
            }
        )
    return models


def nav_html(pages: list[dict], current_idx: int) -> str:
    items = []
    if current_idx > 0:
        prev = pages[current_idx - 1]
        items.append(f'<a href="{page_output_name(prev)}">Prev {prev["number"]}. {html.escape(prev["title"])}</a>')
    items.append('<a href="index.html">Index</a>')
    if current_idx + 1 < len(pages):
        nxt = pages[current_idx + 1]
        items.append(f'<a href="{page_output_name(nxt)}">Next {nxt["number"]}. {html.escape(nxt["title"])}</a>')
    return '<nav class="page-nav">' + "".join(items) + "</nav>"


def main() -> int:
    source_path = Path(sys.argv[1]).resolve()
    output_path = Path(sys.argv[4]).resolve()
    template = Path(sys.argv[2]).resolve().read_text(encoding="utf-8")
    styles_path = Path(sys.argv[3]).resolve()
    index_markdown = source_path.read_text(encoding="utf-8")
    index_heading = extract_first_h1(index_markdown) or "SFML: перший інтерактивний проєкт на C++"
    chapter_dir = source_path.parent / source_path.stem
    page_order = parse_index_page_order(index_markdown)
    pages = build_page_models(chapter_dir, page_order)

    assets_dir = DOCS / "assets"
    assets_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(styles_path, assets_dir / "book.css")

    index_body = (
        '<div class="wrap">\n'
        '<section class="hero">\n'
        f"<h1>{html.escape(index_heading)}</h1>\n"
        '</section>\n'
        '<section class="card">\n'
        '<ul class="index-list">\n'
        + "".join(
            f'<li><a href="{page_output_name(page)}">{page["number"]}. {html.escape(page["title"])}</a></li>\n'
            for page in pages
        )
        + "</ul>\n"
        "</section>\n"
        "</div>"
    )
    output_path.write_text(
        render_page(template, index_heading, wrap_assets(index_body)),
        encoding="utf-8",
    )

    for idx, page in enumerate(pages):
        source = page["source_text"]
        output_name = page_output_name(page)
        body = (
            '<div class="wrap">\n'
            f"{nav_html(pages, idx)}\n"
            '<section class="hero">\n'
            f'<h1>{page["number"]}. {html.escape(page["title"])}</h1>\n'
            "</section>\n"
            f'<section class="card">{render_markdown(source, drop_first_h1=True)}</section>\n'
            f"{nav_html(pages, idx)}\n"
            "</div>"
        )
        (DOCS / output_name).write_text(
            render_page(template, f'{page["number"]}. {page["title"]}', wrap_assets(body)),
            encoding="utf-8",
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
