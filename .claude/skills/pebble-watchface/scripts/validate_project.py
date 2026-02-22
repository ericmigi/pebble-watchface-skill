#!/usr/bin/env python3
"""Validate an Alloy (Moddable) Pebble watchface project."""

import json
import re
import sys
from pathlib import Path

REQUIRED_FILES = [
    "package.json",
    "wscript",
    "src/c/mdbl.c",
    "src/embeddedjs/main.js",
    "src/embeddedjs/manifest.json",
    "src/pkjs/index.js",
]

UUID_RE = re.compile(r"^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$", re.I)


def fail(message: str) -> None:
    print(f"✗ {message}")


def ok(message: str) -> None:
    print(f"✓ {message}")


def validate_package_json(project: Path) -> int:
    errors = 0
    pkg = json.loads((project / "package.json").read_text(encoding="utf-8"))
    pebble = pkg.get("pebble", {})

    checks = [
        (pkg.get("name"), "package.json name"),
        (pebble.get("displayName"), "pebble.displayName"),
        (pebble.get("projectType") == "moddable", "pebble.projectType is moddable"),
        (pebble.get("enableMultiJS") is True, "pebble.enableMultiJS is true"),
        (pebble.get("targetPlatforms") == ["emery", "gabbro"], "targetPlatforms are [emery, gabbro]"),
        (pebble.get("watchapp", {}).get("watchface") is True, "watchface flag is true"),
    ]

    for passed, label in checks:
        if passed:
            ok(label)
        else:
            fail(f"Invalid/missing {label}")
            errors += 1

    uid = pebble.get("uuid", "")
    if UUID_RE.match(uid):
        ok("UUID format valid")
    else:
        fail("Invalid pebble.uuid")
        errors += 1

    return errors


def main() -> None:
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} /path/to/project")
        sys.exit(1)

    project = Path(sys.argv[1]).resolve()
    errors = 0

    for rel in REQUIRED_FILES:
        p = project / rel
        if p.exists():
            ok(f"{rel} exists")
        else:
            fail(f"{rel} missing")
            errors += 1

    if (project / "package.json").exists():
        try:
            errors += validate_package_json(project)
        except Exception as exc:
            fail(f"Could not parse package.json: {exc}")
            errors += 1

    if errors:
        print(f"\nValidation failed with {errors} issue(s)")
        sys.exit(1)

    print("\nValidation passed")


if __name__ == "__main__":
    main()
