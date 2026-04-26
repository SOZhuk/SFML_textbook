from __future__ import annotations

import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
LIMIT_BYTES = 12 * 1024
PROMPT_HEADING = "## Metadata cleanup"


def tracked_paths() -> list[Path]:
    paths: list[Path] = []
    paths.extend(sorted((ROOT / "meta").glob("*.md")))
    paths.extend(
        [
            ROOT / "prompts.md",
            ROOT / "src" / "README.md",
            ROOT / "docs" / "static" / "credits.md",
        ]
    )
    return [path for path in paths if path.is_file()]


def rel(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def git_commit_count() -> int | None:
    try:
        result = subprocess.run(
            ["git", "rev-list", "--count", "HEAD"],
            cwd=ROOT,
            check=True,
            capture_output=True,
            text=True,
        )
    except (OSError, subprocess.CalledProcessError):
        return None

    try:
        return int(result.stdout.strip())
    except ValueError:
        return None


def metadata_prompt() -> str:
    prompts_path = ROOT / "prompts.md"
    if not prompts_path.is_file():
        return ""

    lines = prompts_path.read_text(encoding="utf-8").splitlines()
    for index, line in enumerate(lines):
        if line.strip() != PROMPT_HEADING:
            continue

        block: list[str] = []
        for next_line in lines[index + 1 :]:
            if next_line.startswith("## "):
                break
            if next_line.strip():
                block.append(next_line.strip())
        return "\n".join(block).strip()

    return ""


def main() -> int:
    limit_bytes = LIMIT_BYTES
    if len(sys.argv) == 3 and sys.argv[1] == "--limit-bytes":
        try:
            limit_bytes = int(sys.argv[2])
        except ValueError:
            print("--limit-bytes expects an integer", file=sys.stderr)
            return 2
    elif len(sys.argv) != 1:
        print("usage: audit-meta.py [--limit-bytes N]", file=sys.stderr)
        return 2

    paths = tracked_paths()
    rows = [(rel(path), path.stat().st_size) for path in paths]
    total = sum(size for _, size in rows)
    commit_count = git_commit_count()

    reasons: list[str] = []
    if commit_count is not None and commit_count > 0 and commit_count % 10 == 0:
        reasons.append(f"commit count is {commit_count}, divisible by 10")
    if total > limit_bytes:
        reasons.append(f"scoped metadata is {total} bytes, above {limit_bytes} bytes")

    print("Metadata audit scope")
    print("--------------------")
    for name, size in sorted(rows, key=lambda item: item[1], reverse=True):
        print(f"{name:<32} {size:>6} bytes")

    print()
    print(f"Total: {total} bytes")
    print(f"Limit: {limit_bytes} bytes")
    print(
        "Git commits: "
        + (str(commit_count) if commit_count is not None else "unavailable")
    )

    print()
    if reasons:
        print("maintenance due: yes")
        for reason in reasons:
            print(f"- {reason}")
        prompt = metadata_prompt()
        if prompt:
            print()
            print("Maintenance prompt:")
            print(prompt)
    else:
        print("maintenance due: no")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
