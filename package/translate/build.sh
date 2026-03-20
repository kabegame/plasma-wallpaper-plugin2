#!/bin/sh
# Build .mo translation files and install to package/contents/locale/
# Run from repo root or from package/translate/
# Requires: gettext (msgfmt)

DIR=$(cd "$(dirname "$0")" && pwd)
# Plugin Id from metadata.json (used as gettext domain for Plasma wallpapers)
DOMAIN="plasma_wallpaper_org.kabegame.wallpaper"
LOCALE_ROOT="${DIR}/../contents/locale"

if [ ! -d "$DIR" ] || [ ! -f "$DIR/template.pot" ]; then
    echo "[build] Error: Run from package/translate/ or ensure template.pot exists."
    exit 1
fi

echo "[build] Compiling translations (domain: $DOMAIN)"

for po in "$DIR"/*.po; do
    [ -f "$po" ] || continue
    locale=$(basename "$po" .po)
    echo "  $locale"
    msgfmt -o "${DIR}/${locale}.mo" "$po" || { echo "[build] msgfmt failed for $po"; exit 1; }
    install_path="${LOCALE_ROOT}/${locale}/LC_MESSAGES/${DOMAIN}.mo"
    mkdir -p "$(dirname "$install_path")"
    mv "${DIR}/${locale}.mo" "$install_path"
done

echo "[build] Done. .mo files installed under contents/locale/<lang>/LC_MESSAGES/"
