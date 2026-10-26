# Counter Platform Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** ship a selectable counter and bring-up app using hardware-independent contracts.
**Architecture:** static firmware composition injects display/input into one native C app.
The host tool delegates to ESP-IDF with a separate build directory per app/board.
**Tech Stack:** C11, Python standard library, ESP-IDF v5.4.2, CMake, GitHub Actions.
**Spec:** `docs/superpowers/specs/2026-10-26-counter-platform-design.md`

## Global Constraints

- ESP-IDF v5.4.2; no third-party libraries or vendor source uploads.
- Hardware-independent C11 apps; small stripe buffer; count saturates at 9999.
- No general runtime registry, app IR, dynamic loading, or second-board support claim.
- App API experimental; hardware visual success requires user confirmation.

## Review Focus

- Sustained/dragged touch must not create repeated clicks or unintended resets.
- Invalid input and off-panel coordinates must not change state.
- Display failure must reach the firmware entry point and release resources.
- Small/different displays must receive only valid rectangle coordinates.
- Build selection must reject unknown apps and incompatible boards before flashing.

### Task 1: Preserve hardware baseline

Files: existing board.c, display_st7789.c, hardware debug record.
- [x] Check diff and hardware evidence; commit only the three tracked hardware-fix files.
  Expected: mode 3, inversion true, RGB565 little endian, no vendor files staged.

### Task 2: App contract and counter

Files: `contracts/app.h`, `apps/touch-counter/{counter.h,counter.c,render.c,CMakeLists.txt}`,
`tests/host/test_counter.c`, `tests/host/CMakeLists.txt`.
Interface: `dada_app_start(const dada_app_services_t *)`, `dada_app_step(void)`, `dada_app_stop(void)`;
counter exposes `counter_count(void)` to verify business state without hardware.
- [x] Write host C tests for click/release, hold/drag, reset, invalid samples and bounds,
  saturation, missing capabilities, two resolutions, start/flush failure, retry after failure.
- [x] Run host build; expect RED because app API/implementation is missing.
- [x] Implement contract, private strip renderer and counter, run CTest; expect GREEN.

### Task 3: Static app selection and delivery

Files: `apps/bringup/*`, `firmware/{CMakeLists.txt,main/*}`, `tools/dada.py`,
`tests/host/test_tool.py`, `.github/workflows/{firmware.yml,host.yml}`.
- [x] Test CLI command construction, explicit serial selection, independent build paths,
  invalid IDs and capability mismatch; observe RED before implementation.
- [x] Move bring-up into its own app, keep main as board/lifecycle/logging glue.
- [x] Add selection/capability checks and CLI; run host CTest + Python unittest.
- [x] Build touch-counter and bringup with ESP-IDF; unknown app must fail configure.
  Expected: both real apps compile, invalid selection fails clearly.

### Task 4: Team handoff and hardware validation

Files: README pair, firmware docs, architecture roadmap, counter README pair, hardware report.
- [x] Document actual status, layer responsibilities, commands and next acceptance gates.
- [x] Flash touch-counter and inspect startup logs; request user count/reset acceptance.
- [ ] Receive physical counter interaction and cold-power-on acceptance (pending user observation).
- [x] Preserve pending cold-boot/physical-coordinate checks explicitly.
- [x] Run full host suite, both firmware builds, manifest checks and diff check.
- [x] Review final change and commit project files only; do not push without a new request.

## Execution ledger

User selected the proposed minimal counter path and requested incremental architecture.
Continue implementation inline; no extra approach selection is needed.


### Evidence and decisions (2026-10-26)

- Hardware baseline committed as `3a73a8f`; vendor archive/tree left untracked.
- Native C/CLI tests each observed RED before implementation, then passed.
- Independent review found wide-display proportions; added landscape regression, observed
  failing startup, clamped digit width by available height, then CTest passed.
- Both applications built with IDF v5.4.2 through isolated CLI directories. Counter flashed
  with hash verification; startup log reports `touch-counter started (240x320)`.
- Four Python tests and both board manifests passed; generated headers; invalid app rejected
  by real IDF configure. `git diff --check` passed.
- Optional sanitizers inconclusive on this host: test timed out; a print-only instrumented
  executable also stalls before main in ASan shadow-memory initialization. No sanitizer pass claimed.
- Counter physical interaction, orientation and cold-start remain pending; see hardware record.
- Added incremental roadmap with acceptance gates: counter, offline pomodoro/shared needs,
  second-board portability, external hardware contributor, external application ecosystem.
- `firmware/sdkconfig` remains shared for current C3 target, despite separate build outputs.
  Complete cross-chip configuration and capability/resource matching deliberately deferred.

- Implementation saved as `8354e1d` on `codex/c3-counter-platform`; documentation saved separately.
