# Kabegame Plasma Wallpaper Plugin

一个功能强大的 KDE Plasma 壁纸插件，使用 C++ 后端 + QML 前端架构，支持多种过渡效果、智能轮播和 Kabegame 集成。

> **需要 Plasma 6**：本插件仅支持 KDE Plasma 6，不支持 Plasma 5。需先安装 [Kabegame 主程序](https://github.com/kabegame/kabegame#installation)。

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

### 方法 1：deb 包（推荐，需 Plasma 6）

1. 确保已安装 [Kabegame 主程序](https://github.com/kabegame/kabegame#installation)。
2. 从 [Releases](https://github.com/kabegame/plasma-wallpaper-plugin2/releases) 下载 `kabegame-plasma-wallpaper_*_amd64.deb`。
3. 安装：
   ```bash
   sudo dpkg -i kabegame-plasma-wallpaper_*_amd64.deb
   ```
   若依赖报错：`sudo apt-get install -f`
4. 重启 Plasma Shell：
   ```bash
   kquitapp6 plasmashell 2>/dev/null; kstart6 plasmashell &
   ```
   或注销后重新登录。

**本地打 deb**：在仓库根执行 `./build-deb.sh`，`.deb`、`.changes`、`.buildinfo` 会整理到 `dist/`，不会写到父目录。

### 方法 2：从源码构建（需 Plasma 6 / KF6）

需要先安装依赖：

```bash
# Ubuntu/Debian (需 KF6，可添加 ppa:kubuntu-ppa/backports)
sudo apt install cmake extra-cmake-modules libkf6i18n-dev libkf6package-dev \
  libqt6core6-dev libqt6qml6-dev plasma-workspace-dev qt6-base-dev gettext
```

构建并安装：

```bash
./install.sh
```

### 安装后

重启 Plasma Shell 使插件生效（Plasma 6）：

```bash
kquitapp6 plasmashell 2>/dev/null; kstart6 plasmashell &
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
.
├── CMakeLists.txt              # 主构建配置
├── build.sh                    # 构建脚本
├── build-deb.sh                # 构建 deb 包
├── install.sh                  # 安装脚本
│
├── plugin/                     # C++ 后端
│   ├── CMakeLists.txt
│   ├── wallpaperbackend.h      # 后端接口定义
│   ├── wallpaperbackend.cpp    # 后端实现
│   │   - 文件夹扫描 (QDir)
│   │   - 文件监视 (QFileSystemWatcher)
│   │   - 轮播定时器 (QTimer)
│   ├── kabegamewallpaperplugin.cpp  # QML 插件注册
│   └── qmldir.in               # QML 模块声明
│
├── package/                    # QML 前端
│   ├── metadata.json           # 插件元数据
│   └── contents/
│       ├── config/main.xml     # 配置项定义
│       └── ui/
│           ├── main.qml        # 壁纸显示界面
│           └── config.qml      # 配置界面
│   └── translate/              # i18n (ja, ko, zh_CN)
│
└── debian/                     # Debian 打包
```

## 技术栈

- **C++17** - 后端逻辑
- **Qt 6** - 核心框架
- **QML** - 用户界面
- **KDE Frameworks 6 / Plasma 6** - 壁纸插件 API
- **CMake** - 构建系统

## 调试

### 查看日志

```bash
# 实时查看 Plasma Shell 日志
journalctl -f | grep -i "kabegame"

# 或者在终端启动 Plasma Shell 查看完整输出
kquitapp6 plasmashell 2>/dev/null; plasmashell 2>&1 | tee plasma.log
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
