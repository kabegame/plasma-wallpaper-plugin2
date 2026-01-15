#!/bin/bash

# Kabegame Plasma Wallpaper Plugin - 安装脚本

set -e

PLUGIN_NAME="org.kabegame.wallpaper"
PLUGIN_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/$PLUGIN_NAME"
INSTALL_DIR="$HOME/.local/share/plasma/wallpapers/$PLUGIN_NAME"

echo "=== Kabegame Wallpaper Plugin 安装脚本 ==="
echo ""

# 检查插件目录是否存在
if [ ! -d "$PLUGIN_DIR" ]; then
    echo "错误: 找不到插件目录: $PLUGIN_DIR"
    exit 1
fi

# 创建安装目录
echo "创建安装目录: $INSTALL_DIR"
mkdir -p "$INSTALL_DIR"

# 复制插件文件
echo "复制插件文件..."
cp -r "$PLUGIN_DIR"/* "$INSTALL_DIR/"

echo ""
echo "✓ 插件已安装到: $INSTALL_DIR"
echo ""
echo "下一步:"
echo "1. 重启 Plasma Shell:"
echo "   kquitapp5 plasmashell && kstart5 plasmashell"
echo ""
echo "   或者重新登录"
echo ""
echo "2. 打开系统设置 → 外观 → 壁纸"
echo "3. 在壁纸类型下拉菜单中选择 'Kabegame Wallpaper'"
echo ""
