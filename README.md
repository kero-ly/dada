# dada · 搭搭

**Connect applications and hardware. Focus on your product.**

dada aims to connect application developers with the hardware ecosystem.
Vendors contribute hardware descriptions and drivers; developers build hardware-independent
applications; dada handles matching, building, validation and delivery.

**English** · [简体中文](README.zh-CN.md)

[Development guide](docs/development.md) · [Architecture roadmap](docs/architecture/incremental-roadmap.md) · [Contributing](CONTRIBUTING.md)

> Stage: first-board validation on ESP32-C3. The diagrams below describe the platform vision; see Current progress for implemented capabilities.

## Where can I contribute?

**One integrated C3 board is supported today. Board + module + composition is the agreed direction; composition tooling is not implemented.**

| Goal | Your focus | Start here |
| --- | --- | --- |
| Build an app | Business logic in `apps/`, using contracts without GPIO or board-specific branches | [Developer boundaries](docs/architecture/developer-boundaries.md) |
| Adapt a board | Hardware facts, binding and physical evidence in `hardware/<vendor>/<board>/`; vendor documentation is still needed | [Board guide](docs/hardware/board-contribution.md) |
| Add a peripheral | Real module/interface research and a working adaptation before extracting reusable modules and wiring compositions | [Module/composition direction](docs/architecture/developer-boundaries.md#agreed-direction-board--module--composition) |
| Improve tools, tests or docs | Make builds, flashing and contributions easier to reproduce | [Contributing](CONTRIBUTING.md) |

Boards own fixed devices, modules own reusable peripherals, compositions own connections, and apps declare capabilities.
App reuse depends on compatible capabilities, resources and evidence; one binary does not work on every C3 board.
Missing required capabilities reject a build. Optional-capability degradation and audio interfaces remain future work.

Keep the integrated board intact; do not create empty package scaffolding or a generic composition framework before real hardware validates it.
Read the [developer guide](docs/architecture/developer-boundaries.md) before contributing, and [AGENT.md](AGENT.md) for agent collaboration ([automatic entry point](AGENTS.md)).

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
- Display, input, monotonic time and storage contracts; NVS storage and manifest-generated board configuration.
- Four selectable native C apps: `touch-counter` (default), `bringup`, `focus-clock` and `snake`.
- Experimental single-app lifecycle, injected services, shared app canvas, host tests and local delivery CLI.
- Display, touch, counter, pomodoro and Snake behaviours have board-specific acceptance records;
  see [hardware records](tests/hardware/records/).

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
# First build on a new, still-unverified C3 board:
python tools/dada.py build --board YOUR_BOARD_ID --app bringup --allow-unverified
```

Flashing replaces the existing firmware. Keep its backup if you need to restore it.
Detailed setup: [development guide](docs/development.md).
Counter behaviour: [app guide](apps/touch-counter/README.md).
Adding an ESP32-C3 board: [board contribution guide](docs/hardware/board-contribution.md).

## From one board to a platform

**Counter loop → offline pomodoro → second board → external hardware contribution → app ecosystem**

Use real applications to identify shared services, a second board to validate source reuse,
and reproducible workflows to enable third-party contributions.
See the [acceptance gates](docs/architecture/incremental-roadmap.md) for each stage.

| Explore | Documentation |
| --- | --- |
| Current C3 architecture and delivery workflow | [Implementation diagrams](docs/architecture/current-implementation.md) |
| Setup, build, flash and team workflow | [Development guide](docs/development.md) |
| Add and verify an ESP32-C3 board | [Board contribution](docs/hardware/board-contribution.md) |
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
[host tools](tools/README.md). CI builds all four apps and runs host/manifest checks.
Vendor sample packages without a suitable license must not be uploaded or copied into apps.
`dada verify` is planned, not implemented.

Code and commits use English; documentation uses paired English/Chinese pages where available.

## License

Unless otherwise noted, dada is licensed under the [Apache License 2.0](LICENSE).
Third-party components retain their own licenses. This license does not grant rights to dada trademarks.
See [ADR 0004](docs/adr/0004-license.md) for scope and contribution terms.
