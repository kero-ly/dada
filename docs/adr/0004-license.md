# ADR 0004 · 许可证选择

**Status:** Accepted · 2026-10-27 · 仓库所有者确认采用 Apache-2.0

> **Summary (EN).** Unless otherwise noted, dada is licensed under Apache-2.0.
> The canonical text is in the root LICENSE. Third-party materials retain their original
> licenses; unlicensed vendor samples are not covered by this decision. Intentional
> contributions are governed by section 5 unless explicitly stated otherwise or separately agreed.

## 背景

项目希望硬件厂商和应用作者能够参与，也允许商业产品采用。此前仓库未选定许可证，
缺少对使用、修改与分发的明确授权。所有者于 2026-10-27 确认采用 Apache-2.0。

## 决策

### 1. 授权范围

根目录 [LICENSE](../../LICENSE) 保存 Apache License 2.0 的完整标准文本。
除另有标注外，适用于仓库内项目自有的代码、文档和清单；不对第三方内容重新授权。
当前不另设板级清单的数据许可证，也不要求下游独立应用一律采用 Apache-2.0。

允许使用、修改、商用及闭源衍生开发，但分发时仍需满足许可证第 4 条，包括提供许可证、
保留适用声明、醒目标记修改文件；若作品包含适用的 NOTICE，需保留其中相关归属声明。
不要求下游公开修改或向本仓库提交 PR。

选择理由是宽松的采用条件、明确且有范围限制的贡献者专利授权，以及明确的贡献规则。
Apache-2.0 不保证不存在第三方专利风险，也不阻止别人 fork 或销售竞争产品。
第 6 条不授予商标权；品牌使用政策与商标注册是独立事项，本决定不声称已有注册商标。

### 2. 贡献与第三方来源

有意提交供项目纳入的贡献遵循 Apache-2.0 第 5 条，除非贡献者明确另行声明或存在独立协议。
贡献者应有权提交相关内容。第三方源码、依赖及素材保留各自许可证、版权声明和来源信息；
不能通过放入本仓库自动转换成 Apache-2.0。

商家提供源码不等于提供授权。未明确许可的示例或压缩包不得复制进公开仓库。
可核对接口、引脚、初始化时序等事实，并依据可用资料实现适配；如实记录实际参考来源，
不隐藏对商家资料的参考，也不将阅读源码后的重写自动称为独立 clean-room 实现。

### 3. 依赖管理

新增或升级第三方依赖时，记录名称、版本、来源、许可证及所需声明，并检查分发要求。
ESP-IDF 等上游包含多种许可的组件，不能仅用 SDK 的主要许可证概括所有依赖。
统一依赖许可清单与自动审计尚未建立，本次决定不声称已完成完整供应链许可审计。

## 后果

- README、贡献指南和智能体指南统一采用 Apache-2.0，不再标记许可证待定。
- 第三方许可义务仍需逐项遵守；项目自有许可不覆盖未经授权的商家资料。
- 后续如需变更许可证，必须评估贡献者权利和既有版本的授权，不能假定所有者可任意重新授权全部贡献。
