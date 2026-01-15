/*
 * Kabegame Wallpaper Plugin - Configuration UI
 * Copyright (C) 2024 Kabegame Team
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import org.kde.kirigami 2.20 as Kirigami

ColumnLayout {
    id: root
    
    // 配置绑定
    property alias cfg_Image: pathField.text
    property alias cfg_FillMode: fillModeCombo.currentValue
    property alias cfg_Transition: transitionCombo.currentValue
    property alias cfg_TransitionDuration: durationSpinBox.value
    property alias cfg_SlideshowInterval: intervalSpinBox.value
    property alias cfg_SlideshowOrder: orderCombo.currentValue
    property alias cfg_KabegameBridgeEnabled: bridgeCheckBox.checked
    
    // 判断当前路径是否为文件夹
    readonly property bool isFolder: {
        var path = pathField.text.trim()
        if (!path) return false
        var lowerPath = path.toLowerCase()
        var imageExtensions = [".png", ".jpg", ".jpeg", ".bmp", ".gif", ".webp", ".svg"]
        for (var i = 0; i < imageExtensions.length; i++) {
            if (lowerPath.endsWith(imageExtensions[i])) {
                return false
            }
        }
        return path.length > 0
    }
    
    readonly property string modeDescription: {
        if (!pathField.text.trim()) {
            return i18n("请选择壁纸图片或文件夹")
        }
        return isFolder ? i18n("文件夹模式（轮播已启用）") : i18n("单图模式")
    }
    
    spacing: Kirigami.Units.largeSpacing
    
    Kirigami.FormLayout {
        Layout.fillWidth: true
        
        // 壁纸路径选择
        RowLayout {
            Kirigami.FormData.label: i18n("壁纸路径：")
            
            TextField {
                id: pathField
                Layout.fillWidth: true
                placeholderText: i18n("选择图片或文件夹...")
            }
            
            Button {
                text: i18n("图片")
                icon.name: "document-open"
                onClicked: imageDialog.open()
                ToolTip.text: i18n("选择单张图片")
                ToolTip.visible: hovered
            }
            
            Button {
                text: i18n("文件夹")
                icon.name: "folder-open"
                onClicked: folderDialog.open()
                ToolTip.text: i18n("选择文件夹进行轮播")
                ToolTip.visible: hovered
            }
        }
        
        Label {
            text: modeDescription
            font.italic: true
            opacity: 0.8
            color: isFolder ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor
        }
        
        Item { Kirigami.FormData.isSection: true }
        
        // 填充模式
        ComboBox {
            id: fillModeCombo
            Kirigami.FormData.label: i18n("填充模式：")
            Layout.fillWidth: true
            
            textRole: "text"
            valueRole: "value"
            
            model: [
                { text: i18n("填充（保持比例，裁剪边缘）"), value: "fill" },
                { text: i18n("适应（保持比例，完整显示）"), value: "fit" },
                { text: i18n("拉伸（填满屏幕）"), value: "stretch" },
                { text: i18n("居中（不缩放）"), value: "center" },
                { text: i18n("平铺（重复）"), value: "tile" }
            ]
            
            Component.onCompleted: currentIndex = indexOfValue(cfg_FillMode)
        }
        
        // 过渡效果
        ComboBox {
            id: transitionCombo
            Kirigami.FormData.label: i18n("过渡效果：")
            Layout.fillWidth: true
            
            textRole: "text"
            valueRole: "value"
            
            model: [
                { text: i18n("无"), value: "none" },
                { text: i18n("淡入淡出"), value: "fade" },
                { text: i18n("滑动"), value: "slide" },
                { text: i18n("缩放"), value: "zoom" }
            ]
            
            Component.onCompleted: currentIndex = indexOfValue(cfg_Transition)
        }
        
        // 过渡时长
        SpinBox {
            id: durationSpinBox
            Kirigami.FormData.label: i18n("过渡时长（毫秒）：")
            from: 100
            to: 5000
            stepSize: 100
            editable: true
        }
        
        Item { Kirigami.FormData.isSection: true; visible: isFolder }
        
        // 轮播设置
        Label {
            Kirigami.FormData.label: i18n("轮播设置：")
            text: i18n("选择文件夹后自动启用轮播")
            visible: isFolder
            font.bold: true
        }
        
        SpinBox {
            id: intervalSpinBox
            Kirigami.FormData.label: i18n("轮播间隔（秒）：")
            from: 5
            to: 3600
            stepSize: 5
            editable: true
            visible: isFolder
        }
        
        ComboBox {
            id: orderCombo
            Kirigami.FormData.label: i18n("轮播顺序：")
            Layout.fillWidth: true
            visible: isFolder
            
            textRole: "text"
            valueRole: "value"
            
            model: [
                { text: i18n("随机"), value: "random" },
                { text: i18n("顺序"), value: "sequential" }
            ]
            
            Component.onCompleted: currentIndex = indexOfValue(cfg_SlideshowOrder)
        }
        
        Item { Kirigami.FormData.isSection: true }
        
        // Kabegame Bridge
        CheckBox {
            id: bridgeCheckBox
            Kirigami.FormData.label: i18n("Kabegame 集成：")
            text: i18n("启用 Kabegame Bridge (D-Bus)")
        }
        
        Label {
            Layout.fillWidth: true
            text: i18n("启用后，插件通过 D-Bus 与 Kabegame Daemon 通信，自动同步：\n• 壁纸图片路径\n• 填充模式和过渡效果\n• 轮播设置")
            wrapMode: Text.WordWrap
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            opacity: 0.7
        }
        
        Label {
            Layout.fillWidth: true
            text: bridgeCheckBox.checked ? i18n("⚠ 需要先启动 Kabegame Daemon") : ""
            wrapMode: Text.WordWrap
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            color: Kirigami.Theme.neutralTextColor
            visible: bridgeCheckBox.checked
        }
    }
    
    // 预览
    GroupBox {
        Layout.fillWidth: true
        title: i18n("当前设置")
        
        ColumnLayout {
            anchors.fill: parent
            spacing: Kirigami.Units.smallSpacing
            
            Label { text: i18n("模式：%1", isFolder ? i18n("文件夹轮播") : i18n("单张图片")) }
            Label { text: i18n("填充模式：%1", fillModeCombo.currentText) }
            Label { text: i18n("过渡效果：%1", transitionCombo.currentText) }
            Label { text: i18n("过渡时长：%1 毫秒", durationSpinBox.value) }
            Label { text: i18n("轮播间隔：%1 秒", intervalSpinBox.value); visible: isFolder }
        }
    }
    
    // 文件对话框
    FileDialog {
        id: imageDialog
        title: i18n("选择壁纸图片")
        folder: shortcuts.pictures
        nameFilters: [i18n("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.webp)"), i18n("所有文件 (*)")]
        onAccepted: pathField.text = fileUrl.toString().replace("file://", "")
    }
    
    FileDialog {
        id: folderDialog
        title: i18n("选择壁纸文件夹")
        folder: shortcuts.pictures
        selectFolder: true
        onAccepted: pathField.text = fileUrl.toString().replace("file://", "")
    }
}
