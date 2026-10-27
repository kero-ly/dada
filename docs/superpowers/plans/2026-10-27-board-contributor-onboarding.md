# Board Contributor Onboarding Plan

> **For agentic workers:** use TDD and inline execution. Steps use checkbox (`- [ ]`) syntax.

**Goal:** let contributors build an explicitly unverified diagnostic image without confusing it with supported hardware.
**Architecture:** add an opt-in CLI/CMake flag and authoritative verified-capability checks; document a reproducible board contribution path.
**Tech Stack:** Python 3 standard library, CMake, ESP-IDF v5.4.2, Markdown.
**Spec:** `docs/superpowers/specs/2026-10-27-board-contributor-onboarding-design.md`

## Global Constraints

- Default build and flash require declared and verified app capabilities.
- Unverified mode still requires the relevant hardware sections and board binding.
- The change does not add new SoC targets or claim automatic board adaptation.

## Review Focus

- Explicit empty verification lists must not fall back to inferred support.
- CLI and CMake must use the same verification policy.
- Unverified mode must not bypass absent hardware declarations.
- Contribution steps must distinguish compiled, measured, and supported states.

### Task 1: Selection policy

Files: `tools/dada.py`, `tests/host/test_tool.py`, `firmware/CMakeLists.txt`.
- [x] Add tests proving strict rejection of empty/missing verified capabilities.
- [ ] Run the focused test and observe the expected failure.
- [x] Add `allow_unverified=False` selection and command options; keep missing declared hardware rejected.
- [x] Add `DADA_ALLOW_UNVERIFIED` CMake option and pass it to the configure-time selection check.
- [ ] Run all host tests and reject an intentionally invalid selection during CMake configure.

### Task 2: Contributor workflow

Files: `docs/hardware/board-contribution.md`, `.zh-CN.md`, `CONTRIBUTING.md`, root README pair, `tools/README.md`.
- [x] Document exact discovery, manifest, binding/driver, diagnostic build/flash, validation, and PR steps.
- [x] Show the explicit unverified command and explain its limits.
- [x] Add discoverable links and list all currently selectable apps accurately.
- [ ] Verify local links, CLI help, host tests, and diff whitespace.

## Execution ledger

- Ruling: add an opt-in for local diagnostic firmware rather than permit unverified normal matching — this preserves early hardware experimentation and honest support claims — if users misunderstand the flag, they could mistake a buildable target for a supported one; command and docs must repeat the boundary.
