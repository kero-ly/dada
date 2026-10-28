## 目的与改动

## 验证

- [ ] PlatformIO Core 6.2.0 / Espressif32 6.8.1 / ESP-IDF v5.3.0 编译通过
- [ ] 涉及硬件时已附实机记录（否则说明未测范围）
- [ ] 新增或修改板卡清单时，清单符合 ADR 0002 的 schema（未知值保持 null）
- [ ] 新增板卡附有 `tests/hardware/records/` 实机记录，且 `verified_capabilities` 仅包含通过项
- [ ] 尚未实测的板卡能力没有标为 verified；仅用于 bring-up 时明确使用 `--allow-unverified`
- [ ] 无密钥、个人路径或未经授权的厂商源码
- [ ] 文档改动已提供中英两份（`X.md` + `X.zh-CN.md`）

测试命令、板卡修订、日志摘要：

## 风险与待办
