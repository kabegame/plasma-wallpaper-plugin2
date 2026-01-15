/*
 * Kabegame Wallpaper Plugin
 * Copyright (C) 2024 Kabegame Team
 * 
 * This plugin provides advanced wallpaper transitions and fill modes
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root
    
    // 配置属性
    readonly property string configImage: wallpaper.configuration.Image || ""
    readonly property string configFolder: wallpaper.configuration.ImageFolder || ""
    readonly property string fillMode: wallpaper.configuration.FillMode || "fill"
    readonly property string transition: wallpaper.configuration.Transition || "fade"
    readonly property int transitionDuration: wallpaper.configuration.TransitionDuration || 500
    readonly property bool slideshowEnabled: wallpaper.configuration.SlideshowEnabled || false
    readonly property int slideshowInterval: wallpaper.configuration.SlideshowInterval || 60
    readonly property string slideshowOrder: wallpaper.configuration.SlideshowOrder || "random"
    readonly property bool kabegameBridge: wallpaper.configuration.KabegameBridgeEnabled || false
    
    // 内部状态
    property string currentImageSource: ""
    property var imageList: []
    property int currentIndex: 0
    
    // 双图层用于过渡
    Item {
        id: wallpaperContainer
        anchors.fill: parent
        clip: true
        
        // 底层图片（当前显示的）
        Image {
            id: baseImage
            anchors.fill: parent
            source: currentImageSource
            asynchronous: true
            cache: false
            smooth: true
            
            // 填充模式
            fillMode: {
                switch (root.fillMode) {
                    case "fill": return Image.PreserveAspectCrop
                    case "fit": return Image.PreserveAspectFit
                    case "stretch": return Image.Stretch
                    case "center": return Image.Pad
                    case "tile": return Image.Tile
                    default: return Image.PreserveAspectCrop
                }
            }
            
            horizontalAlignment: Image.AlignHCenter
            verticalAlignment: Image.AlignVCenter
        }
        
        // 顶层图片（用于过渡）
        Image {
            id: topImage
            anchors.fill: parent
            asynchronous: true
            cache: false
            smooth: true
            opacity: 0
            visible: opacity > 0
            
            // 填充模式
            fillMode: baseImage.fillMode
            horizontalAlignment: Image.AlignHCenter
            verticalAlignment: Image.AlignVCenter
            
            // 用于滑动过渡的变换
            transform: Translate {
                id: slideTransform
                x: 0
                y: 0
            }
            
            // 用于缩放过渡的变换
            scale: 1.0
        }
    }
    
    // 过渡动画组
    ParallelAnimation {
        id: transitionAnimation
        
        // 淡入淡出
        NumberAnimation {
            target: topImage
            property: "opacity"
            from: 0
            to: 1
            duration: root.transitionDuration
            easing.type: Easing.InOutQuad
        }
        
        // 滑动（仅在 slide 模式下生效）
        NumberAnimation {
            id: slideAnimation
            target: slideTransform
            property: "x"
            duration: root.transitionDuration
            easing.type: Easing.InOutCubic
        }
        
        // 缩放（仅在 zoom 模式下生效）
        NumberAnimation {
            id: zoomAnimation
            target: topImage
            property: "scale"
            duration: root.transitionDuration
            easing.type: Easing.InOutQuad
        }
        
        onFinished: {
            // 过渡完成后，将顶层图片变为底层
            baseImage.source = topImage.source
            currentImageSource = topImage.source
            
            // 重置顶层图片状态
            topImage.opacity = 0
            topImage.scale = 1.0
            slideTransform.x = 0
            slideTransform.y = 0
        }
    }
    
    // 切换壁纸函数
    function switchWallpaper(newSource) {
        if (!newSource || newSource === currentImageSource) {
            return
        }
        
        console.log("Switching wallpaper:", newSource)
        
        // 准备过渡效果
        prepareTransition(newSource)
        
        // 启动过渡动画
        transitionAnimation.start()
    }
    
    // 准备过渡效果
    function prepareTransition(newSource) {
        topImage.source = newSource
        
        switch (root.transition) {
            case "none":
                // 无过渡，直接切换
                baseImage.source = newSource
                currentImageSource = newSource
                break
                
            case "fade":
                // 淡入淡出（默认动画已包含）
                slideAnimation.enabled = false
                zoomAnimation.enabled = false
                break
                
            case "slide":
                // 滑动
                slideAnimation.enabled = true
                zoomAnimation.enabled = false
                slideTransform.x = root.width
                slideAnimation.from = root.width
                slideAnimation.to = 0
                break
                
            case "zoom":
                // 缩放
                slideAnimation.enabled = false
                zoomAnimation.enabled = true
                topImage.scale = 0.8
                zoomAnimation.from = 0.8
                zoomAnimation.to = 1.0
                break
                
            case "rotate":
                // 旋转（TODO: 需要更复杂的实现）
                slideAnimation.enabled = false
                zoomAnimation.enabled = false
                break
                
            default:
                slideAnimation.enabled = false
                zoomAnimation.enabled = false
        }
    }
    
    // 轮播定时器
    Timer {
        id: slideshowTimer
        interval: root.slideshowInterval * 1000
        running: root.slideshowEnabled && imageList.length > 1
        repeat: true
        
        onTriggered: {
            nextWallpaper()
        }
    }
    
    // 下一张壁纸
    function nextWallpaper() {
        if (imageList.length === 0) {
            return
        }
        
        if (root.slideshowOrder === "random") {
            currentIndex = Math.floor(Math.random() * imageList.length)
        } else {
            currentIndex = (currentIndex + 1) % imageList.length
        }
        
        switchWallpaper(imageList[currentIndex])
    }
    
    // 加载文件夹中的图片
    function loadImageFolder(folderPath) {
        if (!folderPath) {
            return
        }
        
        // TODO: 使用 FolderListModel 或文件对话框加载图片列表
        console.log("Loading images from folder:", folderPath)
        
        // 临时实现：假设我们有图片列表
        imageList = []
        currentIndex = 0
    }
    
    // 监听配置变化
    Connections {
        target: wallpaper
        
        function onConfigurationChanged() {
            console.log("Configuration changed")
            
            // 更新单张图片
            if (configImage && configImage !== currentImageSource) {
                if (root.transition === "none") {
                    baseImage.source = configImage
                    currentImageSource = configImage
                } else {
                    switchWallpaper(configImage)
                }
            }
            
            // 更新文件夹轮播
            if (configFolder) {
                loadImageFolder(configFolder)
            }
        }
    }
    
    // 初始化
    Component.onCompleted: {
        console.log("Kabegame Wallpaper Plugin loaded")
        console.log("Fill mode:", root.fillMode)
        console.log("Transition:", root.transition)
        console.log("Duration:", root.transitionDuration)
        
        // 加载初始图片
        if (configImage) {
            currentImageSource = configImage
            baseImage.source = configImage
        }
        
        // 加载文件夹
        if (configFolder && slideshowEnabled) {
            loadImageFolder(configFolder)
        }
    }
}
