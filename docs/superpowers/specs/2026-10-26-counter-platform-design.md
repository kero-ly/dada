# C3 counter and incremental platform design

> Summary: turn the working board bring-up into a selectable, hardware-independent
> native app, preserving the diagnostic app. This is an incremental platform slice.

## Intent / 意图

用户已选择先固化硬件、再做触摸计数器，并要求架构随验证逐步推进。
本阶段交付一条可复现的「选择开发板与应用 → 构建 → 烧录 → 验证」路径。

## Approach / 方案选择

沿用已讨论并获用户同意的最小独立应用方案。另两种选择是继续把交互写在
`firmware/main`（不能验证应用边界），或立即引入 LVGL + YAML 编译器与通用运行时
（当前没有足够实际需求支持这些抽象）。本次采用原生 C 与现有 sink 级契约。
ADR 0005 仍是 Proposed 的后续方向，本次不实现或承诺 IR。

## Scope / 本次边界

- 保留 mode 3、反色、小端 RGB565 修复与硬件实测证据。
- `contracts/app.h` 定义实验性单应用生命周期：`start(services)`、`step()`、`stop()`。
  services 只有 display 与 input；应用通过契约函数指针访问，不能依赖 ESP-IDF。
- `apps/touch-counter`：原生 C，按下增一次，长按不连增；按下时命中的区域决定动作，
  按住滑动不触发另一个按钮。上部 3/4 区域为加一，下部 1/4 为 RESET；
  四位数 0000–9999，达到上限保持；断电不保存。分辨率相对布局，最小 160×160。
- UI 只画本应用需要的数字、加号和 RESET，渲染器私有，不新增平台绘图契约。
  使用 RGB565 小端条带缓存，不分配全屏缓冲；无动画，仅初始及计数变化刷新。
- `apps/bringup`：保存现有五色条与触摸诊断，用相同生命周期接入。
- `firmware/main` 负责板初始化、选中应用的生命周期、FreeRTOS 调度与错误日志。
  此处是静态装配入口，暂不实现 `runtime/` 注册中心或多应用调度。
- `DADA_APP=touch-counter|bringup` 构建期选择，默认 touch-counter；未知 id、缺少
  display/input 的目标板要在配置时失败。应用源码在主机无 SDK 下可编译测试。
- 提供 Python 薄封装 `tools/dada.py`：build / flash / monitor / ports。
  不安装 SDK、不猜端口；命令明确携带 board、app，构建目录按组合隔离。
- CI 构建两个应用；主机测试验证点击、长按、滑动、无效输入、边界、显示错误与布局。

## Architecture / 分工

```text
应用： apps/touch-counter     apps/bringup
                    ↓ contracts/app.h + display.h + input.h
静态装配：firmware/main       主机交付：tools/dada.py → ESP-IDF
                    ↓ 注入 display/input
硬件绑定：hardware/<vendor>/<board>/board
                    ↓ 配置、构造
复用驱动：components/display_st7789 + touch_cst836u
                    ↓ ESP-IDF → ESP32-C3 + panel
```

这是构造依赖与运行调用的组合示意。应用不知道板卡，固件装配代码知道双方。
`runtime/` 后续承接被两个真实应用证明有必要复用的生命周期、时钟与 UI 服务。

## Validation and next gates / 验收与下一步

1. 当前修复已实测：mode 0 黑屏、mode 3 显示，反色与小端后红绿蓝白黑正确，触摸事件正常。
   断电冷启动和坐标对应尚未验证，不能补写为通过。
2. 本次：主机测试通过、两个应用编译通过；烧录计数器并由用户确认点击和重置。
3. 第二个业务应用：离线番茄钟，引入单调时间、共享 UI 后再提取最小 runtime。
4. 第二块板：同一应用源码不变，只新增板级描述与绑定；通过后再扩充 manifest 组件组合、
   资源/引脚校验、构建矩阵。尚不宣称跨板支持。
5. 第三方贡献与交付：有第二块板证据之后制定支持矩阵、可重复构建与验证报告协议；
   AI 连接器、声明式 IR、市场、OTA 均由具体应用需求触发。

## Constraints / 约束

ESP-IDF v5.4.2；不上传商家材料、个人路径、备份固件或凭据；不新增第三方库。
所有新增契约、应用逻辑使用标准 C11；硬件相关参数只在硬件绑定及驱动中。
首版 app API 标为 experimental，不承诺 ABI 或动态加载。未知能力只据 manifest 做静态
检查，不把声明的能力视为实测通过。保持当前触摸坐标方向，实机检查后再决定变换。
