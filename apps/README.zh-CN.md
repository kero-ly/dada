# 应用

七个原生 C 应用，全部只依赖能力契约——所以它们里面没有一个引脚号、板名或驱动调用。

最快的上手方式是**选一个和你目标最接近的应用**来读。

| 应用 | 需要的能力 | 是什么 | 什么时候读它 |
| --- | --- | --- | --- |
| `touch-counter` | display, input | 最小的输入→输出闭环：点一下加一，下四分之一重置 | 只想先证明板子是好的 |
| `bringup` | display, input, storage | 硬件诊断：编号网格、触点标记、存储自检 | 屏幕或触摸行为可疑时 |
| `focus-clock` | display, input, **time** | 25 分钟番茄钟，开始/暂停/重置 | 想看时间契约怎么用 |
| `snake` | display, input, time, **storage** | 贪吃蛇，滑动转向，最高分落盘 | 想看一个完整的小游戏 |
| `paint` | display, input, **storage** | 48×64 像素画板，15 色，位图打包进 NVS | 想看 blob 存储 |
| `whale` | display, **storage** | 160×120 头像，按条带渲染后旋转铺满 | 想看非交互渲染 |
| `flappy` | display, input, time, storage | 从作者 Arduino 版本完整移植的游戏 | 想看一个既有项目迁到契约上 |

## 共同点

- **只依赖 `contracts/`**：没有 GPIO 编号、没有板型判断、没有驱动初始化。
- **声明式需求**：每个 `app.json` 写明需要的能力，`tools/dada.py check` 会在
  构建前拒绝能力不匹配的板子——在烧录之前。
- **规则与像素分离**：`counter.c`、`clock.c`、`snake.c`、`paint.c`、`flappy.c`
  只放逻辑、不含 UI，因此能在主机测试里跑。
- **降级而不是崩溃**：多数应用的 storage 是可选的，没有它只是不记最高分。

## 新增一个应用

1. `apps/<id>/app.json`：id、`format: native-c-v1`、需要的能力、最小显示尺寸。
2. 实现 `contracts/app.h` 里的 `dada_app_start/step/stop`。
3. 加 `CMakeLists.txt`，`REQUIRES dada_contracts`（需要画布辅助再加 `app_support`）。
4. 在三处登记：`firmware/CMakeLists.txt` 的 `DADA_APP` 候选值、
   `firmware/platformio.ini` 的一个 env、`.github/workflows/firmware.yml` 的矩阵。
5. 把逻辑单独放一个 `.c`，这样能进 `tests/host/`。

应用里不要出现板级知识；如果它需要契约没提供的东西，那是契约该讨论的问题，
而不是 `#include <driver/...>`。
