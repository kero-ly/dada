# 固件集成入口

`main/main.c` 是组合入口：初始化板卡 → 注入显示/输入服务 → 启动所选应用 →
每 20ms 调用一次 step；应用返回错误后 stop 并释放板级资源。
业务和界面放在 `apps/`，引脚和屏幕参数放在板级绑定与驱动。

推荐在仓库根目录使用 `python tools/dada.py build --app touch-counter`。
需要直接调用 IDF 时：

```sh
idf.py -C firmware -DDADA_APP=touch-counter build
idf.py -C firmware -DDADA_APP=touch-counter -p YOUR_PORT flash
```

直接调用的默认构建目录为 `firmware/build/`；CLI 为每个板卡/应用使用独立子目录。
可选应用为 `touch-counter`（默认）和 `bringup`，默认板为 `jinyichen-esp32-c3-lcd-2.4`。
CMake 配置时检查应用需求，未知或不匹配的选择直接失败。

工具链固定 ESP-IDF v5.4.2；当前 C3 配置为已测得的 4MB Flash，单应用分区。
控制台使用原生 USB Serial/JTAG，显示与触摸已接入；Wi-Fi 和 OTA 尚未启用。
`sdkconfig` 是本机生成配置，不提交；公共设置写在 `sdkconfig.defaults`。
它目前仍由同一 C3 集成工程共享，不承诺跨芯片配置隔离。

团队上手与串口选择见 [开发指南](../docs/development.md)。
