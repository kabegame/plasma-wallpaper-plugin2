import QtQuick
import QtMultimedia
import org.kde.plasma.plasmoid
import org.kabegame.wallpaper 1.0

WallpaperItem {
    id: root
    anchors.fill: parent

    // Plasma 6: configuration 由 WallpaperItem 提供；不再依赖全局 wallpaper
    readonly property var cfg: root.configuration
    readonly property string configPath: cfg ? (cfg.Image || "") : ""

    property bool isTransitioning: false
    property int activeLayer: 0 // 0=layerA, 1=layerB

    function isVideoPath(path) {
        if (!path || path.length === 0) {
            return false
        }
        var lower = path.toLowerCase()
        return lower.endsWith(".mp4")
            || lower.endsWith(".mkv")
            || lower.endsWith(".webm")
            || lower.endsWith(".avi")
            || lower.endsWith(".mov")
    }

    function imageFillModeFromStyle() {
        switch (backend.wallpaperStyle) {
            case "fit": return Image.PreserveAspectFit
            case "stretch": return Image.Stretch
            case "center": return Image.Pad
            case "tile": return Image.Tile
            case "fill":
            default:
                return Image.PreserveAspectCrop
        }
    }

    function videoFillModeFromStyle() {
        switch (backend.wallpaperStyle) {
            case "fit": return VideoOutput.PreserveAspectFit
            case "stretch": return VideoOutput.Stretch
            case "fill":
            default:
                return VideoOutput.PreserveAspectCrop
        }
    }

    function mediaSource(path) {
        if (!path || path.length === 0) return ""
        return backend.toFileUrl(path.startsWith("file://") ? path.replace("file://", "") : path)
    }

    function prepareLayer(layer, path) {
        layer.sourcePath = path
        layer.videoMode = isVideoPath(path)
    }

    function switchWallpaper(path) {
        if (!path || path.length === 0 || isTransitioning) {
            return
        }

        var currentLayer = activeLayer === 0 ? layerA : layerB
        if (currentLayer.sourcePath === path) {
            return
        }

        // 首次加载
        if (layerA.sourcePath.length === 0 && layerB.sourcePath.length === 0) {
            prepareLayer(layerA, path)
            layerA.opacity = 1
            layerB.opacity = 0
            activeLayer = 0
            return
        }

        var targetLayer = activeLayer === 0 ? layerB : layerA
        var targetAnim = activeLayer === 0 ? transitionToB : transitionToA
        var targetSlide = activeLayer === 0 ? slideTransformB : slideTransformA
        var targetSlideAnim = activeLayer === 0 ? slideAnimationB : slideAnimationA
        var targetZoomAnim = activeLayer === 0 ? zoomAnimationB : zoomAnimationA

        prepareLayer(targetLayer, path)

        if (backend.wallpaperTransition === "none") {
            targetLayer.opacity = 1
            currentLayer.opacity = 0
            activeLayer = activeLayer === 0 ? 1 : 0
            return
        }

        if (backend.wallpaperTransition === "slide") {
            targetSlide.x = root.width
            targetSlideAnim.from = root.width
            targetSlideAnim.to = 0
        } else if (backend.wallpaperTransition === "zoom") {
            targetLayer.scale = 0.8
            targetZoomAnim.from = 0.8
            targetZoomAnim.to = 1.0
        }

        targetLayer.opacity = 0
        isTransitioning = true
        targetAnim.start()
    }

    WallpaperBackend {
        id: backend

        onWallpaperChangeRequested: function(newWallpaper) {
            switchWallpaper(newWallpaper)
        }

        onImageConfigSyncRequested: function(path) {
            if (root.cfg && root.cfg.Image !== path) {
                root.cfg.Image = path
                if (typeof root.cfg.writeConfig === "function") {
                    root.cfg.writeConfig()
                }
            }
        }
    }

    // 显式在速率变更时设置播放器 playbackRate（绑定有时在 MediaPlayer 上不会实时生效）
    Connections {
        target: backend
        function onWallpaperVideoPlaybackRateChanged() {
            var rate = Math.max(backend.wallpaperVideoPlaybackRate, 0.01)
            playerA.playbackRate = rate
            playerB.playbackRate = rate
        }
    }

    onConfigPathChanged: switchWallpaper(configPath)

    Item {
        anchors.fill: parent
        clip: true

        Item {
            id: layerA
            anchors.fill: parent
            opacity: activeLayer === 0 ? 1 : 0
            z: activeLayer === 0 ? 1 : 0

            property string sourcePath: ""
            property bool videoMode: false

            transform: Translate {
                id: slideTransformA
                x: 0
            }
            scale: 1.0

            Image {
                anchors.fill: parent
                visible: !layerA.videoMode
                source: isVideoPath(layerA.sourcePath) ? "" : mediaSource(layerA.sourcePath)
                asynchronous: true
                cache: true
                smooth: true
                fillMode: imageFillModeFromStyle()
                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignVCenter
            }

            VideoOutput {
                id: videoOutputA
                anchors.fill: parent
                visible: layerA.videoMode
                fillMode: videoFillModeFromStyle()
            }

            AudioOutput {
                id: audioOutputA
                muted: backend.wallpaperVolume <= 0
                volume: layerA.opacity * backend.wallpaperVolume
            }

            MediaPlayer {
                id: playerA
                videoOutput: videoOutputA
                audioOutput: audioOutputA
                source: layerA.videoMode ? mediaSource(layerA.sourcePath) : ""
                loops: MediaPlayer.Infinite
                playbackRate: Math.max(backend.wallpaperVideoPlaybackRate, 0.01)
                onSourceChanged: {
                    if (source.toString().length > 0) {
                        playbackRate = Math.max(backend.wallpaperVideoPlaybackRate, 0.01)
                        play()
                    }
                }
            }
        }

        Item {
            id: layerB
            anchors.fill: parent
            opacity: activeLayer === 1 ? 1 : 0
            z: activeLayer === 1 ? 1 : 0

            property string sourcePath: ""
            property bool videoMode: false

            transform: Translate {
                id: slideTransformB
                x: 0
            }
            scale: 1.0

            Image {
                anchors.fill: parent
                visible: !layerB.videoMode
                source: isVideoPath(layerB.sourcePath) ? "" : mediaSource(layerB.sourcePath)
                asynchronous: true
                cache: true
                smooth: true
                fillMode: imageFillModeFromStyle()
                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignVCenter
            }

            VideoOutput {
                id: videoOutputB
                anchors.fill: parent
                visible: layerB.videoMode
                fillMode: videoFillModeFromStyle()
            }

            AudioOutput {
                id: audioOutputB
                muted: backend.wallpaperVolume <= 0
                volume: layerB.opacity * backend.wallpaperVolume
            }

            MediaPlayer {
                id: playerB
                videoOutput: videoOutputB
                audioOutput: audioOutputB
                source: layerB.videoMode ? mediaSource(layerB.sourcePath) : ""
                loops: MediaPlayer.Infinite
                playbackRate: Math.max(backend.wallpaperVideoPlaybackRate, 0.01)
                onSourceChanged: {
                    if (source.toString().length > 0) {
                        playbackRate = Math.max(backend.wallpaperVideoPlaybackRate, 0.01)
                        play()
                    }
                }
            }
        }
    }

    ParallelAnimation {
        id: transitionToA
        NumberAnimation {
            target: layerA; property: "opacity"; to: 1
            duration: 500; easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: layerB; property: "opacity"; to: 0
            duration: 500; easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            id: slideAnimationA; target: slideTransformA; property: "x"; to: 0
            duration: 500; easing.type: Easing.InOutCubic
        }
        NumberAnimation {
            id: zoomAnimationA; target: layerA; property: "scale"; to: 1.0
            duration: 500; easing.type: Easing.InOutQuad
        }
        onFinished: {
            activeLayer = 0
            isTransitioning = false
            slideTransformB.x = 0
            layerB.scale = 1.0
        }
    }

    ParallelAnimation {
        id: transitionToB
        NumberAnimation {
            target: layerB; property: "opacity"; to: 1
            duration: 500; easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: layerA; property: "opacity"; to: 0
            duration: 500; easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            id: slideAnimationB; target: slideTransformB; property: "x"; to: 0
            duration: 500; easing.type: Easing.InOutCubic
        }
        NumberAnimation {
            id: zoomAnimationB; target: layerB; property: "scale"; to: 1.0
            duration: 500; easing.type: Easing.InOutQuad
        }
        onFinished: {
            activeLayer = 1
            isTransitioning = false
            slideTransformA.x = 0
            layerA.scale = 1.0
        }
    }

    Component.onCompleted: {
        switchWallpaper(configPath)
    }
}
