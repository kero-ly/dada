# Applications

Seven native C apps. All of them build only on the capability contracts, so
none of them contains a pin number, a board name or a driver call.

The fastest way in is to pick the app closest to what you want to build.

| App | Requires | What it is | You should see | Read it if… |
| --- | --- | --- | --- | --- |
| `touch-counter` | display, input | Smallest input → output loop: tap to count, bottom quarter resets | `0000` and `TAP +`, a brighter strip along the bottom | you just want to prove the board works |
| `bringup` | display, input, storage | Hardware diagnostic | A 3×3 numbered grid; taps leave a red square; the log prints a boot counter | something on the panel behaves oddly |
| `focus-clock` | display, input, **time** | 25-minute pomodoro | `25:00`, a progress bar, a state word (`TAP`/`RUN`/`PAUSE`/`DONE`), `RESET` strip | you want to see the time contract used |
| `snake` | display, input, time, **storage** | Snake with swipe steering | A 12×14 board, green snake, red food, score in the top bar | you want a complete small game |
| `paint` | display, input, **storage** | 48×64 pixel canvas, 15 colours | An empty white canvas and a colour button in the bottom-right corner | you want to see blob storage |
| `whale` | display, **storage** | 160×120 portrait | The portrait filling the panel; hold the board sideways | you want non-interactive rendering |
| `flappy` | display, input, time, storage | Complete game ported onto the contracts | Sky, green pipes, ground, a yellow bird, score at the top | you want to see a project migrated |

## Suggested reading order

Easiest first; each step adds exactly one new idea:

1. **`touch-counter`** — the whole lifecycle in 53 lines: start, poll, step, stop.
2. **`focus-clock`** — adds the `time` contract, and splits rules (`clock.c`) from pixels (`render.c`).
3. **`snake`** — adds `storage`, a turn queue, and a second host test file.
4. **`paint`** — adds packed blobs, a popup palette, and size-checked loading.
5. **`whale`** — no input at all: rendering, rotation, and stripe flushing.
6. **`flappy`** — the largest one, and a worked example of porting existing code.

`bringup` is a tool rather than a demo: reach for it when the hardware misbehaves, not to learn the pattern.

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
