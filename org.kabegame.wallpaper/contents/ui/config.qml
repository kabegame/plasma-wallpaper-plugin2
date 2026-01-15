/*
 * Kabegame Wallpaper Plugin - Configuration UI
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import org.kde.kirigami 2.20 as Kirigami

ColumnLayout {
    id: root
    
    property alias cfg_Image: imageField.text
    property alias cfg_ImageFolder: folderField.text
    property alias cfg_FillMode: fillModeCombo.currentValue
    property alias cfg_Transition: transitionCombo.currentValue
    property alias cfg_TransitionDuration: durationSpinBox.value
    property alias cfg_SlideshowEnabled: slideshowCheckBox.checked
    property alias cfg_SlideshowInterval: intervalSpinBox.value
    property alias cfg_SlideshowOrder: orderCombo.currentValue
    property alias cfg_KabegameBridgeEnabled: bridgeCheckBox.checked
    
    spacing: Kirigami.Units.largeSpacing
    
    // 壁纸图片选择
    Kirigami.FormLayout {
        Layout.fillWidth: true
        
        // 单张图片
        RowLayout {
            Kirigami.FormData.label: i18n("Wallpaper Image:")
            
            TextField {
                id: imageField
                Layout.fillWidth: true
                placeholderText: i18n("Select an image file...")
            }
            
            Button {
                text: i18n("Browse...")
                icon.name: "document-open"
                onClicked: imageDialog.open()
            }
        }
        
        // 文件夹（轮播用）
        RowLayout {
            Kirigami.FormData.label: i18n("Slideshow Folder:")
            
            TextField {
                id: folderField
                Layout.fillWidth: true
                placeholderText: i18n("Select a folder for slideshow...")
            }
            
            Button {
                text: i18n("Browse...")
                icon.name: "folder-open"
                onClicked: folderDialog.open()
            }
        }
        
        Item {
            Kirigami.FormData.isSection: true
        }
        
        // 填充模式
        ComboBox {
            id: fillModeCombo
            Kirigami.FormData.label: i18n("Fill Mode:")
            Layout.fillWidth: true
            
            textRole: "text"
            valueRole: "value"
            
            model: [
                { text: i18n("Fill (Preserve Aspect, Crop)"), value: "fill" },
                { text: i18n("Fit (Preserve Aspect, Show All)"), value: "fit" },
                { text: i18n("Stretch (Fill Screen)"), value: "stretch" },
                { text: i18n("Center (No Scaling)"), value: "center" },
                { text: i18n("Tile (Repeat)"), value: "tile" }
            ]
            
            Component.onCompleted: {
                currentIndex = indexOfValue(cfg_FillMode)
            }
        }
        
        // 过渡效果
        ComboBox {
            id: transitionCombo
            Kirigami.FormData.label: i18n("Transition Effect:")
            Layout.fillWidth: true
            
            textRole: "text"
            valueRole: "value"
            
            model: [
                { text: i18n("None"), value: "none" },
                { text: i18n("Fade"), value: "fade" },
                { text: i18n("Slide"), value: "slide" },
                { text: i18n("Zoom"), value: "zoom" },
                { text: i18n("Rotate (TODO)"), value: "rotate" }
            ]
            
            Component.onCompleted: {
                currentIndex = indexOfValue(cfg_Transition)
            }
        }
        
        // 过渡时长
        SpinBox {
            id: durationSpinBox
            Kirigami.FormData.label: i18n("Transition Duration (ms):")
            from: 100
            to: 5000
            stepSize: 100
            editable: true
        }
        
        Item {
            Kirigami.FormData.isSection: true
        }
        
        // 轮播开关
        CheckBox {
            id: slideshowCheckBox
            Kirigami.FormData.label: i18n("Slideshow:")
            text: i18n("Enable slideshow")
        }
        
        // 轮播间隔
        SpinBox {
            id: intervalSpinBox
            Kirigami.FormData.label: i18n("Slideshow Interval (seconds):")
            from: 5
            to: 3600
            stepSize: 5
            editable: true
            enabled: slideshowCheckBox.checked
        }
        
        // 轮播顺序
        ComboBox {
            id: orderCombo
            Kirigami.FormData.label: i18n("Slideshow Order:")
            Layout.fillWidth: true
            enabled: slideshowCheckBox.checked
            
            textRole: "text"
            valueRole: "value"
            
            model: [
                { text: i18n("Random"), value: "random" },
                { text: i18n("Sequential"), value: "sequential" }
            ]
            
            Component.onCompleted: {
                currentIndex = indexOfValue(cfg_SlideshowOrder)
            }
        }
        
        Item {
            Kirigami.FormData.isSection: true
        }
        
        // Kabegame Bridge
        CheckBox {
            id: bridgeCheckBox
            Kirigami.FormData.label: i18n("Kabegame Integration:")
            text: i18n("Enable Kabegame Bridge (requires Kabegame running)")
        }
        
        Label {
            Layout.fillWidth: true
            text: i18n("When enabled, the plugin will sync with Kabegame for automatic wallpaper updates.")
            wrapMode: Text.WordWrap
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            opacity: 0.7
        }
    }
    
    // 预览区域（可选）
    GroupBox {
        Layout.fillWidth: true
        title: i18n("Preview")
        
        ColumnLayout {
            anchors.fill: parent
            
            Label {
                text: i18n("Current Settings:")
                font.bold: true
            }
            
            Label {
                text: i18n("Fill Mode: %1", fillModeCombo.currentText)
            }
            
            Label {
                text: i18n("Transition: %1", transitionCombo.currentText)
            }
            
            Label {
                text: i18n("Duration: %1ms", durationSpinBox.value)
            }
            
            Label {
                text: i18n("Slideshow: %1", slideshowCheckBox.checked ? i18n("Enabled") : i18n("Disabled"))
            }
        }
    }
    
    // 文件对话框
    FileDialog {
        id: imageDialog
        title: i18n("Select Wallpaper Image")
        folder: shortcuts.pictures
        nameFilters: [
            i18n("Image files (*.png *.jpg *.jpeg *.bmp *.gif *.webp)"),
            i18n("All files (*)")
        ]
        onAccepted: {
            imageField.text = fileUrl.toString().replace("file://", "")
        }
    }
    
    FileDialog {
        id: folderDialog
        title: i18n("Select Wallpaper Folder")
        folder: shortcuts.pictures
        selectFolder: true
        onAccepted: {
            folderField.text = fileUrl.toString().replace("file://", "")
        }
    }
}
