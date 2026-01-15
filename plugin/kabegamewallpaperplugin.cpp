/*
 * Kabegame Wallpaper Plugin - Plugin Registration
 * Copyright (C) 2024 Kabegame Team
 */

#include "wallpaperbackend.h"

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class KabegameWallpaperPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kabegame.wallpaper"));
        
        // 注册 WallpaperBackend 类型
        qmlRegisterType<WallpaperBackend>(uri, 1, 0, "WallpaperBackend");
        
        qDebug() << "[Kabegame] 插件已注册: org.kabegame.wallpaper";
    }
};

#include "kabegamewallpaperplugin.moc"
