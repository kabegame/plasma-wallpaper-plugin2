#!/bin/bash

# Kabegame Plasma Wallpaper Plugin - 打包脚本

set -e

PLUGIN_NAME="org.kabegame.wallpaper"
VERSION="1.0.0"
PACKAGE_NAME="kabegame-wallpaper-${VERSION}.tar.gz"

echo "=== Kabegame Wallpaper Plugin 打包脚本 ==="
echo ""
echo "版本: $VERSION"
echo "包名: $PACKAGE_NAME"
echo ""

# 进入插件目录
cd "$(dirname "${BASH_SOURCE[0]}")"

# 创建临时打包目录
TEMP_DIR=$(mktemp -d)
cp -r "$PLUGIN_NAME" "$TEMP_DIR/"

# 打包
echo "正在打包..."
cd "$TEMP_DIR"
tar -czf "$PACKAGE_NAME" "$PLUGIN_NAME"

# 移动到原目录
mv "$PACKAGE_NAME" "$OLDPWD/"
cd "$OLDPWD"

# 清理临时目录
rm -rf "$TEMP_DIR"

echo ""
echo "✓ 打包完成: $PACKAGE_NAME"
echo ""
echo "安装方法:"
echo "1. 在 KDE 系统设置中:"
echo "   系统设置 → 外观 → 壁纸 → 获取新壁纸 → 从文件安装"
echo ""
echo "2. 或者手动解压:"
echo "   tar -xzf $PACKAGE_NAME -C ~/.local/share/plasma/wallpapers/"
echo ""
