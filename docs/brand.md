# 命名与品牌

**Status:** Proposed · 2026-10-25 · 待所有者决定

## 问题

现在的名字 `dada / 搭搭` 有三个搜索与商标层面的麻烦：

| 冲突源 | 说明 |
| --- | --- |
| 达达集团（NASDAQ: DADA） | 京东系即时配送，中文“达达”在物流类目有强商标 |
| 达达主义（Dadaism） | 艺术流派，占满搜索结果 |
| Dada mail | 老牌邮件列表软件 |

后果：搜“dada”几乎不可能命中本项目；口口相传时也只能说“dada”，没有区分度。

## 域名实测（2026-10-25，DNS SOA 查询，`google.com` 作对照）

| 候选 | .com | .dev | 判断 |
| --- | --- | --- | --- |
| `dadaboard` | NXDOMAIN | NXDOMAIN | 大概率可用 |
| `dadastack` | NXDOMAIN | NXDOMAIN | 大概率可用 |
| `dadakit` | 已注册 | NXDOMAIN | .com 被占 |
| `dadahub` | 已注册 | 已注册(.io/.org) | 被占 |
| `dadaiot` | 已注册 | — | 被占 |
| `dadamod` | 已注册 | — | 被占 |

> NXDOMAIN 是强信号但不是注册证明，注册前请在注册商或 whois 复核。

## 建议

**英文名改用 `dadastack`，中文名保留“搭搭”。**

理由：

1. `stack` 直接对应五层架构（应用 / 运行时 / 组件 / 板级 / 硬件），名字即结构。
2. 比 `board` 更有延展性——以后做云服务、做编辑器都不违和。
3. 加了限定词后，“dadastack”在搜索引擎里是干净的。

长期方案（如果将来要彻底去 dada 化）：`tessera`（马赛克小块，拉丁语），
“一块块拼起来”正好是产品隐喻，词罕见、好念、商标空间大。中文仍叫“搭搭”。

## 仓库描述必须带的关键词

GitHub 搜索靠描述文本，建议：

> 一次开发，自由组合 · ESP32 / LVGL / 触摸屏 HMI 开源平台 · Open-source platform for screen+touch devices

## 待办

- [ ] 所有者确认英文名（`dadastack` / `tessera` / 维持 `dada`）
- [ ] 注册域名（建议 `.dev` + `.com` 双持，`.dev` 强制 HTTPS，适合开发者项目）
- [ ] 建 GitHub Organization（便于以后拆分组织仓库）
- [ ] 商标检索（至少查第 9 类“计算机硬件/软件”）
