#!/bin/bash
# Kabegame Wallpaper Plugin - 构建并安装脚本

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "=========================================="
echo "  Kabegame 壁纸插件安装 (C++ 版本)"
echo "=========================================="

# 检查依赖
echo ""
echo "[1/4] 检查依赖..."

if ! command -v cmake &> /dev/null; then
    echo "错误: 未找到 cmake"
    echo "请安装: sudo apt install cmake extra-cmake-modules"
    exit 1
fi

# 检查 KDE 开发包
MISSING_DEPS=""
if ! pkg-config --exists Qt5Core 2>/dev/null; then
    MISSING_DEPS="$MISSING_DEPS qtbase5-dev"
fi

if [ -n "$MISSING_DEPS" ]; then
    echo "警告: 可能缺少依赖"
    echo "建议运行: sudo apt install cmake extra-cmake-modules libkf5plasma-dev libkf5i18n-dev libkf5package-dev qtdeclarative5-dev"
fi

# 创建构建目录
echo ""
echo "[2/4] 配置项目..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DKDE_INSTALL_USE_QT_SYS_PATHS=ON

# 编译
echo ""
echo "[3/4] 编译..."
make -j$(nproc)

# 安装
echo ""
echo "[4/4] 安装..."
sudo make install

echo ""
echo "=========================================="
echo "  安装完成！"
echo "=========================================="
echo ""
echo "C++ 插件已安装到系统 QML 路径"
echo ""
echo "重启 Plasma Shell 以生效:"
echo "  kquitapp5 plasmashell && kstart5 plasmashell"
echo ""
echo "或者运行:"
echo "  ./restart-plasma.sh"
