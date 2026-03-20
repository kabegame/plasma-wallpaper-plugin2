#include "thumbnailprovider.h"

#include <QDebug>
#include <QFileInfo>
#include <QMutexLocker>

ThumbnailPathStore &ThumbnailPathStore::global()
{
    static ThumbnailPathStore store;
    return store;
}

QString ThumbnailPathStore::path(const QString &imageId) const
{
    QMutexLocker locker(&m_mutex);
    return m_paths.value(imageId);
}

void ThumbnailPathStore::insert(const QString &imageId, const QString &thumbnailPath)
{
    QMutexLocker locker(&m_mutex);
    m_paths.insert(imageId, thumbnailPath);
}

void ThumbnailPathStore::clear()
{
    QMutexLocker locker(&m_mutex);
    m_paths.clear();
}

ThumbnailProvider::ThumbnailProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage ThumbnailProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    const QString path = ThumbnailPathStore::global().path(id);

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

void ThumbnailProvider::setThumbnailPath(const QString &imageId, const QString &thumbnailPath)
{
    ThumbnailPathStore::global().insert(imageId, thumbnailPath);
}

void ThumbnailProvider::clearPaths()
{
    ThumbnailPathStore::global().clear();
}
