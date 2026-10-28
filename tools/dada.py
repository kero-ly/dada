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


def resolve_selection(board_id, app_id, root=ROOT, allow_unverified=False):
    """Validate known native app requirements against declared board capabilities."""
    for name in (board_id, app_id):
        if not re.fullmatch(r"[a-z0-9]+(?:[.-][a-z0-9]+)*", name):
            raise ValueError("invalid board/app id: " + name)
    manifest = None
    for path in (root / "hardware").glob("*/*/board.json"):
        board = json.loads(path.read_text(encoding="utf-8-sig"))
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
    app = json.loads((app_dir / "app.json").read_text(encoding="utf-8-sig"))
    if app.get("id") != app_id or app.get("format") != "native-c-v1":
        raise ValueError("unsupported app manifest: " + app_id)
    # Missing verification data is never evidence of verification.
    verified = set(board.get("verified_capabilities") or [])
    declared_extra = set(board.get("declared_capabilities") or [])
    declared = {
        "display": bool(board.get("display")),
        "input": bool(board.get("touch")),
        "time": "time" in declared_extra,
        "storage": "storage" in declared_extra,
    }
    capabilities = {
        "display": "display" in verified,
        "input": "input" in verified or "touch" in verified,
        "time": "time" in verified,
        "storage": "storage" in verified,
    }
    for required in app["requires"]:
        if not declared.get(required, False):
            raise ValueError("board lacks declared capability: " + required)
        if not allow_unverified and not capabilities.get(required, False):
            raise ValueError("board capability is not hardware-verified: " + required +
                             " (pass --allow-unverified for bring-up)")
    for dimension in ("width", "height"):
        minimum = app.get("display", {}).get("min_" + dimension, 0)
        if board.get("display", {}).get(dimension, 0) < minimum:
            raise ValueError("display " + dimension + " is below app minimum " + str(minimum))
    return manifest.parent, app_dir


def pio_environment(app_id):
    """PlatformIO environment name for an app (declared in platformio.ini)."""
    return "dada-c3-" + app_id


def command(action, board_id, app_id, port=None, allow_unverified=False,
            use_pio=None):
    """Build the command for one action.

    PlatformIO is the documented baseline, so it wins when available; a
    hand-installed ESP-IDF stays supported as a fallback because the CMake
    project is still the source of truth. Pass use_pio to pin the choice (the
    tests do this so they do not depend on what is installed).
    """
    resolve_selection(board_id, app_id, allow_unverified=allow_unverified)
    if action not in ("build", "flash", "monitor"):
        raise ValueError("unsupported action: " + action)
    if action in ("flash", "monitor") and not port:
        raise ValueError("--port is required; use 'ports' to list devices")
    if use_pio is None:
        use_pio = shutil.which("pio") is not None

    if use_pio:
        if allow_unverified:
            # platformio.ini pins board and app per environment, so there is no
            # free-form CMake argument to append. Say so instead of silently
            # ignoring the flag.
            raise ValueError(
                "PlatformIO environments cannot take --allow-unverified; "
                "add -DDADA_ALLOW_UNVERIFIED=ON to board_build.cmake_extra_args "
                "in firmware/platformio.ini, or build with ESP-IDF instead")
        env = pio_environment(app_id)
        base = ["pio", "run", "-d", str(ROOT / "firmware"), "-e", env]
        if action == "build":
            return base
        if action == "flash":
            return base + ["--target", "upload", "--upload-port", port]
        return ["pio", "device", "monitor", "--port", port, "--baud", "115200"]

    cmd = ["idf.py", "-C", str(ROOT / "firmware"),
           "-B", str(ROOT / "firmware/build" / (board_id + "--" + app_id)),
           "-DDADA_BOARD=" + board_id, "-DDADA_APP=" + app_id]
    if allow_unverified:
        cmd.append("-DDADA_ALLOW_UNVERIFIED=ON")
    if port:
        cmd += ["-p", port]
    return cmd + [action]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("build", "flash", "monitor", "ports", "check"))
    parser.add_argument("--board", default=DEFAULT_BOARD)
    parser.add_argument("--app", default=DEFAULT_APP)
    parser.add_argument("--port")
    parser.add_argument("--allow-unverified", action="store_true",
                        help="allow declared but not yet hardware-verified capabilities (local bring-up only)")
    args = parser.parse_args()
    try:
        if args.action == "ports":
            # Pyserial comes with ESP-IDF's Python environment.
            return subprocess.call([sys.executable, "-m", "serial.tools.list_ports", "-v"])
        if args.action == "check":
            resolve_selection(args.board, args.app, allow_unverified=args.allow_unverified)
            return 0
        cmd = command(args.action, args.board, args.app, args.port, args.allow_unverified)
        if args.allow_unverified:
            print("warning: this only permits local bring-up; it does not certify hardware",
                  file=sys.stderr)
        if not shutil.which("idf.py"):
            parser.error("idf.py not found; activate a compatible ESP-IDF environment with export.sh / export.ps1 first")
        return subprocess.call(cmd)
    except (ValueError, OSError, KeyError, TypeError) as exc:
        parser.error(str(exc))


if __name__ == "__main__":
    sys.exit(main())
