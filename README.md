# Kabegame Plasma Wallpaper Plugin

一个功能强大的 KDE Plasma 壁纸插件，使用 C++ 后端 + QML 前端架构，支持多种过渡效果、智能轮播和 Kabegame 集成。

## 功能特性

### 智能模式切换
- **单图模式**：选择单张图片直接显示
- **轮播模式**：选择文件夹自动启用轮播，无需手动开关

### 过渡效果
- **无 (None)**：立即切换
- **淡入淡出 (Fade)**：平滑过渡
- **滑动 (Slide)**：从右向左滑入
- **缩放 (Zoom)**：从小到大放大

### 填充模式
- **填充 (Fill)**：保持比例，填满屏幕（可能裁剪边缘）
- **适应 (Fit)**：保持比例，完整显示（可能有黑边）
- **拉伸 (Stretch)**：拉伸填满屏幕
- **居中 (Center)**：居中显示，不缩放
- **平铺 (Tile)**：重复平铺

### 轮播功能
- 自动扫描文件夹中的图片
- 支持 PNG、JPG、JPEG、BMP、GIF、WEBP、SVG 格式
- 可设置轮播间隔（5秒 - 1小时）
- 支持随机或顺序播放
- 文件夹内容变化时自动刷新

### Kabegame 集成
- 可连接到 Kabegame 后台服务
- 自动同步壁纸更新
- 复用 Kabegame 的爬虫和画册系统

## 安装方法

### 方法 1：C++ 版本（推荐）

需要先安装依赖：

```bash
# Ubuntu/Debian
sudo apt install cmake extra-cmake-modules libkf5plasma-dev qtdeclarative5-dev

# Fedora
sudo dnf install cmake extra-cmake-modules kf5-plasma-devel qt5-qtdeclarative-devel

# Arch Linux
sudo pacman -S cmake extra-cmake-modules plasma-framework qt5-declarative
```

构建并安装：

```bash
cd src-plasma-wallpaper-plugin
./install.sh
```

### 安装后

重启 Plasma Shell 使插件生效：

```bash
kquitapp5 plasmashell && kstart5 plasmashell
```

## 使用方法

1. 右键点击桌面 → **配置桌面和壁纸**
2. 在壁纸类型下拉菜单中选择 **Kabegame 壁纸**
3. 配置壁纸设置：
   - 点击「图片」选择单张壁纸
   - 点击「文件夹」选择文件夹进行轮播
4. 设置填充模式和过渡效果
5. （可选）启用 Kabegame Bridge

## 项目结构

```
src-plasma-wallpaper-plugin/
├── CMakeLists.txt              # 主构建配置
├── build.sh                    # 构建脚本
├── install-cpp.sh              # C++ 版本安装脚本
├── install.sh                  # QML 版本安装脚本
│
├── plugin/                     # C++ 后端
│   ├── CMakeLists.txt
│   ├── wallpaperbackend.h      # 后端接口定义
│   ├── wallpaperbackend.cpp    # 后端实现
│   │   - 文件夹扫描 (QDir)
│   │   - 文件监视 (QFileSystemWatcher)
│   │   - 轮播定时器 (QTimer)
│   ├── kabegamewallpaperplugin.cpp  # QML 插件注册
│   └── qmldir                  # QML 模块声明
│
├── package/                    # QML 前端 (C++ 版本)
│   ├── metadata.json           # 插件元数据
│   └── contents/
│       ├── config/main.xml     # 配置项定义
│       └── ui/
│           ├── main.qml        # 壁纸显示界面
│           └── config.qml      # 配置界面
│
└── org.kabegame.wallpaper/     # 纯 QML 版本（备用）
    ├── metadata.json
    └── contents/
        ├── config/main.xml
        └── ui/
            ├── main.qml
            └── config.qml
```

## 技术栈

- **C++17** - 后端逻辑
- **Qt 5.15** - 核心框架
- **QML** - 用户界面
- **KDE Plasma Framework** - 壁纸插件 API
- **CMake** - 构建系统

## 调试

### 查看日志

```bash
# 实时查看 Plasma Shell 日志
journalctl -f | grep -i "kabegame"

# 或者在终端启动 Plasma Shell 查看完整输出
kquitapp5 plasmashell && plasmashell 2>&1 | tee plasma.log
```

### 日志输出示例

```
[Kabegame] WallpaperBackend 初始化
[Kabegame] 设置路径: /home/user/Pictures/Wallpapers
[Kabegame] 路径类型: 文件夹
[Kabegame] 加载文件夹: /home/user/Pictures/Wallpapers
[Kabegame] 找到 42 张图片
[Kabegame] 启动轮播定时器，间隔: 10 秒
[Kabegame] 定时器触发
[Kabegame] 随机选择索引: 17
[Kabegame] 切换壁纸: /home/user/Pictures/Wallpapers/image17.jpg
```

## IPC 集成

### 简单方案：文件通信

Daemon 写入 JSON 文件，插件监听文件变化：

```json
{
  "wallpaper": "/path/to/image.jpg",
  "timestamp": 1234567890
}
```

详见 `SIMPLE_IPC_INTEGRATION.md`

### 完整方案：D-Bus 通信

支持双向通信、实时事件、画册查询等。

详见 `IPC_INTEGRATION.md`

## 文档索引

| 文档 | 说明 |
|------|------|
| `QUICKSTART.md` | 快速开始指南 |
| `TESTING.md` | 测试指南 |
| `STRUCTURE.md` | 项目结构说明 |
| `SIMPLE_IPC_INTEGRATION.md` | 简单 IPC 集成 |
| `IPC_INTEGRATION.md` | 完整 IPC 集成 |
| `NEXT_STEPS.md` | 功能扩展建议 |

## 功能状态

- [x] 基础壁纸显示
- [x] 多种填充模式
- [x] 过渡动画效果
- [x] 文件夹轮播
- [x] C++ 后端重构
- [x] 智能模式切换
- [x] 文件夹监视
- [ ] D-Bus 集成
- [ ] 多显示器支持
- [ ] 旋转过渡效果

## 许可证

MIT License

## 相关链接

- [Kabegame 主项目](https://github.com/kabegame/kabegame)
- [KDE Plasma 文档](https://develop.kde.org/docs/plasma/)
- [Qt QML 文档](https://doc.qt.io/qt-5/qmlapplications.html)
