# Kabegame Plasma Wallpaper Plugin - 项目结构

## 目录结构

```
src-plugin/
├── README.md                  # 项目说明文档
├── TESTING.md                 # 测试指南
├── .gitignore                 # Git 忽略文件
├── install.sh                 # 安装脚本
├── uninstall.sh               # 卸载脚本
├── package.sh                 # 打包脚本
│
└── org.kabegame.wallpaper/    # 插件主目录
    ├── metadata.json          # 插件元数据
    │
    └── contents/
        ├── config/
        │   └── main.xml       # 配置定义 (KConfigXT)
        │
        └── ui/
            ├── main.qml       # 主界面 (壁纸显示)
            └── config.qml     # 配置界面
```

## 文件说明

### 根目录文件

| 文件 | 说明 |
|------|------|
| `README.md` | 项目文档，包含安装、使用、开发指南 |
| `TESTING.md` | 详细的测试指南和调试方法 |
| `.gitignore` | Git 忽略文件配置 |
| `install.sh` | 一键安装脚本 |
| `uninstall.sh` | 一键卸载脚本 |
| `package.sh` | 打包为 `.tar.gz` 的脚本 |

### 插件文件

| 文件 | 说明 | 作用 |
|------|------|------|
| `metadata.json` | 插件元数据 | 定义插件 ID、名称、版本等信息 |
| `contents/config/main.xml` | 配置定义 | 定义所有配置项及其默认值 |
| `contents/ui/main.qml` | 主界面 | 壁纸显示和过渡效果实现 |
| `contents/ui/config.qml` | 配置界面 | 用户配置界面 |

## 配置项说明

### main.xml 定义的配置项

| 配置项 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| `Image` | String | "" | 单张壁纸路径 |
| `ImageFolder` | String | "" | 轮播文件夹路径 |
| `FillMode` | String | "fill" | 填充模式 |
| `Transition` | String | "fade" | 过渡效果 |
| `TransitionDuration` | Int | 500 | 过渡时长（ms） |
| `SlideshowEnabled` | Bool | false | 是否启用轮播 |
| `SlideshowInterval` | Int | 60 | 轮播间隔（秒） |
| `SlideshowOrder` | String | "random" | 轮播顺序 |
| `KabegameBridgeEnabled` | Bool | false | Kabegame 集成 |

## 主要功能实现

### 1. 过渡效果 (main.qml)

实现文件：`contents/ui/main.qml`

- **双图层架构**：使用 `baseImage` 和 `topImage` 实现无闪烁过渡
- **动画系统**：使用 `ParallelAnimation` 组合多个动画效果
- **效果类型**：
  - Fade: 透明度动画
  - Slide: 位置动画 (Transform)
  - Zoom: 缩放动画
  - Rotate: 旋转动画（待实现）

### 2. 填充模式 (main.qml)

通过 QML Image 组件的 `fillMode` 属性实现：

- `Image.PreserveAspectCrop` → fill
- `Image.PreserveAspectFit` → fit
- `Image.Stretch` → stretch
- `Image.Pad` → center
- `Image.Tile` → tile

### 3. 配置界面 (config.qml)

使用 Kirigami 组件库构建：

- `Kirigami.FormLayout`：表单布局
- `ComboBox`：下拉选择
- `SpinBox`：数值输入
- `CheckBox`：开关选项
- `FileDialog`：文件选择

### 4. 轮播功能 (main.qml)

使用 QML Timer 实现定时切换：

- 读取文件夹中的图片列表
- 根据配置的顺序（随机/顺序）选择下一张
- 通过 Timer 定时触发切换

## 技术要点

### QML 组件

- **Image**: 图片显示
- **ParallelAnimation**: 并行动画
- **NumberAnimation**: 数值动画
- **Timer**: 定时器
- **Connections**: 信号连接

### Plasma Framework

- **wallpaper.configuration**: 配置访问接口
- **Plasma/Wallpaper**: 服务类型
- **KConfig**: 配置持久化

### 性能优化

- `asynchronous: true`: 异步加载图片
- `cache: false`: 禁用缓存避免旧图残留
- `smooth: true`: 平滑缩放
- 过渡完成后释放顶层图片资源

## 开发工作流

### 1. 修改代码

编辑 QML 文件：
- `main.qml`：修改壁纸显示逻辑
- `config.qml`：修改配置界面
- `main.xml`：添加新配置项

### 2. 测试

```bash
# 重新安装
./install.sh

# 重启 Plasma Shell
kquitapp5 plasmashell && kstart5 plasmashell

# 或者查看日志
journalctl -f | grep plasmashell
```

### 3. 调试

在 QML 中使用：
```qml
console.log("Debug info:", value)
```

### 4. 发布

```bash
# 打包
./package.sh

# 生成 kabegame-wallpaper-1.0.0.tar.gz
```

## 扩展计划

### 短期 (v1.1)

- [ ] 实现文件夹图片列表加载
- [ ] 完善旋转过渡效果
- [ ] 添加更多过渡效果
- [ ] 性能优化

### 中期 (v1.5)

- [ ] Kabegame D-Bus 集成
- [ ] 支持动态壁纸
- [ ] 支持视频壁纸
- [ ] 多显示器独立配置

### 长期 (v2.0)

- [ ] 壁纸效果编辑器
- [ ] 社区壁纸库
- [ ] AI 壁纸生成
- [ ] 主题系统

## 相关资源

- [KDE Plasma 开发文档](https://develop.kde.org/docs/plasma/)
- [QML 参考文档](https://doc.qt.io/qt-6/qmlapplications.html)
- [Kirigami 组件库](https://develop.kde.org/frameworks/kirigami/)
- [KConfig 文档](https://api.kde.org/frameworks/kconfig/html/index.html)

## 贡献指南

1. Fork 项目
2. 创建特性分支
3. 提交更改
4. 推送到分支
5. 创建 Pull Request

## 许可证

MIT License - 详见 LICENSE 文件
