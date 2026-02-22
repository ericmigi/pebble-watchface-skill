#!/usr/bin/env python3
"""Create a new Alloy (Moddable) Pebble watchface project."""

import argparse
import json
import shutil
import sys
import uuid
from pathlib import Path


def slugify(name: str) -> str:
    return name.lower().replace(' ', '-').replace('_', '-')


def write_json(path: Path, data: dict) -> None:
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def create_package_json(project_path: Path, name: str, display_name: str, author: str) -> None:
    content = {
        "name": slugify(name),
        "author": author,
        "version": "1.0.0",
        "keywords": ["pebble-app"],
        "private": True,
        "dependencies": {"@moddable/pebbleproxy": "^0.1.8"},
        "pebble": {
            "displayName": display_name,
            "uuid": str(uuid.uuid4()),
            "projectType": "moddable",
            "sdkVersion": "3",
            "enableMultiJS": True,
            "targetPlatforms": ["emery", "gabbro"],
            "watchapp": {"watchface": True},
            "messageKeys": [],
            "resources": {"media": []},
        },
    }
    write_json(project_path / "package.json", content)


def create_wscript(project_path: Path, skill_path: Path) -> None:
    shutil.copy(skill_path / "templates" / "wscript.template", project_path / "wscript")


def create_gitignore(project_path: Path) -> None:
    project_path.joinpath('.gitignore').write_text('build/\nnode_modules/\n', encoding='utf-8')


def copy_templates(project_path: Path, skill_path: Path) -> None:
    embedded = project_path / "src" / "embeddedjs"
    c_dir = project_path / "src" / "c"
    pkjs = project_path / "src" / "pkjs"
    embedded.mkdir(parents=True, exist_ok=True)
    c_dir.mkdir(parents=True, exist_ok=True)
    pkjs.mkdir(parents=True, exist_ok=True)

    shutil.copy(skill_path / "templates" / "alloy-watchface.js", embedded / "main.js")
    shutil.copy(skill_path / "templates" / "alloy-manifest.json", embedded / "manifest.json")
    shutil.copy(skill_path / "templates" / "alloy-mdbl.c", c_dir / "mdbl.c")
    shutil.copy(skill_path / "templates" / "alloy-pkjs-index.js", pkjs / "index.js")


def main() -> None:
    parser = argparse.ArgumentParser(description="Create a new Alloy Pebble watchface project")
    parser.add_argument("name", help="Project name")
    parser.add_argument("--author", default="Your Name", help="Author name")
    parser.add_argument("--display", default=None, help="Display name")
    parser.add_argument("--output", "-o", default=".", help="Output directory")
    args = parser.parse_args()

    project_name = slugify(args.name)
    project_path = Path(args.output).resolve() / project_name
    if project_path.exists():
        print(f"Error: directory already exists: {project_path}")
        sys.exit(1)

    skill_path = Path(__file__).resolve().parent.parent
    project_path.mkdir(parents=True)
    create_package_json(project_path, args.name, args.display or args.name, args.author)
    create_wscript(project_path, skill_path)
    create_gitignore(project_path)
    copy_templates(project_path, skill_path)

    print(f"✓ Created Alloy watchface project at {project_path}")
    print("Next: npm install && pebble build")


if __name__ == "__main__":
    main()
