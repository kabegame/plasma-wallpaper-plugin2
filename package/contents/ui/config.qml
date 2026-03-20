import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
import org.kabegame.wallpaper 1.0

ColumnLayout {
    id: root

    // KConfig 仍要求存在 cfg_Image 绑定；写入 wallpaper.configuration.Image 后桌面壁纸（main.qml）才能通过 configPathChanged 更新
    property string cfg_Image: ""
    readonly property int fixedCellWidth: Kirigami.Units.gridUnit * 12

    function writeWallpaperConfigImage(path) {
        if (!path) return
        cfg_Image = path
        var conf = (typeof wallpaper !== "undefined" && wallpaper && wallpaper.configuration) ? wallpaper.configuration : null
        if (conf) {
            conf.Image = path
            if (typeof conf.writeConfig === "function") {
                conf.writeConfig()
            }
        }
    }

    function aspectRatioNumber() {
        var raw = backend.galleryImageAspectRatio || "16:9"
        raw = ("" + raw).trim()
        if (raw.length === 0) {
            return 16 / 9
        }

        var left = 0
        var right = 0
        if (raw.indexOf(":") >= 0) {
            var parts1 = raw.split(":")
            if (parts1.length === 2) {
                left = parseFloat(parts1[0])
                right = parseFloat(parts1[1])
            }
        } else if (raw.indexOf("/") >= 0) {
            var parts2 = raw.split("/")
            if (parts2.length === 2) {
                left = parseFloat(parts2[0])
                right = parseFloat(parts2[1])
            }
        } else {
            var v = parseFloat(raw)
            if (!isNaN(v) && v > 0.01) {
                return v
            }
        }

        if (!isNaN(left) && !isNaN(right) && left > 0 && right > 0) {
            return left / right
        }
        return 16 / 9
    }

    spacing: Kirigami.Units.largeSpacing

    WallpaperBackend {
        id: backend

        onHintMessage: function(message) {
            if (message && message.length > 0) {
                messageLabel.text = message
            }
        }
        onImageConfigSyncRequested: function(path) {
            root.writeWallpaperConfigImage(path)
        }
    }

    Component.onCompleted: {
        backend.connectToKabegame()
    }

    Kirigami.FormLayout {
        Layout.fillWidth: true

        RowLayout {
            Kirigami.FormData.label: i18n("Kabegame:")
            spacing: Kirigami.Units.smallSpacing

            Rectangle {
                width: 10
                height: 10
                radius: width / 2
                color: backend.connected ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
            }

            Label {
                text: backend.connected ? i18n("Connected") : i18n("Disconnected")
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: i18n("Open Kabegame")
                icon.name: "system-run"
                onClicked: backend.openKabegame()
            }
        }

        Label {
            id: messageLabel
            text: ""
            visible: text.length > 0
            color: Kirigami.Theme.neutralTextColor
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    Item {
        id: gallerySection
        Layout.fillWidth: true
        Layout.fillHeight: true

        ColumnLayout {
            anchors.fill: parent
            spacing: Kirigami.Units.smallSpacing

            RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                Button {
                    id: filterButton
                    text: backend.filterDisplayText
                    icon.name: "view-filter"
                    display: Button.TextBesideIcon

                    Menu {
                        id: filterMenu
                        width: Math.max(filterButton.width, Kirigami.Units.gridUnit * 12)

                        MenuItem {
                            text: i18n("All")
                            onTriggered: backend.setProviderRootPath("all")
                        }

                        MenuSeparator { }

                        Menu {
                            title: i18n("Albums")
                            width: Kirigami.Units.gridUnit * 12

                            Repeater {
                                model: backend.albumList
                                MenuItem {
                                    text: modelData.name || modelData.id
                                    onTriggered: backend.setProviderRootPath("album/" + modelData.id)
                                }
                            }
                        }

                        Menu {
                            title: i18n("Tasks")
                            width: Kirigami.Units.gridUnit * 14

                            Repeater {
                                model: backend.taskList
                                MenuItem {
                                    text: (modelData.displayTime && modelData.displayTime.length > 0)
                                        ? (modelData.pluginId + " " + modelData.displayTime)
                                        : modelData.pluginId
                                    onTriggered: backend.setProviderRootPath("task/" + modelData.id)
                                }
                            }
                        }
                    }

                    onClicked: filterMenu.popup(filterButton, 0, filterButton.height)
                }

                Button {
                    id: sortButton
                    text: backend.sortDescending ? i18n("Time: Desc") : i18n("Time: Asc")
                    icon.name: "view-sort-ascending"
                    display: Button.TextBesideIcon

                    Menu {
                        id: sortMenu
                        width: Math.max(sortButton.width, Kirigami.Units.gridUnit * 10)

                        MenuItem {
                            text: i18n("Time: Asc")
                            onTriggered: backend.setSortDescending(false)
                        }
                        MenuItem {
                            text: i18n("Time: Desc")
                            onTriggered: backend.setSortDescending(true)
                        }
                    }

                    onClicked: sortMenu.popup(sortButton, 0, sortButton.height)
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            KCM.GridView {
                id: galleryGrid
                Layout.fillWidth: true
                Layout.fillHeight: true
                view.model: backend.galleryImages
                view.reuseItems: true
                clip: true

                // 单元宽度固定，不随窗口变化；高度按 Kabegame 的宽高比同步
                view.implicitCellWidth: {
                    return root.fixedCellWidth
                }
                view.implicitCellHeight: {
                    const aspect = root.aspectRatioNumber()
                    const thumbHeight = root.fixedCellWidth / aspect
                    return thumbHeight + Kirigami.Units.smallSpacing * 2 + Kirigami.Units.gridUnit * 3
                }

                view.delegate: KCM.GridDelegate {
                    id: wallpaperDelegate
                    hoverEnabled: true
                    opacity: galleryGrid.view.currentIndex === index ? 1 : 0.96

                    text: modelData.localPath ? modelData.localPath.split("/").pop() : modelData.id

                    actions: [
                        Kirigami.Action {
                            icon.name: "document-open-folder"
                            tooltip: i18n("Open Containing Folder")
                            visible: !!modelData.localPath
                            onTriggered: {
                                var path = (modelData.localPath || "").toString()
                                var idx = path.lastIndexOf("/")
                                var dir = (idx > 0) ? path.substring(0, idx) : path
                                if (dir.length > 0) {
                                    var url = (dir.indexOf("file://") === 0) ? dir : ("file://" + dir)
                                    Qt.openUrlExternally(url)
                                }
                            }
                        }
                    ]

                    thumbnail: Rectangle {
                        anchors.fill: parent
                        radius: Number(Kirigami.Units.cornerRadius) || 0
                        color: Kirigami.Theme.backgroundColor
                        border.width: 1
                        border.color: (galleryGrid.view.currentIndex === index || wallpaperDelegate.hovered)
                            ? Kirigami.Theme.highlightColor
                            : Kirigami.Theme.disabledTextColor
                        clip: true

                        Kirigami.Icon {
                            anchors.centerIn: parent
                            width: Kirigami.Units.iconSizes.large
                            height: width
                            source: "view-preview"
                            visible: !thumbImage.visible
                        }

                        Image {
                            id: thumbImage
                            anchors.fill: parent
                            source: "image://kabegame/" + modelData.id
                            fillMode: Image.PreserveAspectCrop
                            asynchronous: true
                            cache: true
                            visible: status === Image.Ready
                        }
                    }

                    onClicked: {
                        backend.setWallpaperByImageId(modelData.id)
                        root.writeWallpaperConfigImage(modelData.localPath || "")
                        galleryGrid.view.currentIndex = index
                    }
                }

                Component.onCompleted: {
                    view.positionViewAtBeginning()
                }
            }

            KCM.SettingHighlighter {
                target: galleryGrid
                highlight: cfg_Image.length > 0
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing
                Layout.topMargin: Kirigami.Units.smallSpacing

                Label {
                    text: i18n("Page %1 · 100 per page · %2 total", backend.galleryPage, backend.galleryTotal)
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }

                Button {
                    text: i18n("Previous")
                    enabled: backend.galleryPage > 1
                    onClicked: backend.loadGalleryPage(backend.galleryPage - 1)
                }

                Button {
                    text: i18n("Next")
                    enabled: backend.galleryImages.length >= backend.galleryPageSize
                    onClicked: backend.loadGalleryPage(backend.galleryPage + 1)
                }
            }
        }
    }
}
