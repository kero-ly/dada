# Add an ESP32-C3 board / 接入一块 ESP32-C3 开发板

This guide is for a contributor who has a different C3 board and wants to run the diagnostic
app before claiming support. The current firmware integration targets **ESP32-C3 only** and
requires an implemented board binding; adding `board.json` alone does not enable a board.

中文：[board-contribution.zh-CN.md](board-contribution.zh-CN.md).

## 1. Identify the exact hardware

Record vendor, product name, PCB revision, SoC and flash size. Identify the USB interface and
how it enters download mode. For each display/touch/input device, collect controller, bus,
resolution, orientation, GPIOs and reset/backlight polarity. Prefer schematics and datasheets;
link every fact to its source. Unknown or batch-dependent values stay `null` until measured.

Do not upload seller source packages without an explicit compatible license. Read interface
and initialization facts, then implement against the public datasheet and dada driver contracts.

## 2. Add the board package

Copy the structure of the closest existing board under `hardware/<vendor>/<board>/`, then
replace every board-specific value. The manifest id must equal `<vendor>-<board>` and the
board directory must contain `board.json` plus `board/CMakeLists.txt` and a board binding.

Start with `status: "purchased"` (or `planned` before acquisition), empty
`verified_capabilities`, and list non-structural services such as `time` or `storage` in
`declared_capabilities` when the board binding intends to provide them. Display and touch are
declared by their hardware sections. Add provenance for every known hardware field. Use
`tools/validate_manifests.py` and preserve `null` for facts you have not established.

The binding maps the generated manifest header to actual component configuration. Reuse an
existing driver only when its controller and bus match. A new display/touch controller needs
a licensed driver component and host tests where practical. Do not put GPIOs in an app.

## 3. Build the diagnostic firmware

Install and activate ESP-IDF v5.4.2, then use the explicit board id and USB port:

```sh
python tools/dada.py check --board <vendor>-<board> --app bringup --allow-unverified
python tools/dada.py build --board <vendor>-<board> --app bringup --allow-unverified
python tools/dada.py ports
python tools/dada.py flash --board <vendor>-<board> --app bringup \
  --allow-unverified --port YOUR_PORT
python tools/dada.py monitor --board <vendor>-<board> --app bringup \
  --allow-unverified --port YOUR_PORT
```

`--allow-unverified` is only an explicit local bring-up gate: requirements must still be
declared by the board manifest (`display`/`touch` sections or `declared_capabilities`) and a
board binding must exist. It does not add capabilities,
mark the board supported, or skip compilation. Flashing replaces the device firmware.

The current `bringup` app needs display and touch input. Its 3×3 grid lets you compare each
visible cell with the reported touch row/column and coordinates. If only one of those
capabilities exists, create or adapt a diagnostic app whose requirements match the hardware;
do not weaken the app manifest just to make the build pass.

## 4. Verify and claim support

Run the diagnostic app on the physical board. Record the exact PCB revision, build command,
flash/interface procedure, startup logs and results for every capability. Check screen
orientation, colours, full-area bounds, touch mapping and cold power-on. For storage, verify
values survive power removal. Distinguish `vendor-doc`, `schematic`, `measured` and
`community-verified` evidence as defined in ADR 0002.

Only after a capability passes physical checks should it appear in `verified_capabilities`.
Remove `--allow-unverified` and run the normal selection/build command for each app you claim
compatible. A successful compile is not a physical verification report. No universal binary,
automatic pin discovery or arbitrary module composition is implied.

## 5. Run checks and open a pull request

```sh
python tools/validate_manifests.py
python tools/gen_board_config.py hardware/<vendor>/<board>/board.json \
  -o /tmp/dada-board-config.h
cmake -S tests/host -B /tmp/dada-host
cmake --build /tmp/dada-host
ctest --test-dir /tmp/dada-host --output-on-failure
python3 -m unittest discover -s tests/host -p 'test_*.py'
```

Attach a concise hardware record under `tests/hardware/records/`, including passed and
untested capabilities. Link source documentation and list licensed driver dependencies.
Use the **New board proposal** issue template if you need help identifying a board or driver.
The current integration hard-codes ESP32-C3 and has no generic SoC selection; other ESP32
families and Linux boards require a separate platform change before they can build here.
