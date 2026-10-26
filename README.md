# dada · 搭搭

An open hardware application platform, starting with one ESP32-C3 touch board.
Apps consume capability contracts; board bindings own pins, buses and drivers.

中文：[README.zh-CN.md](README.zh-CN.md).

## Current implementation

- Jinyichen ESP32-C3 + 2.4-inch ST7789 display + CST836U touch; ESP-IDF **v5.4.2**.
- Display/input contracts, reusable drivers and manifest-generated board configuration.
- Two selectable native C apps: `touch-counter` (default) and `bringup` (colour bars/touch logs).
- Experimental single-app lifecycle with injected services; host tests and local delivery CLI.
- Real hardware colour bars and touch events confirmed. Counter interaction and cold-start
  results are tracked separately in [hardware records](tests/hardware/records/).

There is no general runtime, app IR compiler, automatic hardware discovery, package registry,
AI connector or second supported board yet. Native C validates the contracts first;
[ADR 0005](docs/adr/0005-app-ir.md) remains a proposed declarative application direction.

## Quick start

Activate an installed ESP-IDF v5.4.2 environment first, then from the repository root:

```sh
python tools/dada.py check
python tools/dada.py build
python tools/dada.py ports
python tools/dada.py flash --port YOUR_PORT
python tools/dada.py monitor --port YOUR_PORT
# Hardware diagnostic app, kept in a separate build directory:
python tools/dada.py build --app bringup
```

Flashing replaces the existing firmware. Keep its backup if you need to restore it.
Detailed setup: [development guide](docs/development.md).
Counter behaviour: [app guide](apps/touch-counter/README.md).

## Architecture and next steps

```text
apps/             hardware-independent application behaviour and private UI
contracts/        display/input capabilities and experimental app lifecycle
firmware/         board selection, dependency injection and scheduling
hardware/         board manifests and bindings (pins, peripherals, init settings)
components/       reusable display and touch drivers
runtime/          reserved for shared services proven necessary by multiple apps
connectors/       reserved for optional local/cloud service adapters
tools/            local selection checks, manifest validation, generation and IDF delivery
```

The application calls injected contracts; board bindings supply driver implementations.
Host tools prepare and deliver firmware; they are not device runtime services.

[Incremental roadmap](docs/architecture/incremental-roadmap.md):
**counter → offline pomodoro → second board → external hardware contribution → app ecosystem**.
Each stage has an acceptance gate. Two resolutions in host tests do not establish portability
across physical boards. No general framework or registry before the second-board gate.

## Tests and collaboration

```sh
cmake -S tests/host -B /tmp/dada-host
cmake --build /tmp/dada-host
ctest --test-dir /tmp/dada-host --output-on-failure
python3 -m unittest discover -s tests/host -p 'test_*.py'
```

See [CONTRIBUTING.md](CONTRIBUTING.md), [ADRs](docs/adr/README.md) and
[host tools](tools/README.md). CI builds both apps and runs host/manifest checks;
physical display and touch acceptance still needs a hardware record.
Vendor sample packages without a suitable license must not be uploaded or copied into apps.
`dada verify` is planned, not implemented.

Code and commits use English; documentation uses paired English/Chinese pages where available.
License selection remains open: [ADR 0004](docs/adr/0004-license.md).
