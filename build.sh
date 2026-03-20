#!/bin/bash
# Kabegame Wallpaper Plugin - 构建脚本

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "=========================================="
echo "  Kabegame 壁纸插件构建"
echo "=========================================="

# 检查依赖
echo "检查依赖..."
if ! command -v cmake &> /dev/null; then
    echo "错误: 未找到 cmake"
    echo "请安装: sudo apt install cmake"
    exit 1
fi

# 检查 KDE 开发包
if ! pkg-config --exists plasma; then
    echo "警告: 未找到 plasma pkg-config"
    echo "可能需要安装: sudo apt install libkf5plasma-dev extra-cmake-modules"
fi

echo "编译翻译文件..."
cd "$SCRIPT_DIR/package/translate"
./build.sh
cd "$SCRIPT_DIR"

# 创建构建目录
echo "创建构建目录..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 配置
echo "配置项目..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr

# 编译
echo "编译..."
make -j$(nproc)

echo ""
echo "=========================================="
echo "  构建完成！"
echo "=========================================="
echo ""
echo "运行以下命令安装:"
echo "  cd $BUILD_DIR && sudo make install"
echo ""
echo "或者使用: ./install.sh"
