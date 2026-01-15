# Kabegame Plasma Wallpaper Plugin

一个功能强大的 KDE Plasma 壁纸插件，支持多种过渡效果和填充模式。

## 功能特性

### 过渡效果
- **None**: 无过渡，立即切换
- **Fade**: 淡入淡出效果
- **Slide**: 滑动切换效果
- **Zoom**: 缩放过渡效果
- **Rotate**: 旋转效果（计划中）

### 填充模式
- **Fill**: 保持比例，填满屏幕（可能裁剪）
- **Fit**: 保持比例，完整显示（可能有黑边）
- **Stretch**: 拉伸填满屏幕
- **Center**: 居中显示，不缩放
- **Tile**: 平铺

### 轮播功能
- 支持文件夹轮播
- 可设置轮播间隔（5秒 - 1小时）
- 支持随机或顺序播放

### Kabegame 集成（可选）
- 可连接到 Kabegame 后台服务
- 自动同步壁纸更新
- 复用 Kabegame 的爬虫和画册系统

## 安装方法

### 方法 1：手动安装

```bash
# 进入插件目录
cd src-plugin

# 复制到 Plasma 插件目录
cp -r org.kabegame.wallpaper ~/.local/share/plasma/wallpapers/

# 重启 Plasma Shell
kquitapp5 plasmashell && kstart5 plasmashell
```

### 方法 2：打包安装

```bash
# 打包插件
cd src-plugin
tar -czf kabegame-wallpaper.tar.gz org.kabegame.wallpaper

# 在 KDE 系统设置中导入
# 系统设置 → 外观 → 壁纸 → 获取新壁纸 → 从文件安装
```

## 使用方法

1. 打开 KDE 系统设置
2. 导航到 **外观 → 壁纸**
3. 在壁纸类型下拉菜单中选择 **Kabegame Wallpaper**
4. 配置你的壁纸设置：
   - 选择单张图片或文件夹
   - 设置填充模式
   - 选择过渡效果和时长
   - （可选）启用轮播
   - （可选）启用 Kabegame Bridge

## 配置说明

### 基础配置

- **Wallpaper Image**: 单张壁纸的路径
- **Slideshow Folder**: 轮播文件夹路径
- **Fill Mode**: 图片填充模式
- **Transition Effect**: 切换过渡效果
- **Transition Duration**: 过渡时长（毫秒）

### 轮播配置

- **Enable slideshow**: 启用轮播
- **Slideshow Interval**: 轮播间隔（秒）
- **Slideshow Order**: 播放顺序（随机/顺序）

### Kabegame 集成

- **Enable Kabegame Bridge**: 启用与 Kabegame 的集成
- 需要 Kabegame 主应用运行中

## 开发

### 目录结构

```
org.kabegame.wallpaper/
├── metadata.json              # 插件元数据
├── contents/
│   ├── config/
│   │   └── main.xml          # 配置定义
│   └── ui/
│       ├── main.qml          # 主界面（壁纸显示）
│       └── config.qml        # 配置界面
```

### 调试

```bash
# 查看 Plasma Shell 日志
journalctl -f | grep plasmashell

# 或者在终端重启 Plasma Shell 查看输出
kquitapp5 plasmashell && plasmashell
```

### 测试

1. 修改 QML 文件后，重启 Plasma Shell 生效
2. 可以在 `main.qml` 中使用 `console.log()` 输出调试信息
3. 在系统设置中测试不同的配置选项

## 技术栈

- QML (Qt Quick 2.15)
- KDE Plasma Framework
- KConfig (配置系统)

## IPC 集成：接受 Daemon 控制

Kabegame Plasma 壁纸插件可以通过 IPC 与 Kabegame Daemon 通信，实现自动壁纸更新。

### 方案选择

我们提供**两种**集成方案：

#### 1. 简单方案：文件通信（推荐入门）

**特点**：
- ⭐ 10分钟搞定
- ⭐ 无需额外依赖
- ⭐ 易于调试

**使用场景**：
- Daemon 控制插件切换壁纸
- 任务完成后自动更新
- 单向通信足够

**文档**：详见 `SIMPLE_IPC_INTEGRATION.md`

#### 2. 完整方案：D-Bus 通信（推荐生产）

**特点**：
- ⭐⭐⭐ 双向通信
- ⭐⭐⭐ 实时事件
- ⭐⭐⭐ 复用爬虫系统

**使用场景**：
- 插件查询 Daemon 画册
- 监听任务事件
- 反馈插件状态
- 运行爬虫插件

**文档**：详见 `IPC_INTEGRATION.md`

### 快速示例

**Daemon 侧（Rust）**：

```rust
use kabegame_core::plasma_sync::PlasmaSync;

// 任务完成后
let sync = PlasmaSync::new();
sync.sync("/path/to/wallpaper.jpg")?;
```

**插件侧（QML）**：

```qml
FileSystemWatcher {
    files: ["/tmp/kabegame-wallpaper.json"]
    onFileChanged: loadWallpaperFromFile()
}
```

### 启用方法

1. 安装插件
2. 右键桌面 → 配置桌面和壁纸
3. 选择 "Kabegame Wallpaper"
4. 启用 "Enable Kabegame Bridge"
5. 启动 Kabegame Daemon

现在插件会自动接收 Daemon 推送的壁纸！

## 文档索引

- `SIMPLE_IPC_INTEGRATION.md` - 简单 IPC 集成（文件通信）⭐ 推荐入门
- `IPC_INTEGRATION.md` - 完整 IPC 集成（D-Bus）⭐⭐⭐ 生产级
- `TESTING.md` - 测试指南
- `NEXT_STEPS.md` - 功能扩展建议
- `STRUCTURE.md` - 项目结构说明
- `QUICKSTART.md` - 快速开始
- `PROJECT_SUMMARY.md` - 项目概述

## 待实现功能

- [ ] 旋转过渡效果
- [ ] 更多过渡效果（擦除、马赛克等）
- [ ] 文件夹图片列表加载
- [x] Kabegame IPC 集成（文件通信）✅
- [ ] Kabegame D-Bus 集成（完整版）
- [ ] 多显示器支持
- [ ] 每个显示器不同壁纸

## 许可证

MIT License

## 贡献

欢迎提交 Issue 和 Pull Request！

## 相关链接

- [Kabegame 主项目](https://github.com/yourname/kabegame)
- [KDE Plasma 文档](https://develop.kde.org/docs/plasma/)
- [QML 文档](https://doc.qt.io/qt-6/qmlapplications.html)
