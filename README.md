# dada · 搭搭

**Connect applications and hardware. Focus on your product.**

dada aims to connect application developers with the hardware ecosystem.
Vendors contribute hardware descriptions and drivers; developers build hardware-independent
applications; dada handles matching, building, validation and delivery.

**English** · [简体中文](README.zh-CN.md)

[Development guide](docs/development.md) · [Architecture roadmap](docs/architecture/incremental-roadmap.md) · [Contributing](CONTRIBUTING.md)

> Stage: first-board validation on ESP32-C3. The diagrams below describe the platform vision; see Current progress for implemented capabilities.

## Platform vision: reusable apps, composable hardware

![dada ecosystem vision: application developers and hardware vendors contribute packages; the platform matches, builds, validates and delivers compatible products, with optional AI and local services.](docs/assets/architecture/platform-ecosystem-vision.png)

Application developers describe behaviour and required capabilities. Hardware vendors provide
chip, board and peripheral descriptions, drivers and verification evidence. Product builders
and users select applications and compatible hardware combinations through the platform.
Apps may connect to different AI providers, use local services or run entirely offline.
Local model inference depends on device resources.

## Global architecture: device platform and delivery platform

![dada target architecture: applications, capability contracts, shared runtime services, hardware adaptation and composable devices, alongside package matching, build, validation and delivery tools.](docs/assets/architecture/platform-target-architecture.png)

- **Applications:** business logic, interaction and capability requirements, with reusable sources.
- **Device platform:** unified contracts, shared runtime services, chip/board adaptation and drivers.
- **Physical hardware:** supported and verified combinations of chips, boards and peripherals.
- **Host and delivery platform:** app/hardware packages, capability and resource matching,
  build orchestration, verification records, release and updates.

Application reuse depends on compatible capabilities, sufficient resources and physical
verification; firmware is built for its target hardware. Runtime services are selected as needed,
and host tools can run locally. ESP32 is the starting point; other MCUs and Linux boards
will be evaluated incrementally. Apps may use cloud AI, local services or run entirely offline.

## Current progress

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

## From one board to a platform

**Counter loop → offline pomodoro → second board → external hardware contribution → app ecosystem**

Use real applications to identify shared services, a second board to validate source reuse,
and reproducible workflows to enable third-party contributions.
See the [acceptance gates](docs/architecture/incremental-roadmap.md) for each stage.

| Explore | Documentation |
| --- | --- |
| Current C3 architecture and delivery workflow | [Implementation diagrams](docs/architecture/current-implementation.md) |
| Setup, build, flash and team workflow | [Development guide](docs/development.md) |
| Capability contracts, board manifests and app formats | [Architecture decisions](docs/adr/README.md) |
| CLI and manifest validation | [Host tools](tools/README.md) |
| Physical evidence and pending acceptance | [Hardware records](tests/hardware/records/) |

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

## License

The project intends to be open source; license selection remains open: [ADR 0004](docs/adr/0004-license.md).
