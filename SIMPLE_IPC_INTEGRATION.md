# Plasma 插件简易 IPC 集成方案

## 快速开始

如果你觉得完整的 D-Bus 方案太复杂，这里提供一个**超简单**的文件通信方案。

## 方案：JSON 文件通信

### 工作原理

```
Daemon → 写入文件 → /tmp/kabegame-wallpaper.json
                          ↓
                    QML 监听文件变化
                          ↓
                    读取并切换壁纸
```

### 优点

- ✅ 实现简单（10分钟搞定）
- ✅ 无需额外依赖
- ✅ 容易调试
- ✅ 跨平台

### 缺点

- ❌ 单向通信（daemon → 插件）
- ❌ 轮询延迟（1-5秒）
- ❌ 无法反馈状态

## 实现步骤

### 步骤 1：Daemon 写入文件

在你的 Rhai 脚本或 Rust 代码中：

```rust
// src-tauri/core/src/wallpaper_sync.rs

use std::fs;
use serde_json::json;

pub fn sync_wallpaper_to_plasma(image_path: &str) -> Result<(), String> {
    let data = json!({
        "wallpaper": image_path,
        "timestamp": chrono::Utc::now().timestamp(),
        "transition": "fade",
        "duration": 500
    });
    
    let path = "/tmp/kabegame-wallpaper.json";
    fs::write(path, data.to_string())
        .map_err(|e| format!("Failed to write wallpaper file: {}", e))?;
    
    println!("✅ Synced wallpaper to Plasma: {}", image_path);
    Ok(())
}
```

### 步骤 2：插件监听文件

修改 `main.qml`：

```qml
import QtQuick 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import Qt.labs.platform 1.1

Item {
    id: root
    
    // ... 现有代码 ...
    
    // 文件监听器
    FileSystemWatcher {
        id: wallpaperWatcher
        files: ["/tmp/kabegame-wallpaper.json"]
        
        onFileChanged: function(path) {
            console.log("Wallpaper file changed:", path)
            loadWallpaperFromFile()
        }
    }
    
    // 加载壁纸函数
    function loadWallpaperFromFile() {
        var xhr = new XMLHttpRequest()
        xhr.open("GET", "file:///tmp/kabegame-wallpaper.json")
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    try {
                        var data = JSON.parse(xhr.responseText)
                        console.log("Received wallpaper:", data.wallpaper)
                        
                        // 切换壁纸
                        setWallpaper(data.wallpaper)
                    } catch (e) {
                        console.error("Failed to parse wallpaper JSON:", e)
                    }
                }
            }
        }
        xhr.send()
    }
    
    // 定时轮询（备用）
    Timer {
        id: pollTimer
        interval: 5000  // 5秒
        running: kabegameBridge
        repeat: true
        onTriggered: loadWallpaperFromFile()
    }
    
    Component.onCompleted: {
        if (kabegameBridge) {
            // 首次加载
            loadWallpaperFromFile()
        }
    }
}
```

### 步骤 3：在任务完成时触发

在 Rhai 脚本中：

```rust
// crawler-plugins/plugins/konachan/main.rhai

// 任务完成后
task.on_complete(|| {
    // 获取最新图片
    let images = storage.get_images_by_task(task.id());
    if images.len() > 0 {
        let latest = images[0];
        
        // 同步到 Plasma
        wallpaper.sync_to_plasma(latest.local_path);
    }
});
```

## 完整示例

### 1. 创建 Rust 模块

```rust
// src-tauri/core/src/plasma_sync.rs

use std::fs;
use std::path::Path;
use serde_json::json;
use chrono::Utc;

pub struct PlasmaSync {
    file_path: String,
}

impl PlasmaSync {
    pub fn new() -> Self {
        Self {
            file_path: "/tmp/kabegame-wallpaper.json".to_string(),
        }
    }
    
    /// 同步壁纸到 Plasma
    pub fn sync(&self, image_path: &str) -> Result<(), String> {
        // 检查文件是否存在
        if !Path::new(image_path).exists() {
            return Err(format!("Image not found: {}", image_path));
        }
        
        let data = json!({
            "wallpaper": image_path,
            "timestamp": Utc::now().timestamp(),
            "transition": "fade",
            "duration": 500,
            "metadata": {
                "source": "kabegame"
            }
        });
        
        fs::write(&self.file_path, serde_json::to_string_pretty(&data).unwrap())
            .map_err(|e| format!("Failed to write: {}", e))?;
        
        eprintln!("✅ Synced wallpaper to Plasma plugin: {}", image_path);
        Ok(())
    }
    
    /// 批量同步（用于幻灯片）
    pub fn sync_batch(&self, image_paths: Vec<String>) -> Result<(), String> {
        let data = json!({
            "wallpapers": image_paths,
            "timestamp": Utc::now().timestamp(),
            "mode": "slideshow",
            "interval": 60
        });
        
        fs::write(&self.file_path, serde_json::to_string_pretty(&data).unwrap())
            .map_err(|e| format!("Failed to write: {}", e))?;
        
        eprintln!("✅ Synced {} wallpapers to Plasma", image_paths.len());
        Ok(())
    }
    
    /// 清除壁纸
    pub fn clear(&self) -> Result<(), String> {
        let data = json!({
            "wallpaper": null,
            "timestamp": Utc::now().timestamp(),
            "action": "clear"
        });
        
        fs::write(&self.file_path, serde_json::to_string_pretty(&data).unwrap())
            .map_err(|e| format!("Failed to write: {}", e))?;
        
        Ok(())
    }
}
```

### 2. 集成到 Rhai

```rust
// src-tauri/core/src/plugin/rhai.rs

// 在注册 API 时添加
engine.register_type_with_name::<Arc<PlasmaSync>>("PlasmaSync")
    .register_fn("sync", |ps: &mut Arc<PlasmaSync>, path: &str| {
        ps.sync(path)
    })
    .register_fn("sync_batch", |ps: &mut Arc<PlasmaSync>, paths: Vec<String>| {
        ps.sync_batch(paths)
    })
    .register_fn("clear", |ps: &mut Arc<PlasmaSync>| {
        ps.clear()
    });

// 在 scope 中添加实例
let plasma_sync = Arc::new(PlasmaSync::new());
scope.push("plasma", plasma_sync);
```

### 3. Rhai 脚本使用

```rust
// crawler-plugins/plugins/konachan/main.rhai

// 下载完成后
fn on_download_complete(image_path) {
    // 同步到 Plasma
    plasma.sync(image_path);
}

// 任务完成后
fn on_task_complete() {
    let images = storage.get_recent_images(10);
    let paths = images.map(|img| img.local_path);
    
    // 批量同步
    plasma.sync_batch(paths);
}
```

### 4. 完整的 QML 实现

```qml
// org.kabegame.wallpaper/contents/ui/main.qml

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import Qt.labs.platform 1.1

Item {
    id: root
    
    // 配置
    readonly property bool kabegameBridge: wallpaper.configuration.KabegameBridgeEnabled || false
    readonly property string syncFile: "/tmp/kabegame-wallpaper.json"
    
    // 当前壁纸
    property string currentWallpaper: ""
    property var wallpaperQueue: []
    property int queueIndex: 0
    
    // 背景图片
    Image {
        id: baseImage
        anchors.fill: parent
        source: currentWallpaper
        fillMode: Image.PreserveAspectCrop
        smooth: true
    }
    
    Image {
        id: topImage
        anchors.fill: parent
        opacity: 0.0
        fillMode: Image.PreserveAspectCrop
        smooth: true
    }
    
    // 淡入动画
    SequentialAnimation {
        id: fadeAnimation
        PropertyAnimation {
            target: topImage
            property: "opacity"
            from: 0.0
            to: 1.0
            duration: 500
        }
        ScriptAction {
            script: {
                currentWallpaper = topImage.source
                baseImage.source = currentWallpaper
                topImage.source = ""
                topImage.opacity = 0.0
            }
        }
    }
    
    // 文件监听
    FileSystemWatcher {
        id: wallpaperWatcher
        files: [syncFile]
        
        onFileChanged: function(path) {
            console.log("🔄 Kabegame wallpaper file changed")
            loadWallpaperFromFile()
        }
    }
    
    // 加载壁纸
    function loadWallpaperFromFile() {
        var xhr = new XMLHttpRequest()
        xhr.open("GET", "file://" + syncFile)
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE && xhr.status === 200) {
                try {
                    var data = JSON.parse(xhr.responseText)
                    
                    if (data.wallpaper) {
                        // 单张壁纸
                        console.log("📷 New wallpaper:", data.wallpaper)
                        switchWallpaper("file://" + data.wallpaper)
                    } else if (data.wallpapers && data.wallpapers.length > 0) {
                        // 幻灯片模式
                        console.log("📷 Slideshow:", data.wallpapers.length, "images")
                        wallpaperQueue = data.wallpapers
                        queueIndex = 0
                        switchWallpaper("file://" + wallpaperQueue[0])
                        
                        // 启动幻灯片定时器
                        slideshowTimer.interval = (data.interval || 60) * 1000
                        slideshowTimer.start()
                    } else if (data.action === "clear") {
                        console.log("🧹 Clear wallpaper")
                        currentWallpaper = ""
                    }
                } catch (e) {
                    console.error("❌ Failed to parse:", e)
                }
            }
        }
        xhr.send()
    }
    
    // 切换壁纸
    function switchWallpaper(path) {
        if (!path || path === currentWallpaper) return
        
        console.log("✅ Switching to:", path)
        topImage.source = path
        fadeAnimation.start()
    }
    
    // 幻灯片定时器
    Timer {
        id: slideshowTimer
        repeat: true
        onTriggered: {
            if (wallpaperQueue.length > 0) {
                queueIndex = (queueIndex + 1) % wallpaperQueue.length
                switchWallpaper("file://" + wallpaperQueue[queueIndex])
            }
        }
    }
    
    // 轮询定时器（备用）
    Timer {
        id: pollTimer
        interval: 5000
        running: kabegameBridge
        repeat: true
        onTriggered: loadWallpaperFromFile()
    }
    
    // 初始化
    Component.onCompleted: {
        console.log("🚀 Kabegame Wallpaper Plugin loaded")
        console.log("Bridge enabled:", kabegameBridge)
        
        if (kabegameBridge) {
            loadWallpaperFromFile()
        }
    }
}
```

## 测试

### 1. 手动测试

```bash
# 写入测试数据
echo '{
  "wallpaper": "/home/user/Pictures/test.jpg",
  "timestamp": 1234567890,
  "transition": "fade",
  "duration": 500
}' > /tmp/kabegame-wallpaper.json

# 观察插件是否切换壁纸
```

### 2. 从 Daemon 测试

```rust
// 在 daemon 中调用
use kabegame_core::plasma_sync::PlasmaSync;

let sync = PlasmaSync::new();
sync.sync("/path/to/wallpaper.jpg")?;
```

### 3. 从 CLI 测试

```bash
# 添加 CLI 命令
kabegame-cli plasma sync /path/to/image.jpg
```

## 进阶功能

### 支持元数据

```json
{
  "wallpaper": "/path/to/image.jpg",
  "timestamp": 1234567890,
  "metadata": {
    "title": "Beautiful Sunset",
    "author": "John Doe",
    "source": "konachan",
    "tags": ["sunset", "nature"],
    "resolution": "1920x1080"
  }
}
```

QML 显示元数据：

```qml
Text {
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    text: wallpaperMetadata.title + " by " + wallpaperMetadata.author
    color: "white"
    style: Text.Outline
    styleColor: "black"
}
```

### 支持命令

```json
{
  "action": "next",
  "timestamp": 1234567890
}
```

```qml
if (data.action === "next") {
    // 切换到下一张
} else if (data.action === "prev") {
    // 切换到上一张
} else if (data.action === "pause") {
    // 暂停幻灯片
}
```

## 性能优化

1. **减少写入频率**：避免每秒更新多次
2. **批量更新**：一次传递多张图片
3. **轮询间隔**：根据需求调整（1-10秒）
4. **文件锁**：避免读写冲突

## 故障排查

### 问题：插件未响应

```bash
# 1. 检查文件是否存在
ls -lh /tmp/kabegame-wallpaper.json

# 2. 查看文件内容
cat /tmp/kabegame-wallpaper.json

# 3. 检查文件权限
chmod 666 /tmp/kabegame-wallpaper.json

# 4. 查看 Plasma 日志
journalctl -f | grep -i wallpaper
```

### 问题：文件监听不工作

使用轮询模式作为后备：

```qml
Timer {
    interval: 2000
    running: true
    repeat: true
    onTriggered: loadWallpaperFromFile()
}
```

## 与 D-Bus 方案对比

| 特性 | 文件通信 | D-Bus |
|-----|---------|-------|
| 实现难度 | ⭐ 简单 | ⭐⭐⭐ 复杂 |
| 实时性 | ⭐⭐ 1-5秒延迟 | ⭐⭐⭐ 即时 |
| 双向通信 | ❌ 否 | ✅ 是 |
| 事件监听 | ❌ 否 | ✅ 是 |
| 资源占用 | ⭐⭐⭐ 低 | ⭐⭐ 中 |
| 可靠性 | ⭐⭐ 中 | ⭐⭐⭐ 高 |

## 总结

**文件通信方案**适合：
- ✅ 快速原型
- ✅ 简单场景
- ✅ 单向通信
- ✅ 不需要高实时性

如果你需要更复杂的功能（如双向通信、事件监听、插件反馈），请参考 `IPC_INTEGRATION.md` 的完整 D-Bus 方案。

但对于**大多数场景**，这个简单方案已经足够了！ 🎉
