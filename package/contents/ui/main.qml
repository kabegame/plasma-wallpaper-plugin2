/*
 * Kabegame Wallpaper Plugin
 * Copyright (C) 2024 Kabegame Team
 * 
 * QML 只负责 UI 展示，业务逻辑在 C++ 后端
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kabegame.wallpaper 1.0

Item {
    id: root
    
    // 安全读取配置
    readonly property var cfg: wallpaper && wallpaper.configuration ? wallpaper.configuration : null
    readonly property string configPath: cfg ? (cfg.Image || "") : ""
    readonly property string configFillMode: cfg ? (cfg.FillMode || "fill") : "fill"
    readonly property string configTransition: cfg ? (cfg.Transition || "fade") : "fade"
    readonly property int configTransitionDuration: cfg ? (cfg.TransitionDuration || 500) : 500
    readonly property int configSlideshowInterval: cfg ? (cfg.SlideshowInterval || 60) : 60
    readonly property string configSlideshowOrder: cfg ? (cfg.SlideshowOrder || "random") : "random"
    readonly property bool configBridgeEnabled: cfg ? (cfg.KabegameBridgeEnabled || false) : false
    
    // C++ 后端
    WallpaperBackend {
        id: backend
        
        // 绑定配置属性
        path: root.configPath
        fillMode: root.configFillMode
        transition: root.configTransition
        transitionDuration: root.configTransitionDuration
        slideshowInterval: root.configSlideshowInterval
        slideshowOrder: root.configSlideshowOrder
        bridgeEnabled: root.configBridgeEnabled
        
        // 处理壁纸切换请求
        onWallpaperChangeRequested: function(newWallpaper) {
            switchWallpaper(newWallpaper)
        }
    }
    
    // 内部状态
    property bool isTransitioning: false
    property int activeImage: 0  // 0 = imageA, 1 = imageB
    property bool pendingNoTransition: false  // 等待无过渡切换
    
    // 获取 QML Image.fillMode
    function getImageFillMode() {
        switch (backend.effectiveFillMode) {
            case "fill": return Image.PreserveAspectCrop
            case "fit": return Image.PreserveAspectFit
            case "stretch": return Image.Stretch
            case "center": return Image.Pad
            case "tile": return Image.Tile
            default: return Image.PreserveAspectCrop
        }
    }
    
    // 壁纸容器
    Item {
        id: wallpaperContainer
        anchors.fill: parent
        clip: true
        
        Image {
            id: imageA
            anchors.fill: parent
            asynchronous: true
            cache: true
            smooth: true
            fillMode: getImageFillMode()
            horizontalAlignment: Image.AlignHCenter
            verticalAlignment: Image.AlignVCenter
            opacity: 1
            z: activeImage === 0 ? 1 : 0
            
            transform: Translate {
                id: slideTransformA
                x: 0
            }
            scale: 1.0
            
            onStatusChanged: {
                if (status === Image.Ready && pendingNoTransition && activeImage === 1) {
                    // imageA 加载完成，执行无过渡切换
                    pendingNoTransition = false
                    imageA.opacity = 1
                    imageB.opacity = 0
                    activeImage = 0
                }
            }
        }
        
        Image {
            id: imageB
            anchors.fill: parent
            asynchronous: true
            cache: true
            smooth: true
            fillMode: getImageFillMode()
            horizontalAlignment: Image.AlignHCenter
            verticalAlignment: Image.AlignVCenter
            opacity: 1
            z: activeImage === 1 ? 1 : 0
            
            transform: Translate {
                id: slideTransformB
                x: 0
            }
            scale: 1.0
            
            onStatusChanged: {
                if (status === Image.Ready && pendingNoTransition && activeImage === 0) {
                    // imageB 加载完成，执行无过渡切换
                    pendingNoTransition = false
                    imageB.opacity = 1
                    imageA.opacity = 0
                    activeImage = 1
                }
            }
        }
    }
    
    // 过渡动画 - A 淡入
    ParallelAnimation {
        id: transitionToA
        
        NumberAnimation {
            target: imageA; property: "opacity"; to: 1
            duration: backend.transitionDuration; easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: imageB; property: "opacity"; to: 0
            duration: backend.transitionDuration; easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            id: slideAnimationA; target: slideTransformA; property: "x"; to: 0
            duration: backend.transitionDuration; easing.type: Easing.InOutCubic
        }
        NumberAnimation {
            id: zoomAnimationA; target: imageA; property: "scale"; to: 1.0
            duration: backend.transitionDuration; easing.type: Easing.InOutQuad
        }
        
        onFinished: {
            activeImage = 0
            isTransitioning = false
            slideTransformB.x = 0
            imageB.scale = 1.0
        }
    }
    
    // 过渡动画 - B 淡入
    ParallelAnimation {
        id: transitionToB
        
        NumberAnimation {
            target: imageB; property: "opacity"; to: 1
            duration: backend.transitionDuration; easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: imageA; property: "opacity"; to: 0
            duration: backend.transitionDuration; easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            id: slideAnimationB; target: slideTransformB; property: "x"; to: 0
            duration: backend.transitionDuration; easing.type: Easing.InOutCubic
        }
        NumberAnimation {
            id: zoomAnimationB; target: imageB; property: "scale"; to: 1.0
            duration: backend.transitionDuration; easing.type: Easing.InOutQuad
        }
        
        onFinished: {
            activeImage = 1
            isTransitioning = false
            slideTransformA.x = 0
            imageA.scale = 1.0
        }
    }
    
    // 切换壁纸
    function switchWallpaper(newSource) {
        if (!newSource) return
        if (isTransitioning) return
        
        var newSourceUrl = "file://" + newSource
        var currentUrl = activeImage === 0 ? imageA.source.toString() : imageB.source.toString()
        if (newSourceUrl === currentUrl) return
        
        var targetImage = activeImage === 0 ? imageB : imageA
        var targetAnim = activeImage === 0 ? transitionToB : transitionToA
        var targetSlide = activeImage === 0 ? slideTransformB : slideTransformA
        var targetSlideAnim = activeImage === 0 ? slideAnimationB : slideAnimationA
        var targetZoomAnim = activeImage === 0 ? zoomAnimationB : zoomAnimationA
        
        // 首次加载
        if (imageA.source.toString() === "" && imageB.source.toString() === "") {
            imageA.source = newSourceUrl
            imageA.opacity = 1
            imageB.opacity = 0
            activeImage = 0
            return
        }
        
        targetImage.source = newSourceUrl
        
        switch (backend.effectiveTransition) {
            case "none":
                // 等待新图片加载完成再切换，避免闪烁
                if (targetImage.status === Image.Ready) {
                    // 已加载，直接切换
                    targetImage.opacity = 1
                    if (activeImage === 0) {
                        imageA.opacity = 0
                        activeImage = 1
                    } else {
                        imageB.opacity = 0
                        activeImage = 0
                    }
                } else {
                    // 等待加载完成
                    pendingNoTransition = true
                }
                return
                
            case "slide":
                targetSlide.x = root.width
                targetSlideAnim.from = root.width
                targetSlideAnim.to = 0
                break
                
            case "zoom":
                targetImage.scale = 0.8
                targetZoomAnim.from = 0.8
                targetZoomAnim.to = 1.0
                break
        }
        
        targetImage.opacity = 0
        isTransitioning = true
        targetAnim.start()
    }
    
    // 初始化
    Component.onCompleted: {
        console.log("========== Kabegame 壁纸插件已加载 ==========")
    }
}
