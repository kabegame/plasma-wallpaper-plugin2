# Plasma 壁纸插件 IPC 集成指南

## 概述

Kabegame Plasma 壁纸插件可以通过 IPC（Inter-Process Communication）与 Kabegame Daemon 通信，实现：
- 从 Kabegame 获取壁纸
- 接收 Daemon 控制
- 自动同步壁纸更新
- 复用爬虫和画册系统

## 架构设计

```
┌─────────────────────────────────┐
│  Plasma 壁纸插件 (QML/JS)        │
│  - 显示壁纸                      │
│  - 过渡效果                      │
│  - 配置界面                      │
└─────────────────────────────────┘
          ↓ (D-Bus 或 Unix Socket)
┌─────────────────────────────────┐
│  IPC 桥接服务 (Rust)             │
│  - 监听 IPC 请求                 │
│  - 转发到 Daemon                 │
│  - 推送事件到插件                │
└─────────────────────────────────┘
          ↓ (Unix Socket)
┌─────────────────────────────────┐
│  Kabegame Daemon (Rust)          │
│  - 爬虫系统                      │
│  - 画册管理                      │
│  - 任务管理                      │
│  - 存储管理                      │
└─────────────────────────────────┘
```

## 方案选择

### 方案 1：D-Bus 通信（推荐）

**优点**：
- KDE/Plasma 原生支持
- QML 有内置 D-Bus 接口
- 标准化协议
- 跨进程通信成熟

**缺点**：
- 需要额外实现 D-Bus 服务
- 略微复杂

### 方案 2：Unix Socket + 桥接服务

**优点**：
- 复用现有 IPC 架构
- 代码量少
- 性能好

**缺点**：
- 需要额外的桥接进程
- QML 不能直接访问 Unix Socket

### 方案 3：文件监听（最简单）

**优点**：
- 实现简单
- 无需额外依赖
- 跨平台

**缺点**：
- 单向通信
- 实时性差
- 无法接收复杂命令

## 推荐实现：D-Bus + IPC 桥接

### 架构流程

```
1. Plasma 插件 (QML)
   ↓ D-Bus 调用
   org.kabegame.Wallpaper.getNextWallpaper()

2. D-Bus 服务 (Rust)
   ↓ IPC 请求
   IpcClient.storage_get_images()

3. Kabegame Daemon
   ↓ 返回图片列表
   Response: [image1, image2, ...]

4. D-Bus 服务
   ↓ D-Bus 信号
   wallpaperChanged(path)

5. Plasma 插件 (QML)
   ↓ 切换壁纸
   switchWallpaper(path)
```

## 实现步骤

### 步骤 1：创建 D-Bus 服务

在项目中创建一个新的 D-Bus 服务：`src-tauri/wallpaper-dbus-service`

```rust
// src-tauri/wallpaper-dbus-service/src/main.rs

use zbus::{dbus_interface, Connection, ConnectionBuilder};
use kabegame_core::cli_daemon::IpcClient;
use std::sync::Arc;
use tokio::sync::RwLock;

struct WallpaperService {
    ipc_client: Arc<IpcClient>,
    current_wallpaper: Arc<RwLock<String>>,
    current_album: Arc<RwLock<Option<String>>>,
}

#[dbus_interface(name = "org.kabegame.Wallpaper")]
impl WallpaperService {
    /// 获取当前壁纸路径
    async fn get_current_wallpaper(&self) -> String {
        self.current_wallpaper.read().await.clone()
    }
    
    /// 获取下一张壁纸
    async fn get_next_wallpaper(&self) -> Result<String, zbus::fdo::Error> {
        // 从 daemon 获取图片列表
        let images = self.ipc_client
            .storage_get_images()
            .await
            .map_err(|e| zbus::fdo::Error::Failed(e))?;
        
        // 随机选择一张
        if let Some(image) = images.choose(&mut rand::thread_rng()) {
            let path = image.local_path.clone();
            *self.current_wallpaper.write().await = path.clone();
            
            // 发送信号通知插件
            self.wallpaper_changed(&path).await?;
            
            Ok(path)
        } else {
            Err(zbus::fdo::Error::Failed("No images available".to_string()))
        }
    }
    
    /// 设置壁纸来源（画册ID）
    async fn set_source(&self, album_id: String) -> Result<(), zbus::fdo::Error> {
        *self.current_album.write().await = Some(album_id.clone());
        
        // 获取画册的第一张图片
        let images = self.ipc_client
            .storage_get_album_images(&album_id)
            .await
            .map_err(|e| zbus::fdo::Error::Failed(e))?;
        
        if let Some(image) = images.first() {
            let path = image.local_path.clone();
            *self.current_wallpaper.write().await = path.clone();
            self.wallpaper_changed(&path).await?;
        }
        
        Ok(())
    }
    
    /// 获取所有画册列表
    async fn get_albums(&self) -> Result<Vec<(String, String)>, zbus::fdo::Error> {
        let albums = self.ipc_client
            .storage_get_albums()
            .await
            .map_err(|e| zbus::fdo::Error::Failed(e))?;
        
        Ok(albums.into_iter()
            .map(|a| (a.id, a.name))
            .collect())
    }
    
    /// 信号：壁纸已更改
    #[dbus_interface(signal)]
    async fn wallpaper_changed(
        &self,
        ctxt: &zbus::SignalContext<'_>,
        path: &str,
    ) -> zbus::Result<()>;
    
    /// 信号：壁纸元数据
    #[dbus_interface(signal)]
    async fn wallpaper_metadata(
        &self,
        ctxt: &zbus::SignalContext<'_>,
        path: &str,
        metadata: &str,
    ) -> zbus::Result<()>;
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // 创建 IPC 客户端
    let ipc_client = Arc::new(IpcClient::new());
    
    // 创建服务
    let service = WallpaperService {
        ipc_client,
        current_wallpaper: Arc::new(RwLock::new(String::new())),
        current_album: Arc::new(RwLock::new(None)),
    };
    
    // 创建 D-Bus 连接
    let _conn = ConnectionBuilder::session()?
        .name("org.kabegame.Wallpaper")?
        .serve_at("/org/kabegame/Wallpaper", service)?
        .build()
        .await?;
    
    println!("Kabegame Wallpaper D-Bus service started");
    println!("Service: org.kabegame.Wallpaper");
    println!("Path: /org/kabegame/Wallpaper");
    
    // 保持运行
    std::future::pending::<()>().await;
    
    Ok(())
}
```

### 步骤 2：修改 Plasma 插件 QML

更新 `main.qml` 添加 D-Bus 支持：

```qml
// org.kabegame.wallpaper/contents/ui/main.qml

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import QtDBus 2.0

Item {
    id: root
    
    // ... 现有配置属性 ...
    
    // D-Bus 接口
    DBusInterface {
        id: kabegameWallpaper
        service: "org.kabegame.Wallpaper"
        path: "/org/kabegame/Wallpaper"
        iface: "org.kabegame.Wallpaper"
        
        signalsEnabled: true
        
        // 监听壁纸变化信号
        function onWallpaperChanged(path) {
            console.log("Received wallpaper from daemon:", path)
            switchWallpaper("file://" + path)
        }
        
        // 监听元数据信号
        function onWallpaperMetadata(path, metadata) {
            console.log("Wallpaper metadata:", metadata)
        }
    }
    
    // Kabegame Bridge 功能
    Timer {
        id: bridgeTimer
        interval: kabegameBridge ? 30000 : 0  // 30秒轮询
        running: kabegameBridge
        repeat: true
        
        onTriggered: {
            if (kabegameBridge) {
                requestNextWallpaper()
            }
        }
    }
    
    // 请求下一张壁纸
    function requestNextWallpaper() {
        console.log("Requesting next wallpaper from Kabegame daemon")
        
        var result = kabegameWallpaper.call("getNextWallpaper", [])
        if (result.error) {
            console.error("Failed to get wallpaper:", result.error)
        }
    }
    
    // 设置壁纸来源（画册）
    function setWallpaperSource(albumId) {
        console.log("Setting wallpaper source to album:", albumId)
        
        kabegameWallpaper.call("setSource", [albumId])
    }
    
    // 获取画册列表
    function loadAlbumList() {
        var result = kabegameWallpaper.call("getAlbums", [])
        if (!result.error && result.value) {
            albumList = result.value
            console.log("Loaded albums:", albumList.length)
        }
    }
    
    // ... 其余代码保持不变 ...
    
    // 初始化
    Component.onCompleted: {
        console.log("Kabegame Wallpaper Plugin loaded")
        
        if (kabegameBridge) {
            // 加载画册列表
            loadAlbumList()
            
            // 立即请求第一张壁纸
            requestNextWallpaper()
        } else {
            // 普通模式：加载本地图片
            if (configImage) {
                currentImageSource = configImage
                baseImage.source = configImage
            }
        }
    }
}
```

### 步骤 3：更新配置界面

添加画册选择功能：

```qml
// org.kabegame.wallpaper/contents/ui/config.qml

// ... 在 Kabegame Bridge 部分添加 ...

CheckBox {
    id: bridgeCheckBox
    Kirigami.FormData.label: i18n("Kabegame Integration:")
    text: i18n("Enable Kabegame Bridge")
}

// 画册选择
ComboBox {
    id: albumCombo
    Kirigami.FormData.label: i18n("Album Source:")
    Layout.fillWidth: true
    enabled: bridgeCheckBox.checked
    
    model: albumListModel
    textRole: "name"
    valueRole: "id"
    
    onCurrentValueChanged: {
        if (enabled && currentValue) {
            // 通知主界面切换画册
            // 需要通过配置传递
        }
    }
}

ListModel {
    id: albumListModel
}

// 加载画册列表
Component.onCompleted: {
    if (bridgeCheckBox.checked) {
        // 从 D-Bus 获取画册列表
        // ...
    }
}
```

### 步骤 4：创建启动脚本

```bash
#!/bin/bash
# wallpaper-dbus-service/start.sh

# 检查 daemon 是否运行
if ! pgrep -f "kabegame-daemon" > /dev/null; then
    echo "Error: Kabegame daemon is not running"
    echo "Please start the daemon first: kabegame-daemon"
    exit 1
fi

# 启动 D-Bus 服务
echo "Starting Kabegame Wallpaper D-Bus service..."
/path/to/kabegame-wallpaper-dbus-service &

echo "D-Bus service started"
echo "You can now use the Plasma wallpaper plugin with Kabegame integration"
```

### 步骤 5：systemd 用户服务（可选）

自动启动 D-Bus 服务：

```ini
# ~/.config/systemd/user/kabegame-wallpaper-dbus.service

[Unit]
Description=Kabegame Wallpaper D-Bus Service
After=kabegame-daemon.service
Requires=kabegame-daemon.service

[Service]
Type=simple
ExecStart=/usr/local/bin/kabegame-wallpaper-dbus-service
Restart=on-failure

[Install]
WantedBy=default.target
```

启用服务：

```bash
systemctl --user enable kabegame-wallpaper-dbus.service
systemctl --user start kabegame-wallpaper-dbus.service
```

## 功能扩展

### 高级功能 1：监听 Daemon 事件

```qml
// 监听任务完成事件
DBusInterface {
    id: kabegameEvents
    service: "org.kabegame.Wallpaper"
    path: "/org/kabegame/Wallpaper"
    iface: "org.kabegame.Wallpaper"
    
    signalsEnabled: true
    
    function onTaskCompleted(taskId, imageCount) {
        console.log("Task completed:", taskId, "images:", imageCount)
        // 自动切换到新图片
        requestNextWallpaper()
    }
}
```

### 高级功能 2：支持爬虫插件

```rust
// D-Bus 服务添加方法
#[dbus_interface(name = "org.kabegame.Wallpaper")]
impl WallpaperService {
    /// 运行爬虫插件
    async fn run_crawler(&self, plugin_id: String, params: HashMap<String, String>) 
        -> Result<String, zbus::fdo::Error> 
    {
        // 通过 IPC 调用 daemon 运行插件
        let task_id = self.ipc_client
            .plugin_run(&plugin_id, params)
            .await
            .map_err(|e| zbus::fdo::Error::Failed(e))?;
        
        Ok(task_id)
    }
    
    /// 获取任务状态
    async fn get_task_status(&self, task_id: String) 
        -> Result<String, zbus::fdo::Error> 
    {
        let task = self.ipc_client
            .storage_get_task(&task_id)
            .await
            .map_err(|e| zbus::fdo::Error::Failed(e))?;
        
        Ok(task.status)
    }
}
```

## 测试

### 测试 D-Bus 服务

```bash
# 1. 启动 daemon
kabegame-daemon

# 2. 启动 D-Bus 服务
kabegame-wallpaper-dbus-service

# 3. 测试调用
dbus-send --session --print-reply \
  --dest=org.kabegame.Wallpaper \
  /org/kabegame/Wallpaper \
  org.kabegame.Wallpaper.getNextWallpaper

# 4. 监听信号
dbus-monitor --session "type='signal',interface='org.kabegame.Wallpaper'"
```

### 测试插件

```bash
# 1. 安装插件
cd src-plugin
./install.sh

# 2. 重启 Plasma
kquitapp5 plasmashell && kstart5 plasmashell

# 3. 配置插件
# 右键桌面 → 配置 → 选择 Kabegame Wallpaper
# 启用 Kabegame Bridge

# 4. 查看日志
journalctl -f | grep -i "kabegame\|wallpaper"
```

## 故障排查

### 问题 1：D-Bus 服务无法启动

```bash
# 检查服务是否注册
dbus-send --session --print-reply \
  --dest=org.freedesktop.DBus \
  /org/freedesktop/DBus \
  org.freedesktop.DBus.ListNames | grep kabegame
```

### 问题 2：插件无法连接

```bash
# 检查 D-Bus 权限
qdbus org.kabegame.Wallpaper /org/kabegame/Wallpaper
```

### 问题 3：Daemon 未运行

```bash
# 检查 daemon
pgrep -f kabegame-daemon

# 查看 daemon 日志
journalctl -f -t kabegame-daemon
```

## 性能优化

1. **缓存图片列表**：避免频繁查询 daemon
2. **批量获取**：一次获取多张图片路径
3. **异步调用**：使用异步 D-Bus 调用
4. **信号优化**：减少不必要的信号发送

## 安全考虑

1. **权限控制**：D-Bus 服务只监听会话总线
2. **输入验证**：验证所有来自插件的输入
3. **错误处理**：优雅处理 daemon 崩溃
4. **资源限制**：限制并发请求数量

## 总结

通过 D-Bus + IPC 桥接，Plasma 壁纸插件可以：

- ✅ 从 Kabegame 获取壁纸
- ✅ 接收 daemon 控制
- ✅ 自动同步更新
- ✅ 复用爬虫系统
- ✅ 支持画册轮播
- ✅ 监听任务事件

这是一个完整、可扩展、性能优秀的解决方案！
