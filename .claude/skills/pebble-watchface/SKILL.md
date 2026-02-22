---
name: pebble-watchface
description: Generate complete Pebble watchfaces using Alloy (Moddable JavaScript) and SDK v127 tooling. Use when building Pebble watchfaces/apps in pure JavaScript for emery and gabbro, including package.json, wscript, embeddedjs code, pkjs bridge, PBW build, emulator install, screenshots, and iteration.
---

# Pebble Alloy Watchface Generator

Generate complete, buildable Alloy (Moddable) watchfaces using JavaScript-first architecture.

## Non-negotiable defaults

- Target **SDK release 4.9.127** tooling.
- Use **Alloy/Moddable project type**, not Rocky.js.
- Target platforms are **only** `emery` and `gabbro`.
- Use JavaScript for watchface logic in `src/embeddedjs/main.js`.

## Required project structure

```text
watchface-name/
├── package.json
├── wscript
└── src/
    ├── c/
    │   └── mdbl.c
    ├── embeddedjs/
    │   ├── manifest.json
    │   └── main.js
    └── pkjs/
        └── index.js
```

## Implementation workflow

1. Capture requirements (visual style, complications, update cadence).
2. Create project scaffold with `scripts/create_project.py`.
3. Implement rendering in `src/embeddedjs/main.js` with `commodetto/Poco`.
4. Keep `src/c/mdbl.c` minimal: create window + `moddable_createMachine(NULL)`.
5. Keep `src/pkjs/index.js` wired to `@moddable/pebbleproxy`.
6. Run `npm install` if `@moddable/pebbleproxy` is required.
7. Build via `pebble build` and verify `.pbw` output.
8. Install/test via emulator and capture screenshots.

## package.json requirements

Use these key fields:

- `"projectType": "moddable"`
- `"enableMultiJS": true`
- `"targetPlatforms": ["emery", "gabbro"]`
- `"watchapp.watchface": true`
- `"dependencies": {"@moddable/pebbleproxy": "^0.1.8"}`

## Rendering guidelines for Alloy

- Draw using `Poco` in `draw(event)`.
- Subscribe with `watch.addEventListener("minutechange", draw)` for low-power watchfaces.
- Use `secondchange` only when animation is required.
- Avoid dynamic allocation in tight draw loops.
- Prefer simple integer math and lookup tables for animation.

## Templates and references

Use these files directly:

- `templates/alloy-watchface.js`
- `templates/alloy-manifest.json`
- `templates/alloy-mdbl.c`
- `templates/alloy-pkjs-index.js`
- `templates/package.json.template`
- `templates/wscript.template`

Reference downloads from the v127 announcement links:

- `reference/downloads/alloy-watchface-tutorial/`
- `reference/downloads/pebble-examples/`

## Validation checklist

- `package.json` uses `projectType: moddable`.
- Targets are exactly `emery` and `gabbro`.
- `src/embeddedjs/main.js` exists and renders time/date.
- `src/pkjs/index.js` exists and uses `@moddable/pebbleproxy`.
- Build produces a `.pbw` file.
