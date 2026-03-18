#include "thumbnailprovider.h"

#include <QDebug>
#include <QFileInfo>
#include <QMutexLocker>

ThumbnailProvider::ThumbnailProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage ThumbnailProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString path;
    {
        QMutexLocker locker(&m_mutex);
        path = m_thumbPaths.value(id);
    }

    if (path.isEmpty() || !QFileInfo::exists(path)) {
        QImage fallback(requestedSize.isValid() ? requestedSize : QSize(320, 200), QImage::Format_ARGB32_Premultiplied);
        fallback.fill(QColor(45, 45, 45));
        if (size) {
            *size = fallback.size();
        }
        return fallback;
    }

    QImage image(path);
    if (image.isNull()) {
        if (size) {
            *size = QSize(0, 0);
        }
        return image;
    }

    if (requestedSize.isValid()) {
        image = image.scaled(requestedSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }

    if (size) {
        *size = image.size();
    }
    return image;
}

ThumbnailProvider *ThumbnailProvider::instance()
{
    static ThumbnailProvider *provider = new ThumbnailProvider();
    return provider;
}

void ThumbnailProvider::setThumbnailPath(const QString &imageId, const QString &thumbnailPath)
{
    ThumbnailProvider *provider = instance();
    QMutexLocker locker(&provider->m_mutex);
    provider->m_thumbPaths.insert(imageId, thumbnailPath);
}

void ThumbnailProvider::clear()
{
    ThumbnailProvider *provider = instance();
    QMutexLocker locker(&provider->m_mutex);
    provider->m_thumbPaths.clear();
}
