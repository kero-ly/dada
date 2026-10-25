# ADR 0004 · 许可证选择

**Status:** Proposed · 2026-10-25 · **需要仓库所有者拍板，本文不能替代法律意见**

> **Summary (EN).** Choose a license before accepting outside contributions, otherwise the
> project cannot be legally reused. Recommendation: **Apache-2.0** (explicit patent grant +
> contribution terms), with MIT as the lighter alternative. Separately: vendor code from the
> board seller has **no license at all** and must never be copied into this repository.

## 背景

仓库现在是**公开但无许可证**状态。这在法律上等于「保留所有权利」：别人不能合法使用、
修改或分发。对一个目标是「开源平台」的项目，这是必须尽快解决的第一号问题。

更紧迫的是第二件事：**卖家提供的源码包没有任何 LICENSE 文件**。

2026-10-24 的核查结果：

| 内容 | 授权状态 |
| --- | --- |
| 卖家自己的 PlatformIO 工程（`最新版程序优化后.rar`） | **无任何 LICENSE / COPYING / NOTICE**，文档中也未提及 |
| TFT_eSPI 2.5.43（依赖） | BSD 系，允许商用与再分发 |
| LVGL 8.4.0（依赖） | MIT |
| ESP-IDF / Arduino-ESP32 | Apache-2.0 |

「拿到源码」不等于「拿到授权」。在没有明确许可的情况下，把它复制进仓库属于侵权，
以后商用会被追责。**这条必须写进 CONTRIBUTING。**

## 决策（建议）

### 1. 采用 Apache-2.0

| 理由 | 说明 |
| --- | --- |
| 显式专利授权 | 硬件/固件项目会碰到实现专利问题，MIT 不含专利条款，Apache-2.0 有 |
| 贡献条款明确 | 第 5 条直接定义「提交即授权」，省掉 CLA 的麻烦 |
| 商标条款隔离 | 第 6 条明确不授予商标权，保护「搭搭」品牌，防止别人拿代码去冒充官方板 |
| 生态兼容 | 与 ESP-IDF（Apache-2.0）同许可，企业采用时法务阻力最小 |

MIT 更短更宽松，但对一个**要做生态、要防山寨**的平台，Apache-2.0 的两个额外条款
（专利 + 商标）值得多出来的那点篇幅。

### 2. 厂商代码零容忍

- **不得**把卖家工程中的任何源文件（含 `.ino`/`.cpp`/`.h`/`User_Setup.h`）复制进本仓库。
- 允许的行为：阅读其引脚与初始化参数，**按 ESP-IDF 组件接口重写**（clean-room）。
- 每个 `components/` 下的驱动，README 中注明「参考了哪些公开资料」（数据手册页码），
  不注明「参考了卖家源码」。

### 3. 依赖许可清单

`LICENSES/` 目录记录直接依赖的名称、版本、许可与来源，CI 在依赖变更时提醒更新。

## 后果

- 别人可以合法 fork、商用、二次分发。
- 卖家日后若主张权利，仓库里没有可被指认的复制品。
- 代价：贡献者提交的代码默认按 Apache-2.0 授权，需要在 CONTRIBUTING 里说清楚。

## 未决问题

1. 是否需要对**板级清单**（`board.json`）单独声明为 CC0/CC-BY？
   清单是数据不是代码，单独授权更利于别人复用。
2. 是否注册「搭搭 / dadastack / tessera」商标？（与 ADR 无关，但影响品牌策略）
3. 商用衍生品（别人做板子预装 dada）是否要求署名或回馈？Apache-2.0 不要求，需另行决定。
