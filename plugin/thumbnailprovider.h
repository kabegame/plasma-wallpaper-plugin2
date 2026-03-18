#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include <QHash>
#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>

class ThumbnailProvider : public QQuickImageProvider
{
public:
    ThumbnailProvider();
    ~ThumbnailProvider() override = default;

    QImage requestImage(
        const QString &id,
        QSize *size,
        const QSize &requestedSize
    ) override;

    static ThumbnailProvider *instance();
    static void setThumbnailPath(const QString &imageId, const QString &thumbnailPath);
    static void clear();

private:
    QHash<QString, QString> m_thumbPaths;
    QMutex m_mutex;
};

#endif // THUMBNAILPROVIDER_H
