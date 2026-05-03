#!/usr/bin/env python3
"""Local web UI server for the Flang implicit allocation profiler."""

from __future__ import annotations

import argparse
import json
import mimetypes
import subprocess
import tempfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse


ROOT = Path(__file__).resolve().parents[1]
UI_DIR = ROOT / "ui"
TESTS_DIR = ROOT / "tests"
TEMP_INPUT_DIR = ROOT / "build" / "ui-inputs"


def find_profiler() -> Path | None:
    names = [
        ROOT / "build-mingw" / "flang-implicit-alloc-profiler.exe",
        ROOT / "build" / "flang-implicit-alloc-profiler",
        ROOT / "build" / "flang-implicit-alloc-profiler.exe",
        ROOT / "flang-implicit-alloc-profiler.exe",
    ]
    for path in names:
        if path.exists():
            return path
    return None


def json_response(handler: BaseHTTPRequestHandler, status: int, payload: dict) -> None:
    body = json.dumps(payload, indent=2).encode("utf-8")
    handler.send_response(status)
    handler.send_header("Content-Type", "application/json; charset=utf-8")
    handler.send_header("Content-Length", str(len(body)))
    handler.end_headers()
    handler.wfile.write(body)


def read_json_body(handler: BaseHTTPRequestHandler) -> dict:
    length = int(handler.headers.get("Content-Length", "0"))
    if length <= 0:
        return {}
    return json.loads(handler.rfile.read(length).decode("utf-8"))


def safe_sample_path(name: str) -> Path | None:
    path = (TESTS_DIR / name).resolve()
    if TESTS_DIR.resolve() not in path.parents or path.suffix != ".mlir":
        return None
    if not path.exists():
        return None
    return path


class UiHandler(BaseHTTPRequestHandler):
    server_version = "FAPUi/0.1"

    def log_message(self, fmt: str, *args) -> None:
        return

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        if parsed.path == "/api/samples":
            samples = sorted(path.name for path in TESTS_DIR.glob("*.mlir"))
            json_response(self, 200, {"samples": samples})
            return

        if parsed.path == "/api/sample":
            params = parse_qs(parsed.query)
            path = safe_sample_path(params.get("name", [""])[0])
            if path is None:
                json_response(self, 404, {"error": "sample not found"})
                return
            json_response(self, 200, {"name": path.name, "content": path.read_text(encoding="utf-8")})
            return

        if parsed.path == "/":
            self.serve_file(UI_DIR / "index.html")
            return

        self.serve_file(UI_DIR / parsed.path.lstrip("/"))

    def do_POST(self) -> None:
        if urlparse(self.path).path != "/api/analyze":
            json_response(self, 404, {"error": "unknown endpoint"})
            return

        profiler = find_profiler()
        if profiler is None:
            json_response(
                self,
                500,
                {
                    "error": "profiler executable not found",
                    "hint": "build the project first with CMake",
                },
            )
            return

        try:
            body = read_json_body(self)
            mlir = str(body.get("mlir", ""))
            output_format = str(body.get("format", "json"))
            threshold_mb = float(body.get("thresholdMb", 0.0))
            show_ir = bool(body.get("showIr", False))
        except Exception as error:
            json_response(self, 400, {"error": f"invalid request: {error}"})
            return

        if not mlir.strip():
            json_response(self, 400, {"error": "MLIR input is empty"})
            return

        TEMP_INPUT_DIR.mkdir(parents=True, exist_ok=True)
        with tempfile.NamedTemporaryFile(
            "w", suffix=".mlir", dir=TEMP_INPUT_DIR, delete=False, encoding="utf-8"
        ) as temp_file:
            temp_file.write(mlir)
            input_path = Path(temp_file.name)

        command = [
            str(profiler),
            f"--format={output_format}",
            f"--threshold-mb={threshold_mb}",
        ]
        if show_ir:
            command.append("--show-ir")
        command.append(str(input_path))

        result = subprocess.run(command, text=True, capture_output=True, cwd=ROOT)
        payload = {
            "command": command,
            "returncode": result.returncode,
            "stdout": result.stdout,
            "stderr": result.stderr,
        }
        if output_format == "json" and result.returncode == 0:
            try:
                payload["report"] = json.loads(result.stdout)
            except json.JSONDecodeError:
                payload["report"] = None

        json_response(self, 200 if result.returncode == 0 else 500, payload)

    def serve_file(self, path: Path) -> None:
        path = path.resolve()
        if UI_DIR.resolve() not in path.parents and path != (UI_DIR / "index.html").resolve():
            self.send_error(403)
            return
        if not path.exists() or not path.is_file():
            self.send_error(404)
            return

        content_type = mimetypes.guess_type(path.name)[0] or "application/octet-stream"
        body = path.read_bytes()
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Serve the local profiler UI.")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8765)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    server = ThreadingHTTPServer((args.host, args.port), UiHandler)
    print(f"UI running at http://{args.host}:{args.port}")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping UI server.")
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

