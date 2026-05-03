#!/usr/bin/env python3
"""Conservative source-to-source prototype for simple Fortran array assignments."""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path


ASSIGNMENT_RE = re.compile(
    r"^(?P<indent>\s*)(?P<lhs>[A-Za-z_]\w*(?:\s*\(\s*:\s*\))?)\s*=\s*"
    r"(?P<left>[^!]+?)\s*(?P<op>[+*])\s*(?P<right>[^!]+?)\s*(?P<comment>!.*)?$"
)

ARRAY_SECTION_RE = re.compile(r"^(?P<name>[A-Za-z_]\w*)\s*\(\s*:\s*\)$")
NAME_RE = re.compile(r"^[A-Za-z_]\w*$")
NUMBER_RE = re.compile(r"^[+-]?(\d+(\.\d*)?|\.\d+)([EeDd][+-]?\d+)?$")


@dataclass
class Operand:
    text: str
    is_array: bool


@dataclass
class TransformResult:
    lines: list[str]
    skipped: list[str]


def split_code_comment(line: str) -> tuple[str, str]:
    if "!" not in line:
        return line, ""
    code, comment = line.split("!", 1)
    return code, "!" + comment


def normalize_lhs(lhs: str) -> str | None:
    lhs = lhs.strip()
    section = ARRAY_SECTION_RE.match(lhs)
    if section:
        return section.group("name")
    if NAME_RE.match(lhs):
        return lhs
    return None


def parse_operand(text: str, default_array: bool) -> Operand | None:
    text = text.strip()
    section = ARRAY_SECTION_RE.match(text)
    if section:
        return Operand(section.group("name"), True)

    if NUMBER_RE.match(text):
        return Operand(text, False)

    if NAME_RE.match(text):
        # Heuristic for the lab prototype: bare names are arrays in A = B + C,
        # except names that clearly advertise scalar intent.
        lowered = text.lower()
        is_scalar_name = lowered in {"scalar", "alpha", "beta"} or lowered.startswith("scalar")
        return Operand(text, default_array and not is_scalar_name)

    return None


def render_operand(operand: Operand, index_name: str) -> str:
    if operand.is_array:
        return f"{operand.text}({index_name})"
    return operand.text


def skip(skipped: list[str], line_number: int, line: str, reason: str) -> None:
    code, _ = split_code_comment(line.rstrip("\n"))
    if code.strip():
        skipped.append(f"line {line_number}: skipped '{code.strip()}': {reason}")


def transform_line(line: str, line_number: int, skipped: list[str]) -> list[str]:
    if not line.strip() or line.lstrip().startswith("!"):
        return [line]

    match = ASSIGNMENT_RE.match(line.rstrip("\n"))
    if not match:
        code, _ = split_code_comment(line.rstrip("\n"))
        if "=" in code and ("+" in code or "*" in code):
            skip(skipped, line_number, line, "not a simple array assignment")
        return [line]

    indent = match.group("indent")
    lhs_name = normalize_lhs(match.group("lhs"))
    if not lhs_name:
        skip(skipped, line_number, line, "left-hand side is not a whole array")
        return [line]

    left_text = match.group("left").strip()
    right_text = match.group("right").strip()
    op = match.group("op")
    comment = match.group("comment") or ""

    if any(token in left_text + right_text for token in ("(", ")", ",", "%")):
        if not ARRAY_SECTION_RE.match(left_text) and not ARRAY_SECTION_RE.match(right_text):
            skip(skipped, line_number, line, "contains calls, components, or complex indexing")
            return [line]

    lhs_used_section = ARRAY_SECTION_RE.match(match.group("lhs").strip()) is not None
    default_array = True
    left = parse_operand(left_text, default_array)
    right = parse_operand(right_text, default_array)

    if left is None or right is None:
        skip(skipped, line_number, line, "right-hand side is not a simple binary expression")
        return [line]

    # A(:) = B(:) + C(:) is explicit; A = B + scalar is accepted by scalar-name
    # heuristic. Mixed literal/name expressions stay conservative and readable.
    if lhs_used_section and (not left.is_array or not right.is_array):
        skip(skipped, line_number, line, "array section assignment mixed with scalar operand")
        return [line]

    index_name = "i"
    assignment = (
        f"{indent}  {lhs_name}({index_name}) = "
        f"{render_operand(left, index_name)} {op} {render_operand(right, index_name)}"
    )
    if comment:
        assignment += f"  {comment.strip()}"

    return [
        f"{indent}! transformed by transform_fortran.py: explicit loop avoids an array temporary\n",
        f"{indent}do {index_name} = 1, size({lhs_name})\n",
        assignment + "\n",
        f"{indent}end do\n",
    ]


def transform_source(source: str) -> TransformResult:
    output: list[str] = []
    skipped: list[str] = []
    for line_number, line in enumerate(source.splitlines(keepends=True), start=1):
        output.extend(transform_line(line, line_number, skipped))
    return TransformResult(output, skipped)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Rewrite very simple Fortran array assignments as explicit DO loops."
    )
    parser.add_argument("input", type=Path, help="Fortran source file to transform")
    parser.add_argument(
        "-o", "--output", type=Path, help="Output Fortran file; stdout is used if omitted"
    )
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    source = args.input.read_text(encoding="utf-8")
    result = transform_source(source)
    transformed = "".join(result.lines)

    if args.output:
        args.output.write_text(transformed, encoding="utf-8")
    else:
        sys.stdout.write(transformed)

    for message in result.skipped:
        print(message, file=sys.stderr)

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
