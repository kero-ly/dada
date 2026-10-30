# 应用

七个原生 C 应用，全部只依赖能力契约——所以它们里面没有一个引脚号、板名或驱动调用。

最快的上手方式是**选一个和你目标最接近的应用**来读。

| 应用 | 需要的能力 | 是什么 | 启动后应该看到 | 什么时候读它 |
| --- | --- | --- | --- | --- |
| `touch-counter` | display, input | 最小的输入→输出闭环：点一下加一，下四分之一重置 | `0000` 和 `TAP +`，屏幕底部有一条较亮的重置条 | 只想先证明板子是好的 |
| `bringup` | display, input, storage | 硬件诊断 | 3×3 编号网格；点哪格留下一个红色方块；日志打印启动计数 | 屏幕或触摸行为可疑时 |
| `focus-clock` | display, input, **time** | 25 分钟番茄钟 | `25:00`、进度条、状态词（`TAP`/`RUN`/`PAUSE`/`DONE`）、底部 `RESET` | 想看时间契约怎么用 |
| `snake` | display, input, time, **storage** | 贪吃蛇，滑动转向 | 12×14 棋盘、绿色蛇身、红色食物、顶部分数 | 想看一个完整的小游戏 |
| `paint` | display, input, **storage** | 48×64 像素画板，15 色 | 空白白色画布，右下角一个调色板按钮 | 想看 blob 存储 |
| `whale` | display, **storage** | 160×120 头像 | 头像铺满屏幕；把板子横过来看 | 想看非交互渲染 |
| `flappy` | display, input, time, storage | 从作者 Arduino 版本完整移植的游戏 | 天空、绿色管道、地面、黄色小鸟、顶部分数 | 想看一个既有项目迁到契约上 |

## 建议阅读顺序

由易到难，每一步只多一个概念：

1. **`touch-counter`**——53 行讲完整个生命周期：start、poll、step、stop。
2. **`focus-clock`**——引入 `time` 契约，并把规则（`clock.c`）和像素（`render.c`）分开。
3. **`snake`**——引入 `storage`、转向队列，以及第二个主机测试文件。
4. **`paint`**——引入打包 blob、弹出式调色板、带长度校验的加载。
5. **`whale`**——完全不做输入：渲染、旋转、条带刷新。
6. **`flappy`**——最大的一个，也是"把既有代码迁到契约上"的完整示例。

`bringup` 是工具而不是示例：硬件出问题时才用，不用来学写法。

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
