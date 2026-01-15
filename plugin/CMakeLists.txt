# Kabegame Wallpaper Plugin

set(wallpaper_SRCS
    wallpaperbackend.cpp
    kabegamewallpaperplugin.cpp
)

add_library(plasma_wallpaper_kabegame MODULE ${wallpaper_SRCS})

# 设置库文件前缀为 lib（Qt QML 插件加载器需要）
set_target_properties(plasma_wallpaper_kabegame PROPERTIES PREFIX "lib")

target_link_libraries(plasma_wallpaper_kabegame
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
    Qt5::DBus
    KF5::Plasma
    KF5::I18n
)

# 安装插件库到 QML 导入路径
install(TARGETS plasma_wallpaper_kabegame DESTINATION ${KDE_INSTALL_QMLDIR}/org/kabegame/wallpaper)

# 安装 qmldir 文件
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kabegame/wallpaper)

# 安装 QML 壁纸包
plasma_install_package(${CMAKE_SOURCE_DIR}/package org.kabegame.wallpaper wallpapers wallpaper)
