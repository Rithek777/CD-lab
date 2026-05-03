#!/usr/bin/env python3
"""Performance evaluation harness for the Flang allocation profiler project."""

from __future__ import annotations

import argparse
import json
import os
import platform
import shutil
import subprocess
import sys
import time
from pathlib import Path
from statistics import mean


ROOT = Path(__file__).resolve().parents[1]
EXAMPLES = ROOT / "examples"
BUILD_DIR = ROOT / "build" / "evaluation"
REPORT_DIR = ROOT / "report"
TRANSFORMER = ROOT / "scripts" / "transform_fortran.py"
MALLOC_COUNTER = ROOT / "scripts" / "malloc_counter.c"

BENCHMARKS = [
    "benchmark_array_temp.f90",
    "benchmark_allocatable_realloc.f90",
    "benchmark_function_result.f90",
]


def run_command(command: list[str], **kwargs) -> subprocess.CompletedProcess:
    return subprocess.run(command, text=True, capture_output=True, **kwargs)


def find_fortran_compiler(preferred: str | None) -> str | None:
    candidates = [preferred] if preferred else []
    candidates += ["gfortran", "flang-new", "flang"]
    for candidate in candidates:
        if candidate and shutil.which(candidate):
            return candidate
    return None


def compiler_output_name(source: Path, label: str) -> Path:
    suffix = ".exe" if platform.system() == "Windows" else ""
    return BUILD_DIR / f"{source.stem}_{label}{suffix}"


def transform_source(source: Path, output: Path) -> tuple[bool, str]:
    command = [sys.executable, str(TRANSFORMER), str(source), "-o", str(output)]
    result = run_command(command)
    return result.returncode == 0, result.stderr.strip()


def compile_source(compiler: str, source: Path, executable: Path) -> tuple[bool, str]:
    command = [compiler, "-O2", str(source), "-o", str(executable)]
    result = run_command(command)
    details = (result.stdout + result.stderr).strip()
    return result.returncode == 0, details


def run_executable(executable: Path, runs: int, env: dict[str, str] | None = None) -> dict:
    timings: list[float] = []
    stdout = ""
    stderr = ""

    for _ in range(runs):
      start = time.perf_counter()
      result = run_command([str(executable)], env=env)
      elapsed = time.perf_counter() - start
      timings.append(elapsed)
      stdout = result.stdout
      stderr = result.stderr
      if result.returncode != 0:
          return {
              "status": "failed",
              "mean_seconds": None,
              "runs": timings,
              "stdout": stdout.strip(),
              "stderr": stderr.strip(),
          }

    return {
        "status": "ok",
        "mean_seconds": mean(timings),
        "runs": timings,
        "stdout": stdout.strip(),
        "stderr": stderr.strip(),
    }


def build_malloc_counter() -> dict:
    if platform.system() == "Windows":
        return {
            "available": False,
            "reason": "LD_PRELOAD malloc counting is not available on Windows",
            "library": None,
        }

    cc = shutil.which("gcc") or shutil.which("clang")
    if not cc:
        return {
            "available": False,
            "reason": "no C compiler found for malloc_counter.c",
            "library": None,
        }

    library = BUILD_DIR / "malloc_counter.so"
    command = [cc, "-shared", "-fPIC", str(MALLOC_COUNTER), "-o", str(library), "-ldl"]
    result = run_command(command)
    if result.returncode != 0:
        return {
            "available": False,
            "reason": (result.stdout + result.stderr).strip(),
            "library": None,
        }

    return {"available": True, "reason": "", "library": str(library)}


def run_allocation_tools(executable: Path, malloc_counter: dict) -> dict:
    tools: dict[str, dict] = {}

    usr_bin_time = Path("/usr/bin/time")
    if usr_bin_time.exists():
        result = run_command([str(usr_bin_time), "-v", str(executable)])
        tools["usr_bin_time"] = {
            "available": True,
            "returncode": result.returncode,
            "stderr": result.stderr.strip(),
        }
    else:
        tools["usr_bin_time"] = {
            "available": False,
            "reason": "/usr/bin/time is unavailable",
        }

    if shutil.which("valgrind"):
        massif_out = BUILD_DIR / f"{executable.stem}.massif.out"
        result = run_command(
            ["valgrind", "--tool=massif", f"--massif-out-file={massif_out}", str(executable)]
        )
        tools["valgrind_massif"] = {
            "available": True,
            "returncode": result.returncode,
            "output_file": str(massif_out),
            "stderr": result.stderr.strip(),
        }
    else:
        tools["valgrind_massif"] = {
            "available": False,
            "reason": "valgrind is unavailable",
        }

    if malloc_counter.get("available"):
        env = os.environ.copy()
        env["LD_PRELOAD"] = str(malloc_counter["library"])
        result = run_command([str(executable)], env=env)
        tools["ld_preload_malloc_counter"] = {
            "available": True,
            "returncode": result.returncode,
            "stderr": result.stderr.strip(),
        }
    else:
        tools["ld_preload_malloc_counter"] = {
            "available": False,
            "reason": malloc_counter.get("reason", "malloc counter unavailable"),
        }

    return tools


def evaluate_benchmark(source: Path, compiler: str | None, runs: int, malloc_counter: dict) -> dict:
    transformed = BUILD_DIR / f"transformed_{source.name}"
    transform_ok, transform_log = transform_source(source, transformed)

    result = {
        "name": source.stem,
        "source": str(source.relative_to(ROOT)),
        "transformed_source": str(transformed.relative_to(ROOT)),
        "transform": {
            "status": "ok" if transform_ok else "failed",
            "messages": transform_log,
        },
        "original": {},
        "transformed": {},
    }

    if compiler is None:
        placeholder = {
            "status": "skipped",
            "reason": "no Fortran compiler found; install gfortran or flang",
        }
        result["original"] = placeholder
        result["transformed"] = placeholder
        return result

    for label, path in [("original", source), ("transformed", transformed)]:
        executable = compiler_output_name(source, label)
        compile_ok, compile_log = compile_source(compiler, path, executable)
        entry = {
            "source": str(path.relative_to(ROOT)),
            "executable": str(executable.relative_to(ROOT)),
            "compile": {
                "status": "ok" if compile_ok else "failed",
                "messages": compile_log,
            },
        }

        if compile_ok:
            entry["run"] = run_executable(executable, runs)
            entry["allocation_tools"] = run_allocation_tools(executable, malloc_counter)
        else:
            entry["run"] = {
                "status": "skipped",
                "reason": "compilation failed",
            }
            entry["allocation_tools"] = {
                "status": "skipped",
                "reason": "compilation failed",
            }

        result[label] = entry

    return result


def write_json_report(report: dict) -> None:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    (REPORT_DIR / "evaluation_results.json").write_text(
        json.dumps(report, indent=2), encoding="utf-8"
    )


def format_seconds(value: float | None) -> str:
    if value is None:
        return "unavailable"
    return f"{value:.6f}"


def write_markdown_report(report: dict) -> None:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    lines = [
        "# Evaluation Results",
        "",
        f"- Compiler: `{report['compiler'] or 'unavailable'}`",
        f"- Runs per executable: {report['runs']}",
        f"- Platform: {report['platform']}",
        "",
    ]

    for benchmark in report["benchmarks"]:
        lines += [
            f"## {benchmark['name']}",
            "",
            f"- Source: `{benchmark['source']}`",
            f"- Transformed source: `{benchmark['transformed_source']}`",
            f"- Transform status: {benchmark['transform']['status']}",
        ]
        if benchmark["transform"]["messages"]:
            lines.append(f"- Transform messages: `{benchmark['transform']['messages']}`")

        lines += [
            "",
            "| Version | Compile | Run Status | Mean Seconds |",
            "| --- | --- | --- | ---: |",
        ]
        for label in ["original", "transformed"]:
            entry = benchmark[label]
            compile_status = entry.get("compile", {}).get("status", entry.get("status", "skipped"))
            run_status = entry.get("run", {}).get("status", entry.get("status", "skipped"))
            mean_seconds = format_seconds(entry.get("run", {}).get("mean_seconds"))
            lines.append(f"| {label} | {compile_status} | {run_status} | {mean_seconds} |")

        lines += ["", "### Allocation Tools", ""]
        for label in ["original", "transformed"]:
            entry = benchmark[label]
            lines.append(f"- {label}:")
            tools = entry.get("allocation_tools", {})
            if not isinstance(tools, dict):
                lines.append(f"  - {tools}")
                continue
            for tool_name, tool_result in tools.items():
                if isinstance(tool_result, dict) and tool_result.get("available"):
                    lines.append(f"  - {tool_name}: available")
                elif isinstance(tool_result, dict):
                    lines.append(f"  - {tool_name}: unavailable ({tool_result.get('reason', 'not run')})")
            if not tools:
                lines.append("  - unavailable")
        lines.append("")

    (REPORT_DIR / "evaluation_results.md").write_text("\n".join(lines), encoding="utf-8")


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run Fortran benchmark evaluation.")
    parser.add_argument("--compiler", help="Fortran compiler command to use")
    parser.add_argument("--runs", type=int, default=3, help="Runs per executable")
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    if args.runs <= 0:
        raise SystemExit("--runs must be positive")

    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    compiler = find_fortran_compiler(args.compiler)
    malloc_counter = build_malloc_counter()

    report = {
        "compiler": compiler,
        "runs": args.runs,
        "platform": platform.platform(),
        "malloc_counter": malloc_counter,
        "benchmarks": [
            evaluate_benchmark(EXAMPLES / name, compiler, args.runs, malloc_counter)
            for name in BENCHMARKS
        ],
    }

    write_json_report(report)
    write_markdown_report(report)

    print(f"Wrote {REPORT_DIR / 'evaluation_results.md'}")
    print(f"Wrote {REPORT_DIR / 'evaluation_results.json'}")
    if compiler is None:
        print("No Fortran compiler found; reports contain placeholder results.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

