cmake_minimum_required(VERSION 3.16)
project(kabegame-wallpaper-plugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.0.0")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Network
    Quick
    Qml
    Multimedia
)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    I18n
)

find_package(Plasma REQUIRED)

set(KABEGAME_WALLPAPER_VERSION "2.2.14" CACHE STRING "Kabegame wallpaper plugin version")

add_subdirectory(plugin)
