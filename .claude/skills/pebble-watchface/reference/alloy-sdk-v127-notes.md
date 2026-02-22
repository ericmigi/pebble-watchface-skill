# Alloy SDK v127 migration notes

Source announcement:
- https://repebble.com/blog/cloudpebble-returns-plus-pure-javascript-and-round-2-sdk

Key decisions applied in this skill:

1. Use Alloy/Moddable projects (`pebble.projectType = "moddable"`).
2. Keep watchface logic in `src/embeddedjs/main.js`.
3. Use Moddable proxy bridge in `src/pkjs/index.js` with `@moddable/pebbleproxy`.
4. Restrict target platforms to `emery` and `gabbro`.
5. Use SDK tooling from the 4.9.127 release train.

Downloaded upstream references:
- `reference/downloads/alloy-watchface-tutorial/`
- `reference/downloads/pebble-examples/`

Related links:
- https://developer.repebble.com/guides/alloy/
- https://developer.repebble.com/tutorials/alloy-watchface-tutorial/part1/
- https://github.com/coredevices/alloy-watchface-tutorial
- https://github.com/Moddable-OpenSource/pebble-examples
