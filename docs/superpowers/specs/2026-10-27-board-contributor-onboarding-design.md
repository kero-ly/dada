# Board contributor onboarding / 新板贡献者接入

## Intent

Help an ESP32-C3 board contributor build a diagnostic image before hardware verification,
without allowing an unverified manifest to look like a supported app/board match.

## Approaches

1. Rely on prose and ask contributors to edit CMake manually. Small change, but easy to bypass
   checks and difficult to reproduce. Rejected.
2. Add an explicit `--allow-unverified` path through CLI and CMake, keep normal selection
   strict, then document the board bring-up workflow. Chosen because it uses the existing
   manifest and delivery path while preserving an obvious verification boundary.
3. Build a hosted one-click flasher or arbitrary board auto-detection. Deferred: neither can
   safely choose drivers, pin mappings, reset procedure, or firmware for unknown hardware.

## Scope and constraints

- Missing `verified_capabilities` is treated as an empty list; it never implies verification.
- Normal build/flash matching requires every app capability to be declared and verified.
- Display/touch are declared by hardware sections; non-structural services such as time/storage
  are declared in `declared_capabilities`.
- Explicit `--allow-unverified` may build/flash only when required capabilities are declared;
  it never certifies or adds capabilities.
- A contributor guide explains package files, evidence, tests, flashing, and support claims.
- No source is copied from unlicensed vendor archives. ESP-IDF remains required for firmware.
- Current integration supports ESP32-C3 only; this change does not add other SoC targets.

## Acceptance

- Host tests prove strict matching, declared-but-unverified bring-up, and rejection of
  undeclared hardware requirements.
- CMake passes the same explicit opt-in used by the local CLI; default remains strict.
- A contributor can follow bilingual instructions from manifest through diagnostic image
  and measured capability records.
