cmake_minimum_required(VERSION 3.19)
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

# Read version from package/metadata.json (single source of truth)
set(METADATA_JSON_PATH "${CMAKE_SOURCE_DIR}/package/metadata.json")
if(EXISTS "${METADATA_JSON_PATH}")
    file(READ "${METADATA_JSON_PATH}" METADATA_JSON)
    string(JSON KABEGAME_WALLPAPER_VERSION GET "${METADATA_JSON}" KPlugin Version)
    if(NOT KABEGAME_WALLPAPER_VERSION)
        set(KABEGAME_WALLPAPER_VERSION "0.0.0")
        message(WARNING "Could not parse Version from ${METADATA_JSON_PATH}, using 0.0.0")
    endif()
else()
    set(KABEGAME_WALLPAPER_VERSION "0.0.0")
    message(WARNING "${METADATA_JSON_PATH} not found, using version 0.0.0")
endif()

add_subdirectory(plugin)
