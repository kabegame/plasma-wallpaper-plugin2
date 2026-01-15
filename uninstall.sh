#!/bin/bash

# Kabegame Plasma Wallpaper Plugin - 卸载脚本

set -e

PLUGIN_NAME="org.kabegame.wallpaper"
INSTALL_DIR="$HOME/.local/share/plasma/wallpapers/$PLUGIN_NAME"

echo "=== Kabegame Wallpaper Plugin 卸载脚本 ==="
echo ""

# 检查插件是否已安装
if [ ! -d "$INSTALL_DIR" ]; then
    echo "插件未安装或已被卸载"
    exit 0
fi

# 删除插件目录
echo "删除插件: $INSTALL_DIR"
rm -rf "$INSTALL_DIR"

echo ""
echo "✓ 插件已卸载"
echo ""
echo "请重启 Plasma Shell 以完全移除插件:"
echo "  kquitapp5 plasmashell && kstart5 plasmashell"
echo ""
echo "或者重新登录"
echo ""
