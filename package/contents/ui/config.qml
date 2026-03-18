import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.20 as Kirigami
import org.kabegame.wallpaper 1.0

ColumnLayout {
    id: root

    // KConfig 仍要求存在 cfg_Image 绑定
    property string cfg_Image: ""

    spacing: Kirigami.Units.largeSpacing

    WallpaperBackend {
        id: backend

        onHintMessage: function(message) {
            if (message && message.length > 0) {
                messageLabel.text = message
            }
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

    GroupBox {
        Layout.fillWidth: true
        Layout.fillHeight: true
        title: i18n("Kabegame 画廊（全部）")

        ColumnLayout {
            anchors.fill: parent
            spacing: Kirigami.Units.smallSpacing

            Flickable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: grid.width
                contentHeight: grid.height

                GridView {
                    id: grid
                    width: parent.width
                    height: contentHeight
                    cellWidth: Math.max(120, Math.floor(width / 4))
                    cellHeight: cellWidth * 0.7
                    model: backend.galleryImages
                    interactive: false
                    reuseItems: true

                    delegate: Item {
                        width: grid.cellWidth
                        height: grid.cellHeight

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 4
                            color: Kirigami.Theme.backgroundColor
                            border.width: 1
                            border.color: Kirigami.Theme.disabledTextColor
                            radius: 4
                            clip: true

                            Image {
                                anchors.fill: parent
                                source: "image://kabegame/" + modelData.id
                                fillMode: Image.PreserveAspectCrop
                                asynchronous: true
                                cache: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    backend.setWallpaperByImageId(modelData.id)
                                }
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                Label {
                    text: i18n("第 %1 页 / 共约 %2 张", backend.galleryPage, backend.galleryTotal)
                    Layout.fillWidth: true
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
