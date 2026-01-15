/*
 * Kabegame Wallpaper Plugin - C++ Backend Implementation
 * Copyright (C) 2024 Kabegame Team
 */

#include "wallpaperbackend.h"

#include <QDebug>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusMessage>
#include <algorithm>

// D-Bus 配置
static const QString DBUS_SERVICE = "org.kabegame.Daemon";
static const QString DBUS_PATH = "/org/kabegame/Daemon";
static const QString DBUS_INTERFACE = "org.kabegame.Daemon";

// 支持的图片扩展名
const QStringList WallpaperBackend::s_imageExtensions = {
    "png", "jpg", "jpeg", "bmp", "gif", "webp", "svg",
    "PNG", "JPG", "JPEG", "BMP", "GIF", "WEBP", "SVG"
};

WallpaperBackend::WallpaperBackend(QObject *parent)
    : QObject(parent)
    , m_slideshowTimer(new QTimer(this))
    , m_watcher(new QFileSystemWatcher(this))
    , m_bridgeCheckTimer(new QTimer(this))
{
    qDebug() << "[Kabegame] WallpaperBackend 初始化";
    
    // 连接定时器
    connect(m_slideshowTimer, &QTimer::timeout, this, &WallpaperBackend::onSlideshowTimer);
    
    // 连接文件系统监视器
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &WallpaperBackend::onDirectoryChanged);
    
    // Bridge 连接检查定时器
    connect(m_bridgeCheckTimer, &QTimer::timeout, this, &WallpaperBackend::onBridgeCheckTimer);
    m_bridgeCheckTimer->setInterval(5000);
    
    // 初始化 D-Bus
    initDBus();
}

WallpaperBackend::~WallpaperBackend()
{
    qDebug() << "[Kabegame] WallpaperBackend 销毁";
    
    if (m_dbusInterface) {
        delete m_dbusInterface;
        m_dbusInterface = nullptr;
    }
}

void WallpaperBackend::initDBus()
{
    m_dbusInterface = new QDBusInterface(
        DBUS_SERVICE,
        DBUS_PATH,
        DBUS_INTERFACE,
        QDBusConnection::sessionBus(),
        this
    );
    
    if (m_dbusInterface->isValid()) {
        qDebug() << "[Kabegame] D-Bus 接口已创建";
    } else {
        qDebug() << "[Kabegame] D-Bus 接口创建失败（Daemon 可能未运行）";
    }
}

QJsonObject WallpaperBackend::sendDBusRequest(const QJsonObject &request)
{
    if (!m_dbusInterface || !m_dbusInterface->isValid()) {
        // 尝试重新连接
        if (m_dbusInterface) {
            delete m_dbusInterface;
        }
        m_dbusInterface = new QDBusInterface(
            DBUS_SERVICE,
            DBUS_PATH,
            DBUS_INTERFACE,
            QDBusConnection::sessionBus(),
            this
        );
        
        if (!m_dbusInterface->isValid()) {
            return QJsonObject{{"ok", false}, {"message", "D-Bus interface not available"}};
        }
    }
    
    // 序列化请求为 JSON 字符串
    QJsonDocument doc(request);
    QString jsonStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    
    // 调用 D-Bus Request 方法
    QDBusReply<QString> reply = m_dbusInterface->call("Request", jsonStr);
    
    if (!reply.isValid()) {
        qDebug() << "[Kabegame] D-Bus 调用失败:" << reply.error().message();
        return QJsonObject{{"ok", false}, {"message", reply.error().message()}};
    }
    
    // 解析响应
    QString responseStr = reply.value();
    QJsonParseError parseError;
    QJsonDocument responseDoc = QJsonDocument::fromJson(responseStr.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "[Kabegame] JSON 解析失败:" << parseError.errorString();
        return QJsonObject{{"ok", false}, {"message", parseError.errorString()}};
    }
    
    return responseDoc.object();
}

void WallpaperBackend::connectToDaemon()
{
    qDebug() << "[Kabegame] 尝试连接到 Daemon...";
    
    // 发送 status 请求探活
    QJsonObject request{{"cmd", "status"}};
    QJsonObject response = sendDBusRequest(request);
    
    bool wasConnected = m_bridgeConnected;
    m_bridgeConnected = response["ok"].toBool(false);
    
    if (m_bridgeConnected) {
        QJsonObject info = response["info"].toObject();
        QString version = info["version"].toString("?");
        qDebug() << "[Kabegame] 已连接到 Daemon v" << version;
        
        // 连接成功，同步设置
        syncFromDaemon();
    } else {
        qDebug() << "[Kabegame] 无法连接到 Daemon:" << response["message"].toString();
    }
    
    if (wasConnected != m_bridgeConnected) {
        Q_EMIT bridgeConnectedChanged();
    }
}

void WallpaperBackend::syncFromDaemon()
{
    if (!m_bridgeConnected) {
        return;
    }
    
    qDebug() << "[Kabegame] 从 Daemon 同步设置...";
    
    // 获取所有设置
    QJsonObject request{{"cmd", "settingsGet"}};
    QJsonObject response = sendDBusRequest(request);
    
    if (!response["ok"].toBool(false)) {
        qDebug() << "[Kabegame] 获取设置失败:" << response["message"].toString();
        return;
    }
    
    QJsonObject data = response["data"].toObject();
    
    // 同步填充模式
    QString newFillMode = data["wallpaperRotationStyle"].toString("fill");
    if (m_bridgeFillMode != newFillMode) {
        m_bridgeFillMode = newFillMode;
        Q_EMIT effectiveFillModeChanged();
        qDebug() << "[Kabegame] 同步填充模式:" << m_bridgeFillMode;
    }
    
    // 同步过渡效果
    QString newTransition = data["wallpaperRotationTransition"].toString("fade");
    if (m_bridgeTransition != newTransition) {
        m_bridgeTransition = newTransition;
        Q_EMIT effectiveTransitionChanged();
        qDebug() << "[Kabegame] 同步过渡效果:" << m_bridgeTransition;
    }
    
    // 同步轮播间隔（分钟转秒）
    int newInterval = data["wallpaperRotationIntervalMinutes"].toInt(60) * 60;
    if (m_bridgeSlideshowInterval != newInterval) {
        m_bridgeSlideshowInterval = newInterval;
        qDebug() << "[Kabegame] 同步轮播间隔:" << m_bridgeSlideshowInterval << "秒";
        
        // 如果正在使用 Bridge 轮播，更新定时器
        if (m_bridgeEnabled && m_slideshowTimer->isActive()) {
            m_slideshowTimer->setInterval(m_bridgeSlideshowInterval * 1000);
        }
    }
    
    // 同步轮播顺序
    QString newOrder = data["wallpaperRotationMode"].toString("random");
    if (m_bridgeSlideshowOrder != newOrder) {
        m_bridgeSlideshowOrder = newOrder;
        Q_EMIT effectiveSlideshowOrderChanged();
        qDebug() << "[Kabegame] 同步轮播顺序:" << m_bridgeSlideshowOrder;
    }
    
    // 获取当前壁纸
    QString currentImageId = data["currentWallpaperImageId"].toString();
    if (!currentImageId.isEmpty()) {
        loadCurrentWallpaperFromDaemon(currentImageId);
    }
    
    // 获取轮播画册图片
    QString albumId = data["wallpaperRotationAlbumId"].toString();
    loadAlbumImagesFromDaemon(albumId);
}

void WallpaperBackend::loadCurrentWallpaperFromDaemon(const QString &imageId)
{
    QJsonObject request{{"cmd", "storageGetImageById"}, {"imageId", imageId}};
    QJsonObject response = sendDBusRequest(request);
    
    if (response["ok"].toBool(false)) {
        QJsonObject data = response["data"].toObject();
        QString localPath = data["localPath"].toString();
        if (!localPath.isEmpty()) {
            qDebug() << "[Kabegame] 从 Daemon 获取当前壁纸:" << localPath;
            m_currentWallpaper = localPath;
            Q_EMIT currentWallpaperChanged();
            Q_EMIT wallpaperChangeRequested(m_currentWallpaper);
        }
    }
}

void WallpaperBackend::loadAlbumImagesFromDaemon(const QString &albumId)
{
    QJsonObject request;
    if (albumId.isEmpty()) {
        // 全画廊
        request = QJsonObject{{"cmd", "storageGetImages"}};
    } else {
        request = QJsonObject{{"cmd", "storageGetAlbumImages"}, {"albumId", albumId}};
    }
    
    QJsonObject response = sendDBusRequest(request);
    
    if (!response["ok"].toBool(false)) {
        qDebug() << "[Kabegame] 获取画册图片失败:" << response["message"].toString();
        return;
    }
    
    QJsonObject data = response["data"].toObject();
    QJsonArray imagesArray = data["images"].toArray();
    if (imagesArray.isEmpty()) {
        // 尝试直接从 data 获取（storageGetImages 返回格式可能不同）
        imagesArray = response["data"].toArray();
    }
    
    QStringList paths;
    for (const QJsonValue &val : imagesArray) {
        QJsonObject img = val.toObject();
        QString path = img["localPath"].toString();
        if (!path.isEmpty()) {
            paths.append(path);
        }
    }
    
    if (!paths.isEmpty()) {
        qDebug() << "[Kabegame] 从 Daemon 加载" << paths.size() << "张画册图片";
        m_imageList = paths;
        Q_EMIT imageListChanged();
        
        // 启动轮播
        if (paths.size() > 1) {
            startSlideshow();
        }
    }
}

void WallpaperBackend::setBridgeEnabled(bool enabled)
{
    if (m_bridgeEnabled == enabled) {
        return;
    }
    
    qDebug() << "[Kabegame] Bridge 启用状态:" << enabled;
    
    m_bridgeEnabled = enabled;
    Q_EMIT bridgeEnabledChanged();
    
    if (enabled) {
        // 启用 Bridge
        stopSlideshow();  // 停止本地轮播
        connectToDaemon();
        m_bridgeCheckTimer->start();  // 启动连接检查
    } else {
        // 禁用 Bridge
        m_bridgeCheckTimer->stop();
        m_bridgeConnected = false;
        Q_EMIT bridgeConnectedChanged();
        
        // 恢复本地模式
        Q_EMIT effectiveFillModeChanged();
        Q_EMIT effectiveTransitionChanged();
        Q_EMIT effectiveSlideshowOrderChanged();
        
        if (m_isFolder && m_imageList.size() > 1) {
            startSlideshow();
        }
    }
}

void WallpaperBackend::onBridgeCheckTimer()
{
    if (!m_bridgeConnected) {
        connectToDaemon();
    }
}

QString WallpaperBackend::effectiveFillMode() const
{
    return (m_bridgeEnabled && m_bridgeConnected) ? m_bridgeFillMode : m_fillMode;
}

QString WallpaperBackend::effectiveTransition() const
{
    return (m_bridgeEnabled && m_bridgeConnected) ? m_bridgeTransition : m_transition;
}

QString WallpaperBackend::effectiveSlideshowOrder() const
{
    return (m_bridgeEnabled && m_bridgeConnected) ? m_bridgeSlideshowOrder : m_slideshowOrder;
}

void WallpaperBackend::setPath(const QString &path)
{
    QString cleanPath = path;
    
    // 移除 file:// 前缀
    if (cleanPath.startsWith("file://")) {
        cleanPath = cleanPath.mid(7);
    }
    
    if (m_path == cleanPath) {
        return;
    }
    
    qDebug() << "[Kabegame] 设置路径:" << cleanPath;
    
    // 移除旧路径的监视
    if (!m_path.isEmpty() && m_isFolder) {
        m_watcher->removePath(m_path);
    }
    
    m_path = cleanPath;
    Q_EMIT pathChanged();
    
    updateIsFolder();
    
    // 如果启用了 Bridge 且已连接，不使用本地文件列表
    if (m_bridgeEnabled && m_bridgeConnected) {
        return;
    }
    
    if (m_isFolder) {
        loadImageList();
        // 添加文件夹监视
        m_watcher->addPath(m_path);
    } else {
        // 单图模式
        m_imageList.clear();
        Q_EMIT imageListChanged();
        
        if (!m_path.isEmpty()) {
            m_currentWallpaper = m_path;
            Q_EMIT currentWallpaperChanged();
            Q_EMIT wallpaperChangeRequested(m_currentWallpaper);
        }
        
        stopSlideshow();
    }
}

void WallpaperBackend::setSlideshowInterval(int interval)
{
    if (interval < 1) interval = 1;
    if (interval > 86400) interval = 86400;  // 最大一天
    
    if (m_slideshowInterval == interval) {
        return;
    }
    
    qDebug() << "[Kabegame] 设置轮播间隔:" << interval << "秒";
    
    m_slideshowInterval = interval;
    Q_EMIT slideshowIntervalChanged();
    
    // 更新定时器间隔（仅当不使用 Bridge 时）
    if (!m_bridgeEnabled && m_slideshowTimer->isActive()) {
        m_slideshowTimer->setInterval(m_slideshowInterval * 1000);
    }
}

void WallpaperBackend::setSlideshowOrder(const QString &order)
{
    if (m_slideshowOrder == order) {
        return;
    }
    
    qDebug() << "[Kabegame] 设置轮播顺序:" << order;
    
    m_slideshowOrder = order;
    Q_EMIT slideshowOrderChanged();
    Q_EMIT effectiveSlideshowOrderChanged();
}

void WallpaperBackend::setFillMode(const QString &mode)
{
    if (m_fillMode == mode) {
        return;
    }
    
    m_fillMode = mode;
    Q_EMIT fillModeChanged();
    Q_EMIT effectiveFillModeChanged();
}

void WallpaperBackend::setTransition(const QString &transition)
{
    if (m_transition == transition) {
        return;
    }
    
    m_transition = transition;
    Q_EMIT transitionChanged();
    Q_EMIT effectiveTransitionChanged();
}

void WallpaperBackend::setTransitionDuration(int duration)
{
    if (duration < 0) duration = 0;
    if (duration > 10000) duration = 10000;
    
    if (m_transitionDuration == duration) {
        return;
    }
    
    m_transitionDuration = duration;
    Q_EMIT transitionDurationChanged();
}

void WallpaperBackend::nextWallpaper()
{
    if (m_imageList.isEmpty()) {
        return;
    }
    
    if (m_imageList.size() == 1) {
        return;
    }
    
    int newIndex;
    QString order = effectiveSlideshowOrder();
    
    if (order == "random") {
        // 随机选择一张不同的图片
        int attempts = 0;
        do {
            newIndex = QRandomGenerator::global()->bounded(m_imageList.size());
            attempts++;
        } while (newIndex == m_currentIndex && attempts < 10);
    } else {
        newIndex = (m_currentIndex + 1) % m_imageList.size();
    }
    
    setWallpaperByIndex(newIndex);
}

void WallpaperBackend::previousWallpaper()
{
    if (m_imageList.isEmpty() || m_imageList.size() == 1) {
        return;
    }
    
    int newIndex = m_currentIndex - 1;
    if (newIndex < 0) {
        newIndex = m_imageList.size() - 1;
    }
    
    setWallpaperByIndex(newIndex);
}

void WallpaperBackend::setWallpaperByIndex(int index)
{
    if (index < 0 || index >= m_imageList.size()) {
        return;
    }
    
    m_currentIndex = index;
    m_currentWallpaper = m_imageList.at(index);
    
    Q_EMIT currentWallpaperChanged();
    Q_EMIT wallpaperChangeRequested(m_currentWallpaper);
}

void WallpaperBackend::refreshImageList()
{
    if (m_bridgeEnabled && m_bridgeConnected) {
        syncFromDaemon();
    } else if (m_isFolder) {
        loadImageList();
    }
}

void WallpaperBackend::onSlideshowTimer()
{
    nextWallpaper();
}

void WallpaperBackend::onDirectoryChanged(const QString &path)
{
    Q_UNUSED(path)
    
    // 如果使用 Bridge，不监视本地文件夹变化
    if (m_bridgeEnabled && m_bridgeConnected) {
        return;
    }
    
    qDebug() << "[Kabegame] 文件夹内容变化，刷新图片列表";
    loadImageList();
}

void WallpaperBackend::updateIsFolder()
{
    if (m_path.isEmpty()) {
        m_isFolder = false;
        Q_EMIT isFolderChanged();
        return;
    }
    
    QFileInfo fileInfo(m_path);
    
    if (fileInfo.isDir()) {
        m_isFolder = true;
    } else if (fileInfo.isFile()) {
        m_isFolder = false;
    } else {
        // 路径不存在，根据扩展名判断
        QString suffix = fileInfo.suffix().toLower();
        m_isFolder = !s_imageExtensions.contains(suffix, Qt::CaseInsensitive);
    }
    
    Q_EMIT isFolderChanged();
}

void WallpaperBackend::loadImageList()
{
    if (!m_isFolder || m_path.isEmpty()) {
        return;
    }
    
    QDir dir(m_path);
    if (!dir.exists()) {
        qDebug() << "[Kabegame] 文件夹不存在:" << m_path;
        m_imageList.clear();
        Q_EMIT imageListChanged();
        stopSlideshow();
        return;
    }
    
    // 构建过滤器
    QStringList filters;
    for (const QString &ext : s_imageExtensions) {
        filters << ("*." + ext);
    }
    
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Readable);
    dir.setSorting(QDir::Name);
    
    QStringList newList;
    const QFileInfoList entries = dir.entryInfoList();
    
    for (const QFileInfo &entry : entries) {
        newList.append(entry.absoluteFilePath());
    }
    
    qDebug() << "[Kabegame] 找到" << newList.size() << "张图片";
    
    m_imageList = newList;
    Q_EMIT imageListChanged();
    
    if (m_imageList.isEmpty()) {
        stopSlideshow();
        return;
    }
    
    // 显示第一张图片
    if (m_currentWallpaper.isEmpty() || !m_imageList.contains(m_currentWallpaper)) {
        if (m_slideshowOrder == "random") {
            m_currentIndex = QRandomGenerator::global()->bounded(m_imageList.size());
        } else {
            m_currentIndex = 0;
        }
        m_currentWallpaper = m_imageList.at(m_currentIndex);
        Q_EMIT currentWallpaperChanged();
        Q_EMIT wallpaperChangeRequested(m_currentWallpaper);
    }
    
    // 启动轮播（仅当不使用 Bridge 时）
    if (!m_bridgeEnabled && m_imageList.size() > 1) {
        startSlideshow();
    }
}

void WallpaperBackend::startSlideshow()
{
    if (m_slideshowTimer->isActive()) {
        return;
    }
    
    int interval = (m_bridgeEnabled && m_bridgeConnected) 
        ? m_bridgeSlideshowInterval 
        : m_slideshowInterval;
    
    qDebug() << "[Kabegame] 启动轮播定时器，间隔:" << interval << "秒";
    m_slideshowTimer->setInterval(interval * 1000);
    m_slideshowTimer->start();
}

void WallpaperBackend::stopSlideshow()
{
    if (!m_slideshowTimer->isActive()) {
        return;
    }
    
    qDebug() << "[Kabegame] 停止轮播定时器";
    m_slideshowTimer->stop();
}

bool WallpaperBackend::isImageFile(const QString &fileName) const
{
    QFileInfo fileInfo(fileName);
    return s_imageExtensions.contains(fileInfo.suffix(), Qt::CaseInsensitive);
}
