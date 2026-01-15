/*
 * Kabegame Wallpaper Plugin - C++ Backend
 * Copyright (C) 2024 Kabegame Team
 */

#ifndef WALLPAPERBACKEND_H
#define WALLPAPERBACKEND_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QDir>
#include <QFileSystemWatcher>
#include <QUrl>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class WallpaperBackend : public QObject
{
    Q_OBJECT
    
    // ========== 本地配置属性 ==========
    // 壁纸路径（可以是文件或文件夹）
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    
    // 是否为文件夹模式
    Q_PROPERTY(bool isFolder READ isFolder NOTIFY isFolderChanged)
    
    // 当前显示的壁纸
    Q_PROPERTY(QString currentWallpaper READ currentWallpaper NOTIFY currentWallpaperChanged)
    
    // 图片列表（文件夹模式）
    Q_PROPERTY(QStringList imageList READ imageList NOTIFY imageListChanged)
    
    // 图片数量
    Q_PROPERTY(int imageCount READ imageCount NOTIFY imageListChanged)
    
    // 轮播间隔（秒）
    Q_PROPERTY(int slideshowInterval READ slideshowInterval WRITE setSlideshowInterval NOTIFY slideshowIntervalChanged)
    
    // 轮播顺序 ("random" 或 "sequential")
    Q_PROPERTY(QString slideshowOrder READ slideshowOrder WRITE setSlideshowOrder NOTIFY slideshowOrderChanged)
    
    // 填充模式
    Q_PROPERTY(QString fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
    
    // 过渡效果
    Q_PROPERTY(QString transition READ transition WRITE setTransition NOTIFY transitionChanged)
    
    // 过渡时长
    Q_PROPERTY(int transitionDuration READ transitionDuration WRITE setTransitionDuration NOTIFY transitionDurationChanged)
    
    // ========== Kabegame Bridge (D-Bus) ==========
    // 是否启用 Kabegame Bridge
    Q_PROPERTY(bool bridgeEnabled READ bridgeEnabled WRITE setBridgeEnabled NOTIFY bridgeEnabledChanged)
    
    // 是否已连接到 Daemon
    Q_PROPERTY(bool bridgeConnected READ bridgeConnected NOTIFY bridgeConnectedChanged)
    
    // 有效的填充模式（Bridge 启用时使用 Daemon 设置）
    Q_PROPERTY(QString effectiveFillMode READ effectiveFillMode NOTIFY effectiveFillModeChanged)
    
    // 有效的过渡效果
    Q_PROPERTY(QString effectiveTransition READ effectiveTransition NOTIFY effectiveTransitionChanged)
    
    // 有效的轮播顺序
    Q_PROPERTY(QString effectiveSlideshowOrder READ effectiveSlideshowOrder NOTIFY effectiveSlideshowOrderChanged)

public:
    explicit WallpaperBackend(QObject *parent = nullptr);
    ~WallpaperBackend();
    
    // 本地属性 getter
    QString path() const { return m_path; }
    bool isFolder() const { return m_isFolder; }
    QString currentWallpaper() const { return m_currentWallpaper; }
    QStringList imageList() const { return m_imageList; }
    int imageCount() const { return m_imageList.count(); }
    int slideshowInterval() const { return m_slideshowInterval; }
    QString slideshowOrder() const { return m_slideshowOrder; }
    QString fillMode() const { return m_fillMode; }
    QString transition() const { return m_transition; }
    int transitionDuration() const { return m_transitionDuration; }
    
    // Bridge 属性 getter
    bool bridgeEnabled() const { return m_bridgeEnabled; }
    bool bridgeConnected() const { return m_bridgeConnected; }
    QString effectiveFillMode() const;
    QString effectiveTransition() const;
    QString effectiveSlideshowOrder() const;
    
    // 本地属性 setter
    void setPath(const QString &path);
    void setSlideshowInterval(int interval);
    void setSlideshowOrder(const QString &order);
    void setFillMode(const QString &mode);
    void setTransition(const QString &transition);
    void setTransitionDuration(int duration);
    
    // Bridge 属性 setter
    void setBridgeEnabled(bool enabled);

public Q_SLOTS:
    // 切换到下一张壁纸
    void nextWallpaper();
    
    // 切换到上一张壁纸
    void previousWallpaper();
    
    // 切换到指定索引的壁纸
    void setWallpaperByIndex(int index);
    
    // 刷新图片列表
    void refreshImageList();
    
    // 手动连接 Daemon
    void connectToDaemon();
    
    // 从 Daemon 同步设置
    void syncFromDaemon();

Q_SIGNALS:
    void pathChanged();
    void isFolderChanged();
    void currentWallpaperChanged();
    void imageListChanged();
    void slideshowIntervalChanged();
    void slideshowOrderChanged();
    void fillModeChanged();
    void transitionChanged();
    void transitionDurationChanged();
    
    // Bridge 信号
    void bridgeEnabledChanged();
    void bridgeConnectedChanged();
    void effectiveFillModeChanged();
    void effectiveTransitionChanged();
    void effectiveSlideshowOrderChanged();
    
    // 请求切换壁纸（带过渡效果）
    void wallpaperChangeRequested(const QString &newWallpaper);

private Q_SLOTS:
    void onSlideshowTimer();
    void onDirectoryChanged(const QString &path);
    void onBridgeCheckTimer();

private:
    void loadImageList();
    void updateIsFolder();
    void startSlideshow();
    void stopSlideshow();
    bool isImageFile(const QString &fileName) const;
    
    // D-Bus 相关
    void initDBus();
    QJsonObject sendDBusRequest(const QJsonObject &request);
    void loadAlbumImagesFromDaemon(const QString &albumId);
    void loadCurrentWallpaperFromDaemon(const QString &imageId);
    
    QString m_path;
    bool m_isFolder = false;
    QString m_currentWallpaper;
    QStringList m_imageList;
    int m_currentIndex = 0;
    
    int m_slideshowInterval = 60;  // 默认 60 秒
    QString m_slideshowOrder = "random";
    QString m_fillMode = "fill";
    QString m_transition = "fade";
    int m_transitionDuration = 500;
    
    QTimer *m_slideshowTimer = nullptr;
    QFileSystemWatcher *m_watcher = nullptr;
    
    // Bridge 相关
    bool m_bridgeEnabled = false;
    bool m_bridgeConnected = false;
    QString m_bridgeFillMode = "fill";
    QString m_bridgeTransition = "fade";
    int m_bridgeSlideshowInterval = 60;
    QString m_bridgeSlideshowOrder = "random";
    
    QDBusInterface *m_dbusInterface = nullptr;
    QTimer *m_bridgeCheckTimer = nullptr;
    
    static const QStringList s_imageExtensions;
};

#endif // WALLPAPERBACKEND_H
