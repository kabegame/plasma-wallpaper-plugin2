#!/bin/bash
# Kabegame Wallpaper Plugin - 构建并安装脚本 (C++ 版本)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "=========================================="
echo "  Kabegame 壁纸插件安装 (C++ 版本)"
echo "=========================================="

# 先构建
"$SCRIPT_DIR/build.sh"

# 安装
echo ""
echo "安装插件..."
cd "$BUILD_DIR"
sudo make install

echo ""
echo "=========================================="
echo "  安装完成！"
echo "=========================================="
echo ""
echo "重启 Plasma Shell 以生效:"
echo "  kquitapp5 plasmashell && kstart5 plasmashell"
echo ""
echo "或者运行:"
echo "  ./restart-plasma.sh"
