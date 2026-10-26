#!/usr/bin/env python3
"""Small local delivery adapter; ESP-IDF remains the builder/flasher."""
import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_BOARD = "jinyichen-esp32-c3-lcd-2.4"
DEFAULT_APP = "touch-counter"


def resolve_selection(board_id, app_id, root=ROOT):
    """Validate known native app requirements against declared board capabilities."""
    for name in (board_id, app_id):
        if not re.fullmatch(r"[a-z0-9]+(?:[.-][a-z0-9]+)*", name):
            raise ValueError("invalid board/app id: " + name)
    manifest = None
    for path in (root / "hardware").glob("*/*/board.json"):
        board = json.loads(path.read_text())
        if board.get("id") == board_id:
            manifest = path
            break
    if manifest is None:
        raise ValueError("unknown board: " + board_id)
    if not (manifest.parent / "board/CMakeLists.txt").is_file():
        raise ValueError("board has no implemented binding: " + board_id)
    app_dir = root / "apps" / app_id
    if not (app_dir / "app.json").is_file():
        raise ValueError("unknown app: " + app_id)
    app = json.loads((app_dir / "app.json").read_text())
    if app.get("id") != app_id or app.get("format") != "native-c-v1":
        raise ValueError("unsupported app manifest: " + app_id)
    capabilities = {"display": bool(board.get("display")), "input": bool(board.get("touch"))}
    for required in app["requires"]:
        if not capabilities.get(required, False):
            raise ValueError("board lacks required capability: " + required)
    for dimension in ("width", "height"):
        minimum = app.get("display", {}).get("min_" + dimension, 0)
        if board.get("display", {}).get(dimension, 0) < minimum:
            raise ValueError("display " + dimension + " is below app minimum " + str(minimum))
    return manifest.parent, app_dir


def command(action, board_id, app_id, port=None):
    resolve_selection(board_id, app_id)
    if action not in ("build", "flash", "monitor"):
        raise ValueError("unsupported action: " + action)
    if action in ("flash", "monitor") and not port:
        raise ValueError("--port is required; use 'ports' to list devices")
    cmd = ["idf.py", "-C", str(ROOT / "firmware"),
           "-B", str(ROOT / "firmware/build" / (board_id + "--" + app_id)),
           "-DDADA_BOARD=" + board_id, "-DDADA_APP=" + app_id]
    if port:
        cmd += ["-p", port]
    return cmd + [action]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("build", "flash", "monitor", "ports", "check"))
    parser.add_argument("--board", default=DEFAULT_BOARD)
    parser.add_argument("--app", default=DEFAULT_APP)
    parser.add_argument("--port")
    args = parser.parse_args()
    try:
        if args.action == "ports":
            # Pyserial comes with ESP-IDF's Python environment.
            return subprocess.call([sys.executable, "-m", "serial.tools.list_ports", "-v"])
        if args.action == "check":
            resolve_selection(args.board, args.app)
            return 0
        cmd = command(args.action, args.board, args.app, args.port)
        if not shutil.which("idf.py"):
            parser.error("idf.py not found; activate ESP-IDF v5.4.2 with export.sh / export.ps1 first")
        return subprocess.call(cmd)
    except (ValueError, OSError, KeyError, TypeError) as exc:
        parser.error(str(exc))


if __name__ == "__main__":
    sys.exit(main())
