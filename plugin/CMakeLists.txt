# Kabegame Wallpaper Plugin

# 版本号转为下划线形式，用于 .so 文件名，避免缓存冲突（如 2.2.0 -> 2_2_0）
string(REPLACE "." "_" KABEGAME_WALLPAPER_VERSION_UNDERSCORE "${KABEGAME_WALLPAPER_VERSION}")
set(KABEGAME_PLUGIN_BASENAME "plasma_wallpaper_kabegame_${KABEGAME_WALLPAPER_VERSION_UNDERSCORE}")

set(wallpaper_SRCS
    wallpaperbackend.cpp
    ipcclient.cpp
    thumbnailprovider.cpp
    kabegamewallpaperplugin.cpp
)

add_library(plasma_wallpaper_kabegame MODULE ${wallpaper_SRCS})

# 设置库前缀为 lib，输出名为带版本号（下划线），如 libplasma_wallpaper_kabegame_2_2_0.so
set_target_properties(plasma_wallpaper_kabegame PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "${KABEGAME_PLUGIN_BASENAME}"
)

target_link_libraries(plasma_wallpaper_kabegame
    Qt5::Core
    Qt5::Network
    Qt5::Quick
    Qt5::Qml
    Qt5::Multimedia
    KF5::Plasma
    KF5::I18n
)

# 安装插件库到 QML 导入路径
install(TARGETS plasma_wallpaper_kabegame DESTINATION ${KDE_INSTALL_QMLDIR}/org/kabegame/wallpaper)

# 生成并安装 qmldir（plugin 名与带版本号的 .so 一致）
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/qmldir.in
    ${CMAKE_CURRENT_BINARY_DIR}/qmldir
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kabegame/wallpaper)

# 安装 QML 壁纸包
plasma_install_package(${CMAKE_SOURCE_DIR}/package org.kabegame.wallpaper wallpapers wallpaper)
