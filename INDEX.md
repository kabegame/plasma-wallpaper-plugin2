# Kabegame Plasma 壁纸插件文档索引

## 📚 文档列表

### 🚀 快速开始

- **[README.md](./README.md)** - 项目主文档
  - 功能介绍
  - 安装指南
  - 使用说明
  - 配置选项
  - **IPC 集成概述** ⭐ 新增

- **[QUICKSTART.md](./QUICKSTART.md)** - 5分钟快速开始
  - 最小化安装步骤
  - 快速测试
  - 常见问题

### 🔌 IPC 集成（与 Daemon 通信）

- **[SIMPLE_IPC_INTEGRATION.md](./SIMPLE_IPC_INTEGRATION.md)** - 简易 IPC 集成 ⭐ 推荐入门
  - 文件通信方案
  - 10分钟实现
  - Daemon 推送壁纸
  - 自动更新支持
  - 适合快速原型

- **[IPC_INTEGRATION.md](./IPC_INTEGRATION.md)** - 完整 IPC 集成 ⭐⭐⭐ 生产级
  - D-Bus 通信方案
  - 双向通信
  - 事件监听
  - 复用爬虫系统
  - 画册管理集成
  - 任务状态监听

### 📖 开发文档

- **[STRUCTURE.md](./STRUCTURE.md)** - 项目结构说明
  - 目录组织
  - 文件说明
  - 配置文件格式
  - QML 组件结构

- **[PROJECT_SUMMARY.md](./PROJECT_SUMMARY.md)** - 项目概述
  - 技术栈
  - 设计思路
  - 实现细节
  - 性能考虑

### 🔬 测试与开发

- **[TESTING.md](./TESTING.md)** - 测试指南
  - 安装测试
  - 功能测试
  - 性能测试
  - 调试技巧
  - **IPC 通信测试** ⭐ 新增

- **[NEXT_STEPS.md](./NEXT_STEPS.md)** - 下一步开发计划
  - 待实现功能
  - 优化方向
  - 扩展建议

### 🛠️ 实用工具

- **[install.sh](./install.sh)** - 安装脚本
- **[uninstall.sh](./uninstall.sh)** - 卸载脚本
- **[package.sh](./package.sh)** - 打包脚本

### 🎨 插件核心文件

- **[metadata.json](./org.kabegame.wallpaper/metadata.json)** - 插件元数据
- **[main.xml](./org.kabegame.wallpaper/contents/config/main.xml)** - 配置定义
- **[main.qml](./org.kabegame.wallpaper/contents/ui/main.qml)** - 主界面
- **[config.qml](./org.kabegame.wallpaper/contents/ui/config.qml)** - 配置界面

## 📋 阅读顺序推荐

### 新手用户
1. README.md - 了解功能
2. QUICKSTART.md - 快速安装
3. TESTING.md - 验证功能

### 使用 IPC 集成
1. **README.md** - IPC 集成概述
2. **SIMPLE_IPC_INTEGRATION.md** - 快速实现（10分钟）
3. **TESTING.md** - 测试 IPC 通信
4. **IPC_INTEGRATION.md** - 高级功能（可选）

### 开发者
1. PROJECT_SUMMARY.md - 理解设计
2. STRUCTURE.md - 熟悉结构
3. main.qml - 阅读核心代码
4. **SIMPLE_IPC_INTEGRATION.md** - IPC 实现
5. NEXT_STEPS.md - 了解扩展方向

### 维护者
1. STRUCTURE.md - 项目结构
2. TESTING.md - 测试流程
3. 所有脚本文件 - 自动化工具
4. **IPC_INTEGRATION.md** - D-Bus 服务维护

## 🔍 快速查找

### 我想...

- **安装插件** → QUICKSTART.md → install.sh
- **配置插件** → README.md 的"配置"章节
- **让 Daemon 控制插件** → SIMPLE_IPC_INTEGRATION.md ⭐
- **实现双向通信** → IPC_INTEGRATION.md
- **开发新功能** → STRUCTURE.md → main.qml
- **调试问题** → TESTING.md 的"故障排查"
- **打包分发** → package.sh → README.md 的"分发"
- **了解原理** → PROJECT_SUMMARY.md
- **贡献代码** → NEXT_STEPS.md → STRUCTURE.md

## 🌟 推荐路线

### 快速体验路线（5分钟）
```
QUICKSTART.md → install.sh → 配置插件 → 完成！
```

### IPC 集成路线（15分钟）
```
README.md (IPC 章节)
  ↓
SIMPLE_IPC_INTEGRATION.md
  ↓
实现 Rust 代码
  ↓
测试 IPC 通信
  ↓
完成！现在 Daemon 可以控制插件了！
```

### 高级开发路线（1小时）
```
PROJECT_SUMMARY.md
  ↓
STRUCTURE.md
  ↓
IPC_INTEGRATION.md (D-Bus)
  ↓
实现 D-Bus 服务
  ↓
扩展 QML 功能
  ↓
完成！拥有生产级 IPC 集成！
```

## 📞 支持

如果文档有不清楚的地方，请：
1. 查看 README.md 的"常见问题"
2. 查看 TESTING.md 的"故障排查"
3. **查看 SIMPLE_IPC_INTEGRATION.md 的故障排查** ⭐
4. 提交 Issue 到主项目
5. 联系维护者

## 🎉 开始

**推荐起点**：

- **只想用插件**：[QUICKSTART.md](./QUICKSTART.md) - 5分钟快速体验
- **需要 Daemon 集成**：[SIMPLE_IPC_INTEGRATION.md](./SIMPLE_IPC_INTEGRATION.md) - 10分钟实现 IPC
- **开发新功能**：[PROJECT_SUMMARY.md](./PROJECT_SUMMARY.md) - 了解架构
