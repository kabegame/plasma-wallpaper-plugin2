cmake_minimum_required(VERSION 3.16)
project(kabegame-wallpaper-plugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 查找依赖
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Network
    Quick
    Qml
    Multimedia
)

find_package(KF5 REQUIRED COMPONENTS
    Plasma
    I18n
    Package
)

# 插件版本（.so 名用下划线，如 2_2_0；与 package/metadata.json 中 Version 保持一致）
set(KABEGAME_WALLPAPER_VERSION "2.2.0" CACHE STRING "Kabegame wallpaper plugin version")

# 添加插件子目录
add_subdirectory(plugin)
