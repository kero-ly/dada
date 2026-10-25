# dada · 搭搭

**An open platform that connects applications to hardware — write once, compose freely.**

Apps talk to boards through a small set of **capability contracts**, so the same app can run on
different screens, MCUs and accessories without rewriting the app.

> 中文说明见 [README.zh-CN.md](README.zh-CN.md)。

---

## Status (be honest about it)

This repository is at the **bootstrap** stage. What exists today:

- ✅ ESP-IDF project skeleton (`firmware/`) that boots and prints one log line.
- ✅ Directory structure, board manifest draft, CI build workflow, collaboration templates.
- ✅ Architecture decisions recorded as ADRs ([`docs/adr/`](docs/adr/README.md)).
- ✅ Display (ST7789) and touch (CST836U) drivers behind the capability contracts, plus a
  board binding that reads the generated manifest header.
- ✅ First board **bring-up passes on ESP-IDF v5.4.2** — colour bars render and touch answers
  on real hardware, with no pin number anywhere in the app code. See
  [`tests/hardware/records/`](tests/hardware/records/).

What does **not** exist yet:

- ❌ No runtime (capability registry / lifecycle), no app IR compiler, no focus-clock app.
- ❌ No second board, so the abstraction is still unproven.
- ❌ No license file yet (see [ADR 0004](docs/adr/0004-license.md)).

The first board is a Jinyichen ESP32-C3 board with a 2.4" 240×320 touch panel.
The first target app is an offline pomodoro clock.

## Quick start

```sh
# requires ESP-IDF v5.4.2 (baseline; enforced by firmware/CMakeLists.txt)
idf.py -C firmware set-target esp32c3
idf.py -C firmware build
```

Building does not require the board. Flashing does — see
[`tests/hardware/bringup.md`](tests/hardware/bringup.md).

## Repository layout

```text
apps/         applications (business logic only; no GPIOs, no board names)
runtime/      platform services and capability registry          (not implemented)
components/   reusable drivers implementing the contracts        (not implemented)
contracts/    capability contracts + JSON schemas                (draft)
hardware/     per-board manifests: pins, init data, provenance
connectors/   optional AI / external service adapters            (not implemented)
firmware/     ESP-IDF integration project
tools/        host-side tooling (manifest validation, codegen, verify)
tests/        hardware bring-up steps and verification records
docs/         ADRs, glossary, development notes
```

Dependency direction: **app → contracts → components / board binding → ESP-IDF → hardware.**

## Design principles

1. **Contracts are sink-level.** A display contract moves pixels and controls a backlight; it
   does *not* offer drawing primitives. UI frameworks live above the contract and stay
   replaceable. → [ADR 0001](docs/adr/0001-layering.md)
2. **The board manifest is the single source of truth**, validated in CI and compiled into a C
   header at build time. Unknown values stay `null`; guessing is forbidden.
   → [ADR 0002](docs/adr/0002-board-manifest.md)
3. **A board is "supported" only if a machine-readable verification report exists.**
   → [ADR 0003](docs/adr/0003-verification.md)
4. **Apps are declarative.** YAML is the IR; a block editor or a visual designer can target the
   same IR later. → [ADR 0005](docs/adr/0005-app-ir.md)

## Roadmap

| Stage | Goal | Exit criterion |
| --- | --- | --- |
| P0 | one board + one app, end to end | a stranger sees a working pomodoro within 10 minutes |
| P1 | second board (ESP32-S3 + separate panel + separate touch) | the same app runs unmodified |
| P2 | third-party boards | an outside contributor submits a board + verify report |
| P3 | app ecosystem | the first declarative app written by someone else is merged |

**Hard rule:** no general-purpose runtime, no package registry and no "supports N boards" claim
before P1 is done. An abstraction designed against a single board is usually wrong.

## Language policy

English is the primary language for code, commit messages and issue templates.
Documentation is bilingual: `X.md` (English) and `X.zh-CN.md` (Chinese). ADRs carry an English
summary at the top and Chinese body.

Machine translation is not used for published docs — better to have three good pages than thirty
bad ones.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Two rules worth repeating here:

- **Never copy code from a board vendor's package.** Seller sources usually ship without a
  license; re-implement against the datasheet instead.
- **New boards must include a verification report** produced by `dada verify`.

## License

Not selected yet — see [ADR 0004](docs/adr/0004-license.md) for the recommendation (Apache-2.0).
Until a license file exists, this repository is "all rights reserved" and cannot be legally
reused.
