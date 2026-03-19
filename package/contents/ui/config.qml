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
        backend.loadGalleryPage(1)
    }

    Kirigami.FormLayout {
        Layout.fillWidth: true

        RowLayout {
            Kirigami.FormData.label: i18n("Kabegame：")
            spacing: Kirigami.Units.smallSpacing

            Rectangle {
                width: 10
                height: 10
                radius: width / 2
                color: backend.connected ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
            }

            Label {
                text: backend.connected ? i18n("已连接") : i18n("未连接")
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: i18n("打开 Kabegame")
                icon.name: "system-run"
                onClicked: backend.openKabegame()
            }

            Button {
                text: i18n("从文件夹打开…")
                icon.name: "document-open"
                onClicked: openFileDialog.open()
            }
        }

        FileDialog {
            id: openFileDialog
            title: i18n("选择壁纸图片")
            nameFilters: [ i18n("Image files") + " (*.png *.jpg *.jpeg *.bmp *.gif *.webp)", i18n("All files") + " (*)" ]
            fileMode: FileDialog.OpenFile
            onAccepted: {
                var path = openFileDialog.selectedFile.toString()
                if (path.indexOf("file://") === 0) {
                    path = path.replace("file://", "")
                }
                if (path.length > 0) {
                    root.writeWallpaperConfigImage(path)
                    backend.syncImageConfig(path)
                }
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
                    text: i18n("第 %1 页 · 本页 100 张 · 共 %2 张", backend.galleryPage, backend.galleryTotal)
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }

                Button {
                    text: i18n("上一页")
                    enabled: backend.galleryPage > 1
                    onClicked: backend.loadGalleryPage(backend.galleryPage - 1)
                }

                Button {
                    text: i18n("下一页")
                    enabled: backend.galleryImages.length >= backend.galleryPageSize
                    onClicked: backend.loadGalleryPage(backend.galleryPage + 1)
                }
            }
        }
    }
}
