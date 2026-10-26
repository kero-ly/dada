# Incremental platform architecture

Validate one delivery path first: app requirements → board selection → capability check →
build → flash → physical acceptance. A second physical board running unchanged app sources
is the first meaningful portability gate.

中文详解与架构图：[incremental-roadmap.zh-CN.md](incremental-roadmap.zh-CN.md).

## Responsibilities

| Layer | Current implementation | Later, when validated |
| --- | --- | --- |
| Developer applications | `apps/touch-counter`, `apps/bringup`, `apps/focus-clock`; native C and contracts only | External apps |
| Device platform | Display/input/time contracts, experimental lifecycle, firmware composition root, board bindings and reusable drivers | Shared event/UI services extracted from actual applications |
| Physical hardware | One C3 board, ST7789 panel, CST836U pointer input | A second documented ESP32 board with different display configuration |
| Host platform tools | Local manifests, basic compatibility checks, generated configuration, delegated IDF build/flash | Versioned board packages, resource constraints and reproducible verification |

Host tools prepare and deliver firmware; they do not execute the application on the device.
`runtime/` is reserved; the current scheduler/lifecycle lives in `firmware/main`.
A hardware description alone cannot support an unknown chip without drivers and a binding.

## Status (2026-10-26)

| Gate | State | Evidence |
| --- | --- | --- |
| 1 Hardware baseline | done | colour bands, orientation and touch coordinates accepted on device; see `tests/hardware/records/` |
| 2 Counter loop | done | count/reset accepted on device; clean-clone build reproduced by CI across three apps |
| 3 Offline pomodoro | first version accepted | `apps/focus-clock` runs with an injected `time` contract; one full session accepted end-to-end with a 2-minute build; the real 25-minute length has not been waited out |
| 4 Second physical board | not started | needs a board decision, not a roadmap label |
| 5 External hardware contribution | not started | templates and compatibility rules still missing |
| 6 Application ecosystem | not started | SDK and compatibility matrix come first |

## Acceptance gates

1. **Hardware baseline:** retain colour-bar/touch diagnostic app. Correct RGB bands and touch
   response are confirmed; power-on and coordinate correspondence require separate checks.
2. **Counter loop (current):** count, press-edge semantics, reset and power-cycle behaviour;
   another developer reproduces build/flash from a clean clone.
3. **Offline pomodoro:** inject a clock contract, test start/pause/reset and timing boundaries.
   Extract only services genuinely shared with the counter. Keep both apps selectable.
4. **Second physical board:** choose from actual resource needs and documentation quality.
   Both app sources remain unchanged. Memory, dimensions and touch differences stay below
   the contracts. Only then assess a general framework.
5. **External hardware contribution:** templates, schemas, compatibility/resource rules and
   traceable acceptance reports allow someone outside the team to submit a board package.
6. **Application ecosystem:** an external author independently delivers an app. Stabilize the
   SDK and compatibility matrix before IR, package distribution and update infrastructure.
   AI/local services are optional connectors, driven by an actual application.

Do not buy an S3 just to satisfy a roadmap label. Select the second board after requirements
and documentation are clear. Host tests at two resolutions are not cross-board validation.

## Current limits

The counter needs RGB565, pointer input and at least 160×160 pixels. The renderer is app-private.
The lifecycle is experimental and supports one statically linked app. Basic `app.json` checks
are not a complete matching engine. No dynamic installation or multi-app execution is promised.
Build output is isolated per board/app; `firmware/sdkconfig` is still shared local configuration
for this C3 project. Cross-chip configuration isolation belongs to the second-board milestone.

Next: accept the flashed counter, capture orientation/touch and cold-start evidence, ask another
team member to reproduce it, then implement the offline pomodoro before buying the next board.
