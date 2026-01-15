# 下一步开发计划

## 🎯 当前状态

已完成的 **MVP（最小可用产品）**：
- ✅ 基础壁纸显示
- ✅ 3种过渡效果（Fade, Slide, Zoom）
- ✅ 5种填充模式
- ✅ 配置界面
- ✅ 配置持久化
- ✅ 安装/卸载脚本
- ✅ 完整文档

## 🚀 立即可以做的事

### 1. 测试插件

```bash
cd /home/cm/code/kabegame/src-plugin
./install.sh
kquitapp5 plasmashell && kstart5 plasmashell
```

然后按照 [QUICKSTART.md](QUICKSTART.md) 进行测试。

### 2. 完善文件夹轮播功能

**需要实现**：加载文件夹中的图片列表

**位置**：`org.kabegame.wallpaper/contents/ui/main.qml`

**代码位置**：
```qml
// 第 237 行左右
function loadImageFolder(folderPath) {
    // TODO: 使用 FolderListModel 或文件对话框加载图片列表
}
```

**建议实现方式**：

```qml
import Qt.labs.folderlistmodel 2.15

FolderListModel {
    id: folderModel
    folder: configFolder
    nameFilters: ["*.jpg", "*.jpeg", "*.png", "*.bmp", "*.gif", "*.webp"]
    showDirs: false
    
    onCountChanged: {
        imageList = []
        for (var i = 0; i < count; i++) {
            imageList.push(folderModel.get(i, "fileURL"))
        }
        if (imageList.length > 0 && slideshowEnabled) {
            nextWallpaper()
        }
    }
}
```

### 3. 实现旋转过渡效果

**位置**：`org.kabegame.wallpaper/contents/ui/main.qml`

**需要添加**：

```qml
// 在 topImage 中添加旋转变换
rotation: 0

// 在 prepareTransition 函数的 switch 中添加
case "rotate":
    slideAnimation.enabled = false
    zoomAnimation.enabled = false
    // 添加旋转动画
    topImage.rotation = 180
    // 需要创建新的 NumberAnimation
    break
```

### 4. 优化性能

**可以优化的地方**：

1. **图片预加载**
```qml
// 预加载下一张图片
Image {
    id: preloadImage
    source: getNextImageSource()
    visible: false
    asynchronous: true
}
```

2. **内存管理**
```qml
// 释放不用的图片
onFinished: {
    baseImage.source = topImage.source
    topImage.source = ""  // 释放资源
}
```

3. **缓存策略**
```qml
// 对小图片启用缓存
cache: sourceSize.width < 1920
```

## 🔮 后续功能开发

### 短期（1-2周）

#### 1. 完善轮播功能
- [ ] 实现文件夹图片列表加载
- [ ] 添加播放/暂停控制
- [ ] 添加上一张/下一张手动控制

#### 2. 添加更多过渡效果
- [ ] 擦除效果（Wipe）
- [ ] 百叶窗效果（Blinds）
- [ ] 马赛克效果（Mosaic）
- [ ] 溶解效果（Dissolve）

#### 3. 增强配置界面
- [ ] 添加预览功能
- [ ] 添加过渡效果预览
- [ ] 添加更多配置选项

### 中期（1-2月）

#### 1. Kabegame Bridge 集成

**架构设计**：

```
Plasma 插件 (QML)
    ↓ (D-Bus)
Kabegame 守护进程 (Rust)
    ↓
Kabegame 核心服务
    ↓
爬虫系统/画册系统
```

**需要实现**：

1. **Rust 端** - D-Bus 服务
```rust
// src-tauri/core/src/dbus/wallpaper_service.rs

use zbus::{dbus_interface, ConnectionBuilder};

struct WallpaperService {
    // 壁纸服务实现
}

#[dbus_interface(name = "org.kabegame.Wallpaper")]
impl WallpaperService {
    async fn get_current_wallpaper(&self) -> String {
        // 返回当前壁纸路径
    }
    
    async fn next_wallpaper(&self) -> String {
        // 切换到下一张
    }
    
    async fn set_source(&self, source_type: String, params: HashMap<String, String>) {
        // 设置壁纸来源
    }
}
```

2. **QML 端** - D-Bus 客户端
```qml
import QtDBus 2.0

DBusInterface {
    id: kabegameBridge
    service: "org.kabegame.Wallpaper"
    path: "/org/kabegame/Wallpaper"
    interface: "org.kabegame.Wallpaper"
    
    function getCurrentWallpaper() {
        return call("getCurrentWallpaper")
    }
    
    function nextWallpaper() {
        call("nextWallpaper")
    }
}

Connections {
    target: kabegameBridge
    function onWallpaperChanged(path) {
        switchWallpaper(path)
    }
}
```

#### 2. 多显示器支持

**挑战**：每个显示器可能是不同的 Containment

**解决方案**：
- 检测当前 Containment ID
- 为每个 Containment 独立配置
- 或提供"所有显示器相同"选项

#### 3. 动态壁纸

**实现思路**：
- 使用 AnimatedImage 或 Video
- 支持 GIF/WEBP 动画
- 支持 MP4 视频作为壁纸

### 长期（3-6月）

#### 1. 壁纸效果编辑器

**功能**：
- 可视化编辑过渡效果
- 自定义动画曲线
- 实时预览

#### 2. 社区壁纸库

**功能**：
- 在线浏览壁纸
- 一键下载和应用
- 评分和收藏

#### 3. AI 功能

**功能**：
- AI 生成壁纸
- 智能推荐
- 自动分类

## 🛠️ 开发环境搭建

### 依赖

```bash
# KDE 开发环境
sudo apt install kde-sdk-dev plasma-sdk

# QML 开发工具
sudo apt install qml-module-qtquick-* qml-module-org-kde-*

# 调试工具
sudo apt install qml qmlscene
```

### IDE 配置

**推荐使用**：
- Qt Creator
- KDevelop
- VS Code + QML 插件

### 调试技巧

```bash
# 1. QML 语法检查
qmllint contents/ui/main.qml

# 2. 独立测试 QML
qmlscene contents/ui/main.qml

# 3. 查看 Plasma Shell 日志
journalctl -f -t plasmashell

# 4. 启用 QML 调试
QML_IMPORT_TRACE=1 plasmashell --replace
```

## 📚 学习资源

### KDE Plasma 开发
- [官方文档](https://develop.kde.org/docs/plasma/)
- [Plasma API 参考](https://api.kde.org/frameworks/)
- [示例代码](https://invent.kde.org/plasma/)

### QML 开发
- [Qt QML 文档](https://doc.qt.io/qt-6/qmlapplications.html)
- [QML 教程](https://doc.qt.io/qt-6/qml-tutorial.html)
- [Qt Quick 控件](https://doc.qt.io/qt-6/qtquickcontrols-index.html)

### D-Bus 集成
- [zbus 文档](https://docs.rs/zbus/)
- [D-Bus 规范](https://dbus.freedesktop.org/doc/)

## 🤝 贡献方式

### 1. 报告 Bug
- 使用 GitHub Issues
- 提供详细的错误信息
- 附上日志和截图

### 2. 提交功能请求
- 描述功能需求
- 说明使用场景
- 提供设计建议

### 3. 提交代码
- Fork 项目
- 创建特性分支
- 编写测试
- 提交 PR

## 📊 项目路线图

```
v1.0 (当前) ────────────────────────────────────
│  ✅ 基础功能
│  ✅ 过渡效果
│  ✅ 配置界面
│
v1.1 (2周) ─────────────────────────────────────
│  🚧 文件夹轮播
│  🚧 更多过渡效果
│  🚧 性能优化
│
v1.5 (2月) ─────────────────────────────────────
│  🔮 Kabegame Bridge
│  🔮 多显示器支持
│  🔮 动态壁纸
│
v2.0 (6月) ─────────────────────────────────────
   🔮 效果编辑器
   🔮 社区壁纸库
   🔮 AI 功能
```

## 🎯 当前优先级

**高优先级**：
1. 完善文件夹轮播功能
2. 添加更多过渡效果
3. 性能优化

**中优先级**：
4. Kabegame Bridge 设计
5. 多显示器支持
6. 增强配置界面

**低优先级**：
7. 动态壁纸
8. 高级效果
9. 社区功能

---

**准备好开始了吗？** 从测试插件开始，然后选择一个功能进行开发！🚀
