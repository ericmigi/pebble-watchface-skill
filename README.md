# Pebble Alloy Watchface Skill (SDK v127)

This repository contains a Codex/Claude skill for generating **Pebble Alloy (Moddable) JavaScript watchfaces**.

## What changed

- Migrated skill guidance from classic C-first Pebble apps to **Alloy (Moddable) JS-first** flow.
- Locked target platforms to **emery** and **gabbro**.
- Added downloaded upstream references from the rePebble blog links:
  - `alloy-watchface-tutorial`
  - `pebble-examples`
- Added a new sample project: **3D globe + major timezone cities**.

## Skill location

- `.claude/skills/pebble-watchface/SKILL.md`

## New sample project

- `samples/projects/globe-timezones-alloy/`

This sample renders a pseudo-3D rotating globe with seven hardcoded cities and local time offsets.

## Quick start

```bash
cd samples/projects/globe-timezones-alloy
npm install
pebble build
pebble install --emulator emery
```

## References downloaded from announcement links

- `.claude/skills/pebble-watchface/reference/downloads/alloy-watchface-tutorial`
- `.claude/skills/pebble-watchface/reference/downloads/pebble-examples`

## Notes

- This skill intentionally does **not** target aplite/basalt/chalk/diorite.
- The watchface logic lives in `src/embeddedjs/main.js` and uses `commodetto/Poco`.


## Binary assets policy

To keep PRs compatible with binary-restricted review tooling, vendored tutorial/example folders in `reference/downloads/` keep source/code files only (no `.png`, `.ttf`, `.pdc`, or other binary assets).
