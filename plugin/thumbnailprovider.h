#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include <QHash>
#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>

class ThumbnailPathStore
{
public:
    static ThumbnailPathStore &global();

    QString path(const QString &imageId) const;
    void insert(const QString &imageId, const QString &thumbnailPath);
    void clear();

private:
    mutable QMutex m_mutex;
    QHash<QString, QString> m_paths;
};

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

    static void setThumbnailPath(const QString &imageId, const QString &thumbnailPath);
    static void clearPaths();
};

#endif // THUMBNAILPROVIDER_H
