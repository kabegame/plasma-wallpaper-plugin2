#!/bin/bash
# Build deb package for Kabegame Plasma Wallpaper plugin.
# Run from plugin repo root. Output: dist/kabegame-plasma-wallpaper_*_amd64.deb
# (dpkg-buildpackage writes to parent dir first; artifacts are moved into dist/.)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
DIST_DIR="$SCRIPT_DIR/dist"
mkdir -p "$DIST_DIR"

# 1. Read version from package/metadata.json
VERSION=$(python3 -c "import json; print(json.load(open('package/metadata.json'))['KPlugin']['Version'])")
echo "[build-deb] Version: $VERSION"

# 2. Build translations
echo "[build-deb] Building translations..."
./package/translate/build.sh

# 3. Configure and compile
echo "[build-deb] Configuring and building..."
mkdir -p build
cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
make -j$(nproc)
cd ..

# 4. Update debian/changelog with version from metadata
echo "[build-deb] Updating debian/changelog to $VERSION..."
sed -i "1s/.*/kabegame-plasma-wallpaper ($VERSION) unstable; urgency=medium/" debian/changelog

# 5. Build deb (dpkg-buildpackage emits to parent of source tree)
echo "[build-deb] Building deb package..."
dpkg-buildpackage -us -uc -b

# 6. Move artifacts from parent into dist/ so monorepo parent stays clean
for f in ../kabegame-plasma-wallpaper_*; do
    if [ -e "$f" ]; then
        mv "$f" "$DIST_DIR/"
    fi
done

DEB="$DIST_DIR/kabegame-plasma-wallpaper_${VERSION}_amd64.deb"
if [ -f "$DEB" ]; then
    echo ""
    echo "[build-deb] Done. Package: $DEB"
else
    echo "[build-deb] Warning: expected .deb not found at $DEB (check $DIST_DIR/)"
fi
