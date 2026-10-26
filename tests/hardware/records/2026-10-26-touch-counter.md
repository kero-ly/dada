# C3 touch counter delivery — 2026-10-26

## Scope / 验证边界

Board: `jinyichen-esp32-c3-lcd-2.4`, ESP32-C3 rev0.4, measured 4MB flash.
SDK: ESP-IDF v5.4.2; local SDK reports `-dirty` due to incomplete nested test submodule setup.
No SDK display/touch driver modifications were used.

Hardware fixes were preserved in commit `3a73a8f`: SPI mode 3, inversion enabled,
RGB565 little endian. Prior colour-bar and touch evidence is in
[display debug record](2026-10-26-idf-display-blocked.md).

Application/tool implementation commit: `8354e1d`.

## Delivery evidence

- Both `python tools/dada.py build --app touch-counter` and `--app bringup` completed.
- Per-selection build directories: `firmware/build/<board>--<app>/`.
- Counter image: **244064 bytes**, SHA256
  `498c09cbd58aacaddc693d62234bc86dd8c285e55398a6a30432e3cb239d3f99`.
- CLI flash to `/dev/cu.usbmodem1101` completed with esptool hash verification.
- Serial startup after USB reset:

```text
I (206) display_st7789: ST7789 240x320 ready
I (286) touch_cst836u: CST836U at 0x15, touches=0
I (1976) app_host: touch-counter started (240x320)
```

This confirms initialization and successful initial rendering calls, not physical screen
appearance or touch-to-widget correspondence. USB reset is not a cold power-on test.
Raw local command logs are under `/tmp/dada-cli-*-build.log`, `/tmp/dada-cli-counter-flash.log`
and `/tmp/dada-counter-startup.log`; these transient logs are not committed artifacts.

## Software checks

- Host CTest passed: click, hold, drag, reset, outside/invalid input, 9999 saturation,
  idempotent stop, error propagation and retry. Geometry: 240×320,160×160,800×160,4000×160.
- Python delivery tests: 4 passed, covering commands, explicit ports, isolated output paths,
  invalid identifiers, missing capabilities and display requirements.
- Both board manifests passed schema/semantic checks; configuration headers generated.
  The planned S3 manifest passing schema does not make it a supported board.
- Real IDF configure with `DADA_APP=missing` failed with `unknown app: missing`, as expected.
- Independent code review found a wide-display digit-proportion bug; regression reproduced
  and fixed by limiting digit width with available height. Host tests passed after the fix.
- Optional AddressSanitizer/UBSan run was inconclusive: timed out at 30 seconds. A minimal
  instrumented program that only prints also failed to reach its output on this host.
  A process sample located it in ASan shadow-memory initialization before main.
  This is not recorded as a sanitizer pass; normal host tests pass. No production SDK change
  was made to work around the host instrumentation issue.
- Remote GitHub Actions have not been run for this branch.

## Physical acceptance — pending user observation

- [ ] Screen shows `0000`, `TAP +`, `RESET` with correct orientation.
- [ ] Tap upper region increments; hold counts once; drag into reset does not reset.
- [ ] Release then tap RESET clears to zero; touch regions correspond to visible regions.
- [ ] Disconnect/reconnect power: app boots and shows zero.
- [ ] Another team member reproduces from a clean clone.

A request for user observation was sent after flashing; no response was available when
this record was written. Do not label the counter or cold-start checks as physically passed.
