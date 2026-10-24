# 固件集成入口

从仓库根目录执行 `idf.py -C firmware build`。`main/main.c` 只验证启动与日志，不代表应用层或平台 API 的设计。

默认串口控制台沿用 ESP-IDF 的 UART 默认设置。若板卡使用原生 USB Serial/JTAG，先确认原理图，再通过 `idf.py -C firmware menuconfig` 设置 Console output channel；不要猜测 USB 与串口接线。

`sdkconfig` 是本机生成配置，不提交；共同配置应经过审查写回 `sdkconfig.defaults`。当前采用 2MB 单应用分区作为最小启动基线，不代表板载 Flash 已确认为 2MB。不启用 OTA、Wi-Fi、显示或触摸。

将来应用组件放在 `apps/`，平台组件放在 `runtime/`，板级组件放在 `hardware/`；在此处通过 `EXTRA_COMPONENT_DIRS` 显式接入。只添加已经实现的组件，避免扫描整个仓库。
