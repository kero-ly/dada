#!/usr/bin/env python3
"""Validate board manifests under hardware/.

Two levels:
  1. JSON Schema (contracts/schema/board.schema.json)
  2. Semantic rules from docs/adr/0002-board-manifest.md that a schema cannot express

Usage:
    python tools/validate_manifests.py                # scan hardware/
    python tools/validate_manifests.py path/to/*.json # explicit files
    python tools/validate_manifests.py --strict       # warnings become errors

Exit code: 0 ok, 1 validation failed.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any, Iterable

REPO_ROOT = Path(__file__).resolve().parent.parent
SCHEMA_PATH = REPO_ROOT / "contracts" / "schema" / "board.schema.json"

# Leaves of the manifest that describe the document itself, not the hardware,
# so they do not need provenance entries.
PROVENANCE_EXEMPT = {
    "manifest_version",
    "id",
    "status",
    "hardware_revision",
    "verified_capabilities",
    "provenance",
    "source_url",
}

# Capability -> condition that must hold for the claim to be plausible.
def capability_problem(cap: str, manifest: dict[str, Any]) -> str | None:
    if cap == "display":
        if manifest.get("display") is None:
            return "claims 'display' but display is null"
    elif cap in ("touch", "input"):
        if manifest.get("touch") is None:
            return f"claims '{cap}' but touch is null"
    elif cap == "usb-serial-jtag":
        usb = manifest.get("usb") or {}
        if usb.get("mode") != "serial-jtag":
            return "claims 'usb-serial-jtag' but usb.mode is not 'serial-jtag'"
    return None


def leaves(node: Any, prefix: str = "") -> Iterable[tuple[str, Any]]:
    """Yield (dotted_path, value) for every leaf, descending into dicts only."""
    if isinstance(node, dict):
        for key, value in node.items():
            path = f"{prefix}.{key}" if prefix else key
            if isinstance(value, dict):
                yield from leaves(value, path)
            else:
                yield path, value
    else:
        yield prefix, node


def check_manifest(path: Path, schema: dict[str, Any]) -> tuple[list[str], list[str]]:
    errors: list[str] = []
    warnings: list[str] = []

    try:
        manifest = json.loads(path.read_text(encoding="utf-8-sig"))
    except json.JSONDecodeError as exc:
        return [f"invalid JSON: {exc}"], []

    # --- layer 1: JSON Schema -------------------------------------------------
    try:
        import jsonschema
    except ImportError:
        return ["jsonschema is not installed; run: pip install -r tools/requirements.txt"], []

    validator = jsonschema.Draft202012Validator(schema)
    for err in sorted(validator.iter_errors(manifest), key=lambda e: list(e.path)):
        where = ".".join(str(p) for p in err.path) or "<root>"
        errors.append(f"schema: {where}: {err.message}")

    if errors:
        return errors, warnings

    # --- layer 2: semantic rules ---------------------------------------------
    rel: tuple[str, ...] = ()
    if REPO_ROOT in path.parents:
        rel = path.relative_to(REPO_ROOT).parts
    if len(rel) >= 4 and rel[0] == "hardware":
        vendor, board_dir = rel[1], rel[2]
        expected_id = f"{vendor}-{board_dir}"
        if manifest["id"] != expected_id:
            errors.append(
                f"id '{manifest['id']}' does not match directory "
                f"hardware/{vendor}/{board_dir} (expected '{expected_id}')"
            )
    else:
        warnings.append(f"not under hardware/<vendor>/<board>/: {path}")

    status = manifest["status"]
    caps = manifest["verified_capabilities"]
    if status == "verified" and not caps:
        errors.append("status is 'verified' but verified_capabilities is empty")
    if status in ("planned", "purchased") and caps:
        errors.append(
            f"status is '{status}' but verified_capabilities is non-empty: {caps}"
        )

    for cap in caps:
        problem = capability_problem(cap, manifest)
        if problem:
            errors.append(f"capability: {problem}")

    # provenance coverage: every non-null leaf must be covered by a provenance key
    prov = manifest.get("provenance") or {}
    for leaf_path, value in leaves(manifest):
        if value is None:
            continue
        top = leaf_path.split(".")[0]
        if top in PROVENANCE_EXEMPT:
            continue
        # `notes` is free-form commentary, not a hardware fact
        if leaf_path.split(".")[-1] == "notes":
            continue
        covered = any(
            key == leaf_path or leaf_path.startswith(key + ".") for key in prov
        )
        if not covered:
            errors.append(f"provenance: '{leaf_path}' has a value but no evidence level")

    for key in prov:
        if not any(
            leaf == key or leaf.startswith(key + ".")
            for leaf, value in leaves(manifest)
            if value is not None
        ):
            warnings.append(f"provenance: '{key}' does not match any non-null field")

    # duplicate GPIO check: the same pin claimed twice is almost always a typo
    used: dict[int, str] = {}
    for section, prefix in (("display", "display.pins"), ("touch", "touch.pins")):
        block = manifest.get(section) or {}
        for name, pin in (block.get("pins") or {}).items():
            if not isinstance(pin, int) or pin < 0:
                continue
            if pin in used:
                errors.append(
                    f"gpio: pin {pin} used by both {used[pin]} and {prefix}.{name}"
                )
            else:
                used[pin] = f"{prefix}.{name}"

    return errors, warnings


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("paths", nargs="*", type=Path, help="manifest files (default: hardware/**/board.json)")
    parser.add_argument("--strict", action="store_true", help="treat warnings as errors")
    args = parser.parse_args()

    schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8-sig"))
    paths = args.paths or sorted((REPO_ROOT / "hardware").rglob("board.json"))
    if not paths:
        print("error: no manifests found", file=sys.stderr)
        return 1

    failed = 0
    for path in paths:
        path = path.resolve()
        errors, warnings = check_manifest(path, schema)
        label = path.relative_to(REPO_ROOT) if REPO_ROOT in path.parents else path
        if errors:
            failed += 1
            print(f"FAIL {label}")
            for item in errors:
                print(f"     error: {item}")
        else:
            print(f"OK   {label}")
        for item in warnings:
            print(f"     warn:  {item}")
        if args.strict and warnings:
            failed += 1

    print(f"\n{len(paths) - failed}/{len(paths)} manifest(s) valid")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
