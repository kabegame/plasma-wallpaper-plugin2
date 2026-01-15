/*
 * Kabegame Wallpaper Plugin - Main QML (C++ Backend Version)
 * Copyright (C) 2024 Kabegame Team
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kabegame.wallpaper 1.0

Item {
    id: root
    
    // 配置属性
    readonly property string configPath: wallpaper.configuration.Image || ""
    readonly property string fillMode: wallpaper.configuration.FillMode || "fill"
    readonly property string transition: wallpaper.configuration.Transition || "fade"
    readonly property int transitionDuration: wallpaper.configuration.TransitionDuration || 500
    readonly property int slideshowInterval: wallpaper.configuration.SlideshowInterval || 60
    readonly property string slideshowOrder: wallpaper.configuration.SlideshowOrder || "random"
    
    // C++ 后端
    WallpaperBackend {
        id: backend
        
        // 绑定配置
        path: configPath
        slideshowInterval: root.slideshowInterval
        slideshowOrder: root.slideshowOrder
        fillMode: root.fillMode
        transition: root.transition
        transitionDuration: root.transitionDuration
        
        // 壁纸切换请求
        onWallpaperChangeRequested: function(newWallpaper) {
            console.log("[QML] 收到壁纸切换请求:", newWallpaper)
            switchWallpaper(newWallpaper)
        }
    }
    
    // 当前显示的壁纸源
    property string currentImageSource: ""
    
    // 双图层用于过渡
    Item {
        id: wallpaperContainer
        anchors.fill: parent
        clip: true
        
        // 底层图片（当前显示的）
        Image {
            id: baseImage
            anchors.fill: parent
            source: currentImageSource ? "file://" + currentImageSource : ""
            asynchronous: true
            cache: false
            smooth: true
            
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
            
            onStatusChanged: {
                if (status === Image.Error) {
                    console.log("[QML] baseImage 加载失败:", source)
                } else if (status === Image.Ready) {
                    console.log("[QML] baseImage 加载成功")
                }
            }
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
            
            fillMode: baseImage.fillMode
            horizontalAlignment: Image.AlignHCenter
            verticalAlignment: Image.AlignVCenter
            
            transform: Translate {
                id: slideTransform
                x: 0
            }
            
            scale: 1.0
        }
    }
    
    // 过渡动画
    ParallelAnimation {
        id: transitionAnimation
        
        NumberAnimation {
            target: topImage
            property: "opacity"
            from: 0
            to: 1
            duration: root.transitionDuration
            easing.type: Easing.InOutQuad
        }
        
        NumberAnimation {
            id: slideAnimation
            target: slideTransform
            property: "x"
            duration: root.transitionDuration
            easing.type: Easing.InOutCubic
            running: false
        }
        
        NumberAnimation {
            id: zoomAnimation
            target: topImage
            property: "scale"
            duration: root.transitionDuration
            easing.type: Easing.InOutQuad
            running: false
        }
        
        onFinished: {
            baseImage.source = topImage.source
            currentImageSource = topImage.source.toString().replace("file://", "")
            
            topImage.opacity = 0
            topImage.scale = 1.0
            slideTransform.x = 0
        }
    }
    
    // 切换壁纸函数
    function switchWallpaper(newSource) {
        if (!newSource) {
            return
        }
        
        var newSourceUrl = "file://" + newSource
        
        if (newSourceUrl === baseImage.source.toString()) {
            return
        }
        
        console.log("[QML] 切换壁纸:", newSource)
        
        // 首次加载直接设置
        if (!currentImageSource) {
            currentImageSource = newSource
            baseImage.source = newSourceUrl
            return
        }
        
        // 准备过渡
        topImage.source = newSourceUrl
        
        switch (root.transition) {
            case "none":
                currentImageSource = newSource
                baseImage.source = newSourceUrl
                break
                
            case "fade":
                slideAnimation.running = false
                zoomAnimation.running = false
                transitionAnimation.start()
                break
                
            case "slide":
                slideAnimation.running = true
                zoomAnimation.running = false
                slideTransform.x = root.width
                slideAnimation.from = root.width
                slideAnimation.to = 0
                transitionAnimation.start()
                break
                
            case "zoom":
                slideAnimation.running = false
                zoomAnimation.running = true
                topImage.scale = 0.8
                zoomAnimation.from = 0.8
                zoomAnimation.to = 1.0
                transitionAnimation.start()
                break
                
            default:
                slideAnimation.running = false
                zoomAnimation.running = false
                transitionAnimation.start()
        }
    }
    
    // 初始化
    Component.onCompleted: {
        console.log("[QML] Kabegame 壁纸插件已加载 (C++ 后端版本)")
        console.log("[QML] 路径:", configPath)
        console.log("[QML] 填充模式:", fillMode)
        console.log("[QML] 过渡效果:", transition)
        console.log("[QML] 轮播间隔:", slideshowInterval, "秒")
    }
}
