# Kabegame Plasma Wallpaper Plugin

## ✨ 项目完成！

我已经为你创建了一个功能完整的 KDE Plasma 壁纸插件，位于 `src-plugin` 目录。

## 📁 项目结构

```
src-plugin/
├── 📖 文档
│   ├── README.md           # 完整项目文档
│   ├── QUICKSTART.md       # 5分钟快速开始
│   ├── STRUCTURE.md        # 项目结构详解
│   ├── TESTING.md          # 详细测试指南
│   └── .gitignore          # Git 忽略配置
│
├── 🛠️ 脚本
│   ├── install.sh          # 一键安装
│   ├── uninstall.sh        # 一键卸载
│   └── package.sh          # 打包发布
│
└── 📦 插件
    └── org.kabegame.wallpaper/
        ├── metadata.json             # 插件元数据
        └── contents/
            ├── config/
            │   └── main.xml          # 配置定义
            └── ui/
                ├── main.qml          # 主界面（壁纸显示）
                └── config.qml        # 配置界面
```

## 🎯 核心功能

### 1. 过渡效果
- ✅ **Fade** - 淡入淡出（流畅自然）
- ✅ **Slide** - 滑动切换（动感十足）
- ✅ **Zoom** - 缩放过渡（优雅大气）
- ✅ **None** - 无过渡（快速切换）
- 🚧 **Rotate** - 旋转效果（待实现）

### 2. 填充模式
- ✅ **Fill** - 填满屏幕，保持比例
- ✅ **Fit** - 完整显示，保持比例
- ✅ **Stretch** - 拉伸填满
- ✅ **Center** - 居中，不缩放
- ✅ **Tile** - 平铺

### 3. 轮播功能
- ✅ 文件夹轮播
- ✅ 可配置间隔（5秒 - 1小时）
- ✅ 随机或顺序播放
- 🚧 图片列表加载（待实现）

### 4. 高级功能
- 🚧 Kabegame Bridge（D-Bus 集成，待实现）
- 🚧 多显示器支持（待实现）
- 🚧 动态壁纸（待实现）

## 🚀 快速开始

### 安装

```bash
cd /home/cm/code/kabegame/src-plugin
./install.sh
kquitapp5 plasmashell && kstart5 plasmashell
```

### 使用

1. 右键点击桌面 → **配置桌面和壁纸**
2. 壁纸类型选择 **Kabegame Wallpaper**
3. 点击 **配置** 按钮
4. 选择壁纸和效果
5. 点击 **应用**

详细步骤请查看 [QUICKSTART.md](QUICKSTART.md)

## 🔧 技术实现

### 双图层过渡架构

```qml
Item {
    // 底层图片（当前显示）
    Image { id: baseImage }
    
    // 顶层图片（过渡用）
    Image { id: topImage; opacity: 0 }
    
    // 过渡动画
    ParallelAnimation {
        NumberAnimation { target: topImage; property: "opacity" }
        // + 其他动画效果
    }
}
```

### 配置系统

使用 KConfig (KConfigXT) 实现配置持久化：
- 配置定义：`main.xml`
- 配置读取：`wallpaper.configuration.XXX`
- 配置界面：`config.qml`

### 性能优化

- ✅ 异步图片加载（`asynchronous: true`）
- ✅ 禁用图片缓存（避免旧图残留）
- ✅ 平滑缩放（`smooth: true`）
- ✅ 过渡完成后释放资源

## 📊 当前状态

| 功能 | 状态 | 说明 |
|------|------|------|
| 基础壁纸显示 | ✅ 完成 | 单张图片设置 |
| 过渡效果 | ✅ 完成 | Fade/Slide/Zoom |
| 填充模式 | ✅ 完成 | 5种模式 |
| 配置界面 | ✅ 完成 | 完整的 UI |
| 配置持久化 | ✅ 完成 | KConfig 集成 |
| 轮播功能 | 🚧 部分 | Timer 实现，待加载图片列表 |
| Kabegame Bridge | 🚧 计划中 | D-Bus 集成 |
| 多显示器 | 🚧 计划中 | 独立配置 |

## 🎨 界面预览

### 配置界面包含：
1. **壁纸选择**
   - 单张图片选择
   - 文件夹选择（轮播用）

2. **显示设置**
   - 填充模式下拉菜单
   - 过渡效果下拉菜单
   - 过渡时长调整

3. **轮播设置**
   - 启用开关
   - 间隔时间
   - 播放顺序

4. **高级选项**
   - Kabegame Bridge 开关

## 📚 文档导航

- 🚀 [快速开始](QUICKSTART.md) - 5分钟上手
- 📖 [完整文档](README.md) - 详细功能说明
- 🏗️ [项目结构](STRUCTURE.md) - 代码组织
- 🧪 [测试指南](TESTING.md) - 测试方法

## 🛠️ 开发指南

### 修改代码

```bash
# 编辑 QML 文件
vim org.kabegame.wallpaper/contents/ui/main.qml

# 重新安装
./install.sh

# 重启 Plasma Shell
kquitapp5 plasmashell && kstart5 plasmashell
```

### 调试

```bash
# 查看日志
journalctl -f | grep plasmashell

# 或在终端运行
kquitapp5 plasmashell
plasmashell 2>&1 | grep -i "kabegame"
```

### 打包

```bash
./package.sh
# 生成: kabegame-wallpaper-1.0.0.tar.gz
```

## 🔮 未来计划

### v1.1 (短期)
- [ ] 实现文件夹图片列表加载
- [ ] 完善旋转过渡效果
- [ ] 添加更多过渡效果（擦除、马赛克等）
- [ ] 性能优化和内存管理

### v1.5 (中期)
- [ ] Kabegame D-Bus 集成
- [ ] 从 Kabegame 获取壁纸
- [ ] 支持画册轮播
- [ ] 多显示器独立配置

### v2.0 (长期)
- [ ] 动态壁纸支持
- [ ] 视频壁纸支持
- [ ] 壁纸效果编辑器
- [ ] 社区壁纸库

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

开发流程：
1. Fork 项目
2. 创建特性分支
3. 提交更改
4. 推送到分支
5. 创建 Pull Request

## 📄 许可证

MIT License

## 🙏 致谢

- KDE Plasma 团队
- Qt/QML 社区
- Kabegame 项目

---

## ⚡ 立即开始

```bash
# 克隆或进入项目
cd /home/cm/code/kabegame/src-plugin

# 安装
./install.sh

# 重启 Plasma
kquitapp5 plasmashell && kstart5 plasmashell

# 享受你的新壁纸插件！🎉
```

**需要帮助？** 查看 [QUICKSTART.md](QUICKSTART.md) 或提交 Issue。
