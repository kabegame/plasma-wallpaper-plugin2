# 快速开始 - Kabegame Plasma Wallpaper Plugin

## 5分钟快速安装和使用

### 第 1 步：安装插件

```bash
cd /home/cm/code/kabegame/src-plugin
./install.sh
```

### 第 2 步：重启 Plasma Shell

```bash
kquitapp5 plasmashell && kstart5 plasmashell
```

或者按 `Alt + F2`，输入 `plasmashell --replace`，然后按回车。

### 第 3 步：配置壁纸

1. 右键点击桌面
2. 选择 **配置桌面和壁纸**
3. 在 **壁纸类型** 下拉菜单中选择 **Kabegame Wallpaper**
4. 点击 **配置** 按钮

### 第 4 步：选择壁纸

1. 点击 **Wallpaper Image** 旁边的 **Browse...** 按钮
2. 选择一张图片
3. 选择填充模式（推荐：Fill）
4. 选择过渡效果（推荐：Fade）
5. 点击 **应用**

## 测试过渡效果

准备 2-3 张不同的图片，然后：

1. 设置第一张图片
2. 等待几秒钟
3. 设置第二张图片
4. 观察过渡效果

尝试不同的过渡效果：
- **Fade**：淡入淡出（推荐）
- **Slide**：滑动切换（酷炫）
- **Zoom**：缩放过渡（动感）

## 启用轮播

1. 点击 **Slideshow Folder** 旁边的 **Browse...** 按钮
2. 选择一个包含多张图片的文件夹
3. 勾选 **Enable slideshow**
4. 设置轮播间隔（例如：10 秒）
5. 选择轮播顺序（Random 或 Sequential）
6. 点击 **应用**

## 调整填充模式

尝试不同的填充模式，找到最适合你的：

- **Fill**：保持比例，填满屏幕（可能裁剪边缘）
- **Fit**：保持比例，完整显示（可能有黑边）
- **Stretch**：拉伸填满（可能变形）
- **Center**：居中显示，不缩放
- **Tile**：平铺（适合小图案）

## 调整过渡时长

在配置中调整 **Transition Duration**：

- **快速**：200-300ms
- **标准**：500ms（默认）
- **慢速**：1000-2000ms

## 卸载插件

如果不想使用了：

```bash
cd /home/cm/code/kabegame/src-plugin
./uninstall.sh
kquitapp5 plasmashell && kstart5 plasmashell
```

## 常见问题

**Q: 插件安装后找不到？**  
A: 重启 Plasma Shell 或重新登录。

**Q: 图片不显示？**  
A: 检查图片路径是否正确，文件是否有读取权限。

**Q: 过渡效果看不到？**  
A: 确保选择了过渡效果（不是 None），并且过渡时长大于 0。

**Q: 轮播不工作？**  
A: 确认轮播已启用，文件夹路径正确，且包含图片文件。

## 下一步

- 查看 [README.md](README.md) 了解详细功能
- 查看 [TESTING.md](TESTING.md) 了解测试方法
- 查看 [STRUCTURE.md](STRUCTURE.md) 了解项目结构

## 获取帮助

如果遇到问题：

1. 查看日志：`journalctl -f | grep plasmashell`
2. 提交 Issue 到 GitHub
3. 加入社区讨论

祝你使用愉快！🎉
