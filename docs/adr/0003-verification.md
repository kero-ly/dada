# ADR 0003 · 硬件验证协议与报告格式

**Status:** Proposed · 2026-10-25

> **Summary (EN).** A board counts as supported only when a machine-readable verification
> report exists. `dada verify` runs a self-test firmware on the device and emits JSON
> (chip, flash, heap, I²C scan, display pattern, touch samples). Reports are committed under
> `tests/hardware/records/` and attached to board PRs. CI validates the report's schema and
> checks it matches the manifest.

## 背景

「支持一块板」如果只靠作者手写一句「测试通过」，这个项目在收到第三块社区板时就会失控。
需要的是一份**任何人可以复现、机器可以比对**的证据。

今天（2026-10-24）第一次把一块板从开箱跑到触摸可用，整个过程里最有价值的产物不是驱动代码，
而是那些**坑**：CS 接地、SPI 60MHz 实际落到 40MHz、附送的线只有电源没有数据、
`lv_conf.h` 对库不生效。这些必须变成结构化的数据，而不是聊天记录。

## 决策

### 1. 三级验证，成本递增

| 级别 | 在哪跑 | 检查什么 | 需要硬件 |
| --- | --- | --- | --- |
| L1 · 清单校验 | CI | Schema、字段自洽、id 与目录一致 | 否 |
| L2 · 编译矩阵 | CI | 每块板 × 每个 app 能否编译、能力断言是否通过 | 否 |
| L3 · 实机自检 | 设备 | 芯片、Flash、堆、I²C 扫描、显示、触摸 | **是** |

**只有 L3 通过的能力才能写进 `verified_capabilities`。**

### 2. `dada verify` 输出 JSON

```json
{
  "report_version": "0.1",
  "board_id": "jinyichen-esp32-c3-lcd-2.4",
  "firmware_git_sha": "8698c91",
  "dada_version": "0.1.0",
  "toolchain": { "framework": "esp-idf", "version": "v5.4.2" },
  "timestamp": "2026-10-25T12:00:00+08:00",
  "chip": { "model": "esp32c3", "revision": "v0.4", "cores": 1, "clock_mhz": 160 },
  "flash": { "size_mb": 4, "vendor": "XMC" },
  "heap": { "free_bytes": 306916, "min_free_bytes": 280000 },
  "usb": { "mode": "serial-jtag", "vid_pid": "303A:1001" },
  "i2c": { "scl": 5, "sda": 4, "devices": [ "0x15" ] },
  "display": {
    "controller": "ST7789", "width": 240, "height": 320,
    "test": "color-bars", "result": "pass"
  },
  "touch": {
    "controller": "CST836U", "address": "0x15",
    "samples": 200, "points_detected": 198,
    "bbox": [ 4, 3, 236, 317 ], "result": "pass"
  },
  "notes": [ "附送的 USB-C 线只有电源，必须用数据线" ]
}
```

设计要点：

- **`firmware_git_sha`** 让报告能对应到某一次提交，避免「用旧固件验证新代码」。
- **`result` 只有 `pass / fail / skip`**，没有「大概可以」。
- **`notes`** 用来记录无法结构化的坑（比如线材问题）。这是本项目最值钱的部分。
- 报告是 JSON，所以 CI 能直接比对：`manifest.verified_capabilities` 必须 ⊆ 报告里 pass 的项。

### 3. 报告与 PR 的关系

- 报告提交到 `tests/hardware/records/YYYY-MM-DD-<board_id>.md`（人读）+ 同名 `.json`（机读）。
- 新增板卡的 PR **必须**附一份报告，否则 CI 直接失败。
- 作者本人无法验证的板子：由贡献者跑 `dada verify` 并提交报告，维护者只做清单评审。

这是扩展支持面**唯一可行**的方式：一个人不可能买齐所有板子。

### 4. 防伪与边界

- 报告不加密、不签名。它证明的是「这台机器上跑过」，不是「这块板永久正常」。
- 报告里的 `toolchain` 必须如实填写。**用 Arduino 验证的结果不能写成 ESP-IDF**——
  第一次验证（2026-10-24）就是 Arduino 工具链，报告里必须写明，否则会误导后续排障。

## 后果

- 支持列表变成可审计的数据，而不是口碑。
- 社区可以用同一套工具产出可信报告，维护成本可控。
- 代价：需要写一个自检固件（`tools/verify-firmware/`）并长期维护，它要跟着契约版本走。

## 备选方案

| 方案 | 否决原因 |
| --- | --- |
| 只写人读的验证记录 | 无法自动比对，社区报告质量参差 |
| 自动化 CI 硬件农场 | 成本极高，一个人维护不了，先做众包 |
| 不验证，靠「社区反馈」 | 支持列表会变成谎话，早期项目最忌讳 |

## 未决问题

1. 自检固件要不要复用正式固件 + 一个隐藏的 verify 应用？（更省事，但会污染发布固件）
2. 报告要不要带屏幕截图？（拍照上传 vs 设备端把帧缓冲哈希进报告）
3. 触摸 bbox 的判定阈值定多少算 pass？（当前实测覆盖 4..236 / 3..317）
