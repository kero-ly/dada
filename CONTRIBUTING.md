# 团队协作

1. 先读 README.md 与 docs/development.md，使用 ESP-IDF v5.4.2。
2. 从 main 创建短期分支，例如 `feat/c3-display`、`fix/touch-coordinates`。
3. 每个 PR 聚焦一个功能；说明改动、构建结果及硬件验证情况。
4. 合并前通过 CI，涉及硬件时附 tests/hardware/ 中的测试记录。

## 边界

- apps/：业务状态与交互，不写板名、GPIO、芯片条件分支。
- runtime/：应用公共能力与必要运行服务。
- hardware/：板型、修订、接线和组件绑定。
- components/：跨板可复用驱动。
- firmware/：集成入口；tools/：主机侧构建交付。

不要把未来抽象提前写满。先跑通厂商示例，再从真实应用抽取接口。

## 提交内容

提交源码、共享 sdkconfig.defaults、依赖锁文件和验证记录。不要提交生成的 sdkconfig、build/、SDK、下载缓存、个人路径、Wi-Fi 密码或 API key。
第三方源码需保留原许可证并记录来源、版本与本地修改；没有授权的厂商压缩包先不要上传公共仓库。

目前许可证待团队决定，不自动默认 MIT 或 Apache-2.0。不要将尚未测试的组合声明为已支持。
