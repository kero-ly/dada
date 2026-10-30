# Applications

Seven native C apps. All of them build only on the capability contracts, so
none of them contains a pin number, a board name or a driver call.

The fastest way in is to pick the app closest to what you want to build.

| App | Requires | What it is | Read it if… |
| --- | --- | --- | --- |
| `touch-counter` | display, input | Smallest input → output loop: tap to count, bottom quarter resets | you just want to prove the board works |
| `bringup` | display, input, storage | Hardware diagnostic: numbered grid, touch marker, storage self-check | something on the panel behaves oddly |
| `focus-clock` | display, input, **time** | 25-minute pomodoro with start/pause/reset | you want to see the time contract used |
| `snake` | display, input, time, **storage** | Snake with swipe steering and a persisted best score | you want a complete small game |
| `paint` | display, input, **storage** | 48×64 pixel canvas, 15 colours, bitmap packed to NVS | you want to see blob storage |
| `whale` | display, **storage** | 160×120 portrait, rendered in stripes and rotated onto the panel | you want non-interactive rendering |
| `flappy` | display, input, time, storage | Complete game ported from the author's Arduino build | you want to see a project migrated onto the contracts |

## What they have in common

- **Only `contracts/`.** No GPIO numbers, no board detection, no driver init.
- **Declared needs.** Each `app.json` states the capabilities it requires, and
  `tools/dada.py check` fails the build when the selected board cannot provide
  them — before anything is flashed.
- **Rules separate from pixels.** `counter.c`, `clock.c`, `snake.c`, `paint.c`
  and `flappy.c` hold the logic with no UI, so they run in the host tests.
- **Degrade, do not crash.** Storage is optional for most apps; without it the
  best score simply is not kept.

## Adding one

1. `apps/<id>/app.json` — id, `format: native-c-v1`, required capabilities, and
   any minimum display size.
2. Implement `dada_app_start/step/stop` from `contracts/app.h`.
3. Add a `CMakeLists.txt` that requires `dada_contracts` (plus `app_support` if
   you want the canvas helper).
4. Register the app in `firmware/CMakeLists.txt` (`DADA_APP` strings),
   `firmware/platformio.ini` (one env) and `.github/workflows/firmware.yml`.
5. Put the logic in its own `.c` so it can be tested in `tests/host/`.

Keep the app free of board knowledge; if it needs something the contracts do not
offer, that is a contract discussion, not a `#include <driver/...>`.
