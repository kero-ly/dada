# 平台运行库（预留）

当前单应用生命周期由 `contracts/app.h` 定义，`firmware/main` 完成服务注入和 20ms 调度。
这里尚无公共运行服务实现，不额外复制一套生命周期管理器。

下一步以离线番茄钟引入可注入时间能力；只在计数器和番茄钟实际需要复用时，
提取事件或绘图服务。第二块板验证后，再评估能力注册、版本化 SDK 与通用运行框架。
见 [增量路线](../docs/architecture/incremental-roadmap.zh-CN.md)。
