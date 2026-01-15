/*
 * Kabegame Wallpaper Plugin - C++ Backend Implementation
 * Copyright (C) 2024 Kabegame Team
 */

#include "wallpaperbackend.h"

#include <QDebug>
#include <QFileInfo>
#include <QRandomGenerator>
#include <algorithm>

// 支持的图片扩展名
const QStringList WallpaperBackend::s_imageExtensions = {
    "png", "jpg", "jpeg", "bmp", "gif", "webp", "svg",
    "PNG", "JPG", "JPEG", "BMP", "GIF", "WEBP", "SVG"
};

WallpaperBackend::WallpaperBackend(QObject *parent)
    : QObject(parent)
    , m_slideshowTimer(new QTimer(this))
    , m_watcher(new QFileSystemWatcher(this))
{
    qDebug() << "[Kabegame] WallpaperBackend 初始化";
    
    // 连接定时器
    connect(m_slideshowTimer, &QTimer::timeout, this, &WallpaperBackend::onSlideshowTimer);
    
    // 连接文件系统监视器
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &WallpaperBackend::onDirectoryChanged);
}

WallpaperBackend::~WallpaperBackend()
{
    qDebug() << "[Kabegame] WallpaperBackend 销毁";
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
    
    // 更新定时器间隔
    if (m_slideshowTimer->isActive()) {
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
}

void WallpaperBackend::setFillMode(const QString &mode)
{
    if (m_fillMode == mode) {
        return;
    }
    
    m_fillMode = mode;
    Q_EMIT fillModeChanged();
}

void WallpaperBackend::setTransition(const QString &transition)
{
    if (m_transition == transition) {
        return;
    }
    
    m_transition = transition;
    Q_EMIT transitionChanged();
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
        qDebug() << "[Kabegame] nextWallpaper: 图片列表为空";
        return;
    }
    
    if (m_imageList.size() == 1) {
        qDebug() << "[Kabegame] nextWallpaper: 只有一张图片";
        return;
    }
    
    int newIndex;
    
    if (m_slideshowOrder == "random") {
        // 随机选择一张不同的图片
        int attempts = 0;
        do {
            newIndex = QRandomGenerator::global()->bounded(m_imageList.size());
            attempts++;
        } while (newIndex == m_currentIndex && attempts < 10);
        
        qDebug() << "[Kabegame] 随机选择索引:" << newIndex;
    } else {
        newIndex = (m_currentIndex + 1) % m_imageList.size();
        qDebug() << "[Kabegame] 顺序选择索引:" << newIndex;
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
        qDebug() << "[Kabegame] 无效索引:" << index;
        return;
    }
    
    m_currentIndex = index;
    m_currentWallpaper = m_imageList.at(index);
    
    qDebug() << "[Kabegame] 切换壁纸:" << m_currentWallpaper;
    
    Q_EMIT currentWallpaperChanged();
    Q_EMIT wallpaperChangeRequested(m_currentWallpaper);
}

void WallpaperBackend::refreshImageList()
{
    if (m_isFolder) {
        loadImageList();
    }
}

void WallpaperBackend::onSlideshowTimer()
{
    qDebug() << "[Kabegame] 定时器触发";
    nextWallpaper();
}

void WallpaperBackend::onDirectoryChanged(const QString &path)
{
    Q_UNUSED(path)
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
    
    qDebug() << "[Kabegame] 路径类型:" << (m_isFolder ? "文件夹" : "单文件");
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
    
    qDebug() << "[Kabegame] 加载文件夹:" << m_path;
    
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
    
    // 打印前几张图片路径（调试用）
    for (int i = 0; i < qMin(5, newList.size()); i++) {
        qDebug() << "[Kabegame]   -" << newList.at(i);
    }
    
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
    
    // 启动轮播
    if (m_imageList.size() > 1) {
        startSlideshow();
    } else {
        stopSlideshow();
    }
}

void WallpaperBackend::startSlideshow()
{
    if (m_slideshowTimer->isActive()) {
        return;
    }
    
    qDebug() << "[Kabegame] 启动轮播定时器，间隔:" << m_slideshowInterval << "秒";
    m_slideshowTimer->setInterval(m_slideshowInterval * 1000);
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
