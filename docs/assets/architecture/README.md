# Architecture illustrations / 架构图说明

These bilingual PNGs are generated with the OpenLux `gpt-image-2` image service for
this repository's English and Chinese READMEs. The images are 1536×1024 pixels, generated
on 2026-10-26, and visually reviewed against the platform vision or current implementation, as labelled.

README 中两张愿景图展示目标生态和平台架构，另外两张 C3 图放在当前实现说明中。详细架构及未来验收门槛以
[增量路线](../../architecture/incremental-roadmap.zh-CN.md)和源码为准。

| Image | Meaning / 含义 |
| --- | --- |
| `platform-ecosystem-vision.png` | Target ecosystem: vendors, application developers, platform, product builders and optional services. / 平台生态愿景，非已交付能力清单。 |
| `platform-target-architecture.png` | Target device layers and host delivery platform; compatible hardware composition and optional connectors. / 全局目标架构。 |
| `platform-layers.png` | Applications → capability contracts → board bindings/drivers → hardware. Firmware host owns initialization, injection and scheduling. / 三层结构及固件入口职责。 |
| `delivery-workflow.png` | Local declaration checks → IDF build → explicit-port flash → device initialization and application execution → manual acceptance. / 电脑端交付、设备端运行与实机验收。 |

## Reading the diagrams / 阅读边界

The two vision diagrams are intentionally broader than current implementation. Their catalogs,
matching, shared runtime, verification and release/update blocks are target responsibilities,
not proof of existing systems. Runtime services may use hardware capabilities themselves;
the layered drawing groups responsibilities rather than prescribing a single call chain.
Local inference depends on device resources; offline applications need no AI provider.
The optional-services panel lists choices rather than imposing a dependency on every app.
Offline applications are an execution mode, not an external connector.

The notes below apply to the two current C3 implementation diagrams.


- Arrows in the layer diagram represent conceptual calls, initialization or grouping;
  they are not a circuit schematic or a C header dependency graph.
- ESP-IDF is an external SDK used by the drivers, not source contained in `components/`.
- The physical hardware row groups components of the board; the connecting lines do not
  specify wiring. The display and touch use their respective board-defined buses.
- Shared services, second-board support and AI connectors are explicitly planned.
- The delivery tool performs basic checks of declared capabilities and display size;
  it does not automatically certify a physical device.
- Device startup injects display/input into one statically linked application.
  Firmware initializes both board and application before scheduling the app.
- Manual acceptance includes screen appearance, touch correspondence and cold boot.
  The diagram does not claim those pending counter checks have passed.

图为概念架构，不是电路接线图。ESP-IDF 是驱动使用的外部 SDK；底层硬件连线仅表示分组关系。
规划能力不代表已实现，交付流程也不代表待验收项目已经通过。图中的固件入口将驱动提供的
显示与输入实现注入应用，随后调度运行。

## Maintaining the assets / 维护

Keep both README image paths and textual descriptions in sync. When regenerating,
use OpenLux, preserve the three-layer distinction, label planned capabilities, and
check all text and arrow directions before replacing the PNGs. Never include API
credentials or private provider configuration in this directory.
