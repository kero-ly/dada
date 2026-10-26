# 验收记录 · 2026-10-26 · 贪吃蛇（首个"好玩"应用）

路线图 gate 3 之后新增的示范应用，验证"能力组合"能不能撑起一个像样的东西。

- 板卡：金逸晨 ESP32-C3 2.4 寸触摸开发板
- 工具链：ESP-IDF v5.4.2，`DADA_APP=snake`
- 能力组合：**display + input + time + storage**

## 结果：可玩 —— 通过

| 操作 | 期望 | 实测 |
| --- | --- | --- |
| 启动 | 棋盘 + 白色方框 + 播放三角，状态 READY | 通过 |
| 点击 | 开局，蛇向右移动 | 通过 |
| 滑动 | 转向（上下左右，禁止反向） | 通过 |
| 撞墙/撞自己 | 结束，红框，分数记录 | 通过 |
| 结束后点击 | 开新一轮 | 通过（修复后） |

串口证据：

```
snake: ready (12x14 cells), tap to start
I (401) app_host: snake started (240x320)
snake: release (102,183) from (102,183) state=0   ← 点击开局
snake: game over, score 20
```

## 实机暴露的三个问题

这三个都不是"逻辑写错"，而是**只在硬件上才出现**的工程问题：

### 1. 结束画面刷新矩形越界

红框从 y=36 起刷 288 行，`36+288 = 324 > 320`，`canvas_flush` 返回
`DADA_ERR_INVALID_ARG`，宿主收到错误后停止应用。
修复：按画布高度裁剪。

### 2. 上电噪声被当成点击

触摸控制器在上电后短时报一次"按下"，被判定为点击，游戏自己就开局了。
修复：启动后 400 ms 内忽略输入。

### 3. `canvas_flush` 自己吃掉双倍内存（最重要）

症状：撞墙后应用直接停掉，日志 `app_host: snake stopped: 1`（`DADA_ERR_FAIL`）。

原因不在游戏规则，而在应用侧辅助函数：

```
整帧画布         150 KB（已在堆中）
+ overlay 刷新   240×284 → 再 malloc 136 KB
= 堆不足 → flush 失败 → 宿主 stop
```

修复：改为每 16 行打包发送一次，临时缓冲固定约 7.7 KB，
与刷新区域大小无关。新增 `tests/host/test_canvas.c` 覆盖条带数量、
奇数高度、越界拒绝与像素来源。

**教训**：抽象层自身的内存开销会被忽略，逻辑推理看不出来，只能实机撞。

## 未覆盖

- 最高分持久化未实测（`storage` 本身已用 bringup 自检验证过：0→1→2→3）
- 长局稳定性（单局最长约 20 分）
- 多点触控、长按、快速连续滑动
- 不同分辨率下的布局（代码按屏宽自适应，但只在 240×320 上跑过）

## 复现

```sh
python tools/dada.py build --app snake
python tools/dada.py flash --app snake --port YOUR_PORT
```

主机测试（不需要板子）：

```sh
cmake -S tests/host -B tests/host/build
cmake --build tests/host/build
ctest --test-dir tests/host/build --output-on-failure
```
