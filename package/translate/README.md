# Kabegame Wallpaper plugin – translations

- **Default language:** English (source strings in QML/C++).
- **Supported:** Japanese (ja), Korean (ko), Simplified Chinese (zh_CN).

## Rebuild .mo files

After editing `.po` files or adding new strings to `template.pot`:

```bash
./build.sh
```

Requires `gettext` (e.g. `msgfmt`). Output: `../contents/locale/<lang>/LC_MESSAGES/plasma_wallpaper_org.kabegame.wallpaper.mo`.

## Extract/merge (optional)

To refresh `template.pot` from source and merge into `.po` files, use gettext/xgettext on `package/contents/ui/*.qml` and `plugin/*.cpp` with ki18n keywords, then run `msgmerge` for each `.po`.
