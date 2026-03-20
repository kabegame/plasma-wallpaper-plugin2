#include "wallpaperbackend.h"

#include "thumbnailprovider.h"

#include <KLocalizedString>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

WallpaperBackend::WallpaperBackend(QObject *parent)
    : QObject(parent)
    , m_ipc(new IpcClient(this))
{
    connect(m_ipc, &IpcClient::connectedChanged, this, &WallpaperBackend::onIpcConnectedChanged);
    connect(m_ipc, &IpcClient::eventReceived, this, &WallpaperBackend::onIpcEventReceived);

    connectToKabegame();
}

void WallpaperBackend::connectToKabegame()
{
    m_ipc->connectToDaemon();
}

void WallpaperBackend::openKabegame()
{
    QCborMap request;
    request.insert(QStringLiteral("cmd"), QStringLiteral("app-show-window"));
    m_ipc->sendRequest(request, [this](const QCborMap &response, const QString &error) {
        if (!error.isEmpty()) {
            const bool started = QProcess::startDetached(QStringLiteral("kabegame"), QStringList());
            if (!started) {
                Q_EMIT hintMessage(i18n("Cannot connect to Kabegame, and failed to start the application."));
            }
            return;
        }

        if (!response.value(QStringLiteral("ok")).toBool()) {
            const bool started = QProcess::startDetached(QStringLiteral("kabegame"), QStringList());
            if (!started) {
                Q_EMIT hintMessage(i18n("Failed to bring window to front, and failed to start Kabegame."));
            }
        }
    });
}

void WallpaperBackend::loadGalleryPage(int page)
{
    if (page < 1) {
        page = 1;
    }

    QString path;
    if (m_providerRootPath == QLatin1String("all")) {
        path = m_sortDescending ? QStringLiteral("all/desc/%1").arg(page) : QStringLiteral("all/%1").arg(page);
    } else {
        path = QStringLiteral("%1/%2").arg(m_providerRootPath, QString::number(page));
    }

    const QString pathForCallback = path;

    QCborMap request;
    request.insert(QStringLiteral("cmd"), QStringLiteral("gallery-browse-provider"));
    request.insert(QStringLiteral("path"), path);

    m_ipc->sendRequest(request, [this, page, pathForCallback](const QCborMap &response, const QString &error) {
        if (!error.isEmpty()) {
            Q_EMIT hintMessage(i18n("Failed to load gallery: %1", error));
            return;
        }
        if (!response.value(QStringLiteral("ok")).toBool()) {
            Q_EMIT hintMessage(i18n("Failed to load gallery: %1", cborToString(response.value(QStringLiteral("message")))));
            return;
        }

        const QVariantMap dataMap = cborToVariantMap(response.value(QStringLiteral("data")));
        const QVariantList entries = dataMap.value(QStringLiteral("entries")).toList();
        const int total = dataMap.value(QStringLiteral("total")).toInt();

        if (total == 0 && !pathForCallback.startsWith(QLatin1String("all"))) {
            setProviderRootPath(QStringLiteral("all"));
            return;
        }

        QVariantList images;
        for (const QVariant &entryVar : entries) {
            const QVariantMap entry = entryVar.toMap();
            if (entry.value(QStringLiteral("kind")).toString() != QLatin1String("image")) {
                continue;
            }

            const QVariantMap image = entry.value(QStringLiteral("image")).toMap();
            const QString id = image.value(QStringLiteral("id")).toString();
            const QString localPath = image.value(QStringLiteral("localPath")).toString();
            const QString thumbnailPath = image.value(QStringLiteral("thumbnailPath")).toString();
            if (id.isEmpty()) {
                continue;
            }

            QVariantMap item;
            item.insert(QStringLiteral("id"), id);
            item.insert(QStringLiteral("localPath"), localPath);
            item.insert(QStringLiteral("thumbnailPath"), thumbnailPath);
            images.append(item);

            if (!thumbnailPath.isEmpty()) {
                ThumbnailProvider::setThumbnailPath(id, thumbnailPath);
            } else if (!localPath.isEmpty()) {
                ThumbnailProvider::setThumbnailPath(id, localPath);
            }
        }

        m_galleryImages = images;
        m_galleryTotal = total;
        m_galleryPage = page;

        Q_EMIT galleryImagesChanged();
        Q_EMIT galleryTotalChanged();
        Q_EMIT galleryPageChanged();
    });
}

void WallpaperBackend::loadAlbums()
{
    QCborMap request;
    request.insert(QStringLiteral("cmd"), QStringLiteral("storage-get-albums"));

    m_ipc->sendRequest(request, [this](const QCborMap &response, const QString &error) {
        if (!error.isEmpty() || !response.value(QStringLiteral("ok")).toBool()) {
            return;
        }

        const QVariant dataVar = response.value(QStringLiteral("data")).toVariant();
        const QVariantList raw = dataVar.toList();
        QVariantList albums;
        for (const QVariant &v : raw) {
            const QVariantMap m = v.toMap();
            QVariantMap item;
            item.insert(QStringLiteral("id"), m.value(QStringLiteral("id")).toString());
            item.insert(QStringLiteral("name"), m.value(QStringLiteral("name")).toString());
            item.insert(QStringLiteral("createdAt"), m.value(QStringLiteral("createdAt")).toLongLong());
            albums.append(item);
        }

        m_albumList = albums;
        Q_EMIT albumListChanged();
    });
}

void WallpaperBackend::loadTasks()
{
    QCborMap request;
    request.insert(QStringLiteral("cmd"), QStringLiteral("storage-get-all-tasks"));

    m_ipc->sendRequest(request, [this](const QCborMap &response, const QString &error) {
        if (!error.isEmpty() || !response.value(QStringLiteral("ok")).toBool()) {
            return;
        }

        const QVariant dataVar = response.value(QStringLiteral("data")).toVariant();
        const QVariantList raw = dataVar.toList();
        QVariantList tasks;
        for (const QVariant &v : raw) {
            const QVariantMap m = v.toMap();
            const qint64 startTime = m.value(QStringLiteral("startTime")).toLongLong();
            QString displayTime;
            if (startTime > 0) {
                displayTime = QDateTime::fromSecsSinceEpoch(startTime).toString(Qt::ISODate);
            }

            QVariantMap item;
            item.insert(QStringLiteral("id"), m.value(QStringLiteral("id")).toString());
            item.insert(QStringLiteral("pluginId"), m.value(QStringLiteral("pluginId")).toString());
            item.insert(QStringLiteral("startTime"), startTime);
            item.insert(QStringLiteral("displayTime"), displayTime);
            tasks.append(item);
        }

        m_taskList = tasks;
        Q_EMIT taskListChanged();
    });
}

void WallpaperBackend::setProviderRootPath(const QString &path)
{
    if (m_providerRootPath == path) {
        return;
    }

    m_providerRootPath = path;
    updateFilterDisplayText();
    Q_EMIT providerRootPathChanged();
    Q_EMIT filterDisplayTextChanged();
    loadGalleryPage(1);
}

void WallpaperBackend::setSortDescending(bool desc)
{
    if (m_sortDescending == desc) {
        return;
    }

    m_sortDescending = desc;
    Q_EMIT sortDescendingChanged();
    if (m_providerRootPath == QLatin1String("all")) {
        loadGalleryPage(1);
    }
}

void WallpaperBackend::updateFilterDisplayText()
{
    if (m_providerRootPath == QLatin1String("all")) {
        m_filterDisplayText = QStringLiteral("All");
        return;
    }

    if (m_providerRootPath.startsWith(QLatin1String("album/"))) {
        const QString id = m_providerRootPath.mid(6);
        for (const QVariant &v : m_albumList) {
            const QVariantMap m = v.toMap();
            if (m.value(QStringLiteral("id")).toString() == id) {
                m_filterDisplayText = QStringLiteral("Album: %1").arg(m.value(QStringLiteral("name")).toString());
                return;
            }
        }
        m_filterDisplayText = QStringLiteral("Album: ?");
        return;
    }

    if (m_providerRootPath.startsWith(QLatin1String("task/"))) {
        const QString id = m_providerRootPath.mid(5);
        for (const QVariant &v : m_taskList) {
            const QVariantMap m = v.toMap();
            if (m.value(QStringLiteral("id")).toString() == id) {
                const QString pluginId = m.value(QStringLiteral("pluginId")).toString();
                const QString displayTime = m.value(QStringLiteral("displayTime")).toString();
                m_filterDisplayText = displayTime.isEmpty()
                    ? QStringLiteral("Task: %1").arg(pluginId)
                    : QStringLiteral("Task: %1 %2").arg(pluginId, displayTime);
                return;
            }
        }
        m_filterDisplayText = QStringLiteral("Task: ?");
        return;
    }

    m_filterDisplayText = m_providerRootPath;
}

void WallpaperBackend::setWallpaperByImageId(const QString &imageId)
{
    if (imageId.trimmed().isEmpty()) {
        return;
    }

    QCborMap request;
    request.insert(QStringLiteral("cmd"), QStringLiteral("settings-set-current-wallpaper-image-id"));
    request.insert(QStringLiteral("image_id"), imageId);

    const QString id = imageId.trimmed();
    m_ipc->sendRequest(request, [this, id](const QCborMap &response, const QString &error) {
        if (!error.isEmpty()) {
            Q_EMIT hintMessage(i18n("Failed to set wallpaper: %1", error));
            return;
        }
        if (!response.value(QStringLiteral("ok")).toBool()) {
            Q_EMIT hintMessage(i18n("Failed to set wallpaper: %1", cborToString(response.value(QStringLiteral("message")))));
            return;
        }
        requestCurrentWallpaperPathByImageId(id);
    });
}

void WallpaperBackend::syncImageConfig(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }
    Q_EMIT imageConfigSyncRequested(path);
}

void WallpaperBackend::onIpcConnectedChanged(bool connected)
{
    if (m_connected == connected) {
        return;
    }

    m_connected = connected;
    Q_EMIT connectedChanged();

    if (!connected) {
        return;
    }

    QCborArray kinds;
    kinds.append(QStringLiteral("wallpaper-update-image"));
    kinds.append(QStringLiteral("setting-change"));
    kinds.append(QStringLiteral("images-change"));
    kinds.append(QStringLiteral("daemon-shutdown"));
    kinds.append(QStringLiteral("album-added"));
    kinds.append(QStringLiteral("album-name-changed"));
    kinds.append(QStringLiteral("album-deleted"));
    kinds.append(QStringLiteral("task-status"));
    m_ipc->subscribeEvents(kinds);

    requestInitialState();
}

void WallpaperBackend::onIpcEventReceived(const QString &eventType, const QCborMap &payload)
{
    if (eventType == QLatin1String("wallpaper-update-image")) {
        const QString imagePath = cborToString(payload.value(QStringLiteral("imagePath")));
        setCurrentWallpaper(imagePath, true);
        return;
    }

    if (eventType == QLatin1String("setting-change")) {
        const QVariantMap root = cborToVariantMap(QCborValue(payload));
        applySettingChanges(root.value(QStringLiteral("changes")).toMap());
        return;
    }

    if (eventType == QLatin1String("album-added")) {
        loadAlbums();
        loadGalleryPage(m_galleryPage);
        return;
    }

    if (eventType == QLatin1String("album-deleted")) {
        const QVariantMap root = cborToVariantMap(QCborValue(payload));
        const QString albumId = root.value(QStringLiteral("albumId")).toString();
        loadAlbums();
        if (m_providerRootPath == QStringLiteral("album/%1").arg(albumId)) {
            setProviderRootPath(QStringLiteral("all"));
        } else {
            loadGalleryPage(m_galleryPage);
        }
        return;
    }

    if (eventType == QLatin1String("album-name-changed")) {
        const QVariantMap root = cborToVariantMap(QCborValue(payload));
        const QString albumId = root.value(QStringLiteral("albumId")).toString();
        const QString newName = root.value(QStringLiteral("newName")).toString();
        loadAlbums();
        if (m_providerRootPath == QStringLiteral("album/%1").arg(albumId)) {
            m_filterDisplayText = QStringLiteral("Album: %1").arg(newName);
            Q_EMIT filterDisplayTextChanged();
        }
        loadGalleryPage(m_galleryPage);
        return;
    }

    if (eventType == QLatin1String("task-status")) {
        loadTasks();
        loadGalleryPage(m_galleryPage);
        return;
    }

    if (eventType == QLatin1String("images-change")) {
        loadGalleryPage(m_galleryPage);
        return;
    }

    if (eventType == QLatin1String("daemon-shutdown")) {
        m_connected = false;
        Q_EMIT connectedChanged();
    }
}

void WallpaperBackend::requestInitialState()
{
    requestSettingValue(QStringLiteral("settings-get-wallpaper-style"));
    requestSettingValue(QStringLiteral("settings-get-wallpaper-rotation-transition"));
    requestSettingValue(QStringLiteral("settings-get-wallpaper-volume"));
    requestSettingValue(QStringLiteral("settings-get-wallpaper-video-playback-rate"));
    requestSettingValue(QStringLiteral("settings-get-wallpaper-rotation-interval-minutes"));
    requestSettingValue(QStringLiteral("settings-get-wallpaper-rotation-mode"));
    requestSettingValue(QStringLiteral("settings-get-gallery-image-aspect-ratio"));

    QCborMap currentReq;
    currentReq.insert(QStringLiteral("cmd"), QStringLiteral("settings-get-current-wallpaper-image-id"));
    m_ipc->sendRequest(currentReq, [this](const QCborMap &response, const QString &error) {
        if (!error.isEmpty() || !response.value(QStringLiteral("ok")).toBool()) {
            return;
        }

        const QString imageId = cborToString(response.value(QStringLiteral("data")));
        if (!imageId.isEmpty()) {
            requestCurrentWallpaperPathByImageId(imageId);
        }
    });

    loadAlbums();
    loadTasks();
    loadGalleryPage(1);
}

void WallpaperBackend::requestCurrentWallpaperPathByImageId(const QString &imageId)
{
    QCborMap imageReq;
    imageReq.insert(QStringLiteral("cmd"), QStringLiteral("storage-get-image-by-id"));
    imageReq.insert(QStringLiteral("image_id"), imageId);

    m_ipc->sendRequest(imageReq, [this](const QCborMap &response, const QString &error) {
        if (!error.isEmpty() || !response.value(QStringLiteral("ok")).toBool()) {
            return;
        }
        const QVariantMap dataMap = cborToVariantMap(response.value(QStringLiteral("data")));
        const QString path = dataMap.value(QStringLiteral("localPath")).toString();
        setCurrentWallpaper(path, true);
    });
}

void WallpaperBackend::requestSettingValue(const QString &cmd)
{
    QCborMap req;
    req.insert(QStringLiteral("cmd"), cmd);

    m_ipc->sendRequest(req, [this, cmd](const QCborMap &response, const QString &error) {
        if (!error.isEmpty() || !response.value(QStringLiteral("ok")).toBool()) {
            return;
        }

        const QCborValue data = response.value(QStringLiteral("data"));
        if (cmd == QLatin1String("settings-get-wallpaper-style")) {
            const QString style = cborToString(data);
            if (!style.isEmpty() && m_wallpaperStyle != style) {
                m_wallpaperStyle = style;
                Q_EMIT wallpaperStyleChanged();
            }
            return;
        }
        if (cmd == QLatin1String("settings-get-wallpaper-rotation-transition")) {
            const QString transition = cborToString(data);
            if (!transition.isEmpty() && m_wallpaperTransition != transition) {
                m_wallpaperTransition = transition;
                Q_EMIT wallpaperTransitionChanged();
            }
            return;
        }
        if (cmd == QLatin1String("settings-get-wallpaper-volume")) {
            const double volume = data.toDouble();
            if (!qFuzzyCompare(m_wallpaperVolume, volume)) {
                m_wallpaperVolume = volume;
                Q_EMIT wallpaperVolumeChanged();
            }
            return;
        }
        if (cmd == QLatin1String("settings-get-wallpaper-video-playback-rate")) {
            const double rate = data.toDouble();
            if (!qFuzzyCompare(m_wallpaperVideoPlaybackRate, rate)) {
                m_wallpaperVideoPlaybackRate = rate;
                Q_EMIT wallpaperVideoPlaybackRateChanged();
            }
            return;
        }
        if (cmd == QLatin1String("settings-get-wallpaper-rotation-interval-minutes")) {
            const int minutes = static_cast<int>(data.toInteger());
            if (m_wallpaperRotationIntervalMinutes != minutes) {
                m_wallpaperRotationIntervalMinutes = minutes;
                Q_EMIT wallpaperRotationIntervalMinutesChanged();
            }
            return;
        }
        if (cmd == QLatin1String("settings-get-wallpaper-rotation-mode")) {
            const QString mode = cborToString(data);
            if (!mode.isEmpty() && m_wallpaperRotationMode != mode) {
                m_wallpaperRotationMode = mode;
                Q_EMIT wallpaperRotationModeChanged();
            }
        }
    });
}

void WallpaperBackend::applySettingChanges(const QVariantMap &changes)
{
    if (changes.contains(QStringLiteral("wallpaperStyle"))) {
        const QString style = changes.value(QStringLiteral("wallpaperStyle")).toString();
        if (!style.isEmpty() && m_wallpaperStyle != style) {
            m_wallpaperStyle = style;
            Q_EMIT wallpaperStyleChanged();
        }
    }

    if (changes.contains(QStringLiteral("wallpaperRotationTransition"))) {
        const QString transition = changes.value(QStringLiteral("wallpaperRotationTransition")).toString();
        if (!transition.isEmpty() && m_wallpaperTransition != transition) {
            m_wallpaperTransition = transition;
            Q_EMIT wallpaperTransitionChanged();
        }
    }

    if (changes.contains(QStringLiteral("wallpaperVolume"))) {
        const double volume = changes.value(QStringLiteral("wallpaperVolume")).toDouble();
        if (!qFuzzyCompare(m_wallpaperVolume, volume)) {
            m_wallpaperVolume = volume;
            Q_EMIT wallpaperVolumeChanged();
        }
    }

    if (changes.contains(QStringLiteral("wallpaperVideoPlaybackRate"))) {
        const double rate = changes.value(QStringLiteral("wallpaperVideoPlaybackRate")).toDouble();
        if (!qFuzzyCompare(m_wallpaperVideoPlaybackRate, rate)) {
            m_wallpaperVideoPlaybackRate = rate;
            Q_EMIT wallpaperVideoPlaybackRateChanged();
        }
    }

    if (changes.contains(QStringLiteral("wallpaperRotationIntervalMinutes"))) {
        const int minutes = changes.value(QStringLiteral("wallpaperRotationIntervalMinutes")).toInt();
        if (m_wallpaperRotationIntervalMinutes != minutes) {
            m_wallpaperRotationIntervalMinutes = minutes;
            Q_EMIT wallpaperRotationIntervalMinutesChanged();
        }
    }

    if (changes.contains(QStringLiteral("wallpaperRotationMode"))) {
        const QString mode = changes.value(QStringLiteral("wallpaperRotationMode")).toString();
        if (!mode.isEmpty() && m_wallpaperRotationMode != mode) {
            m_wallpaperRotationMode = mode;
            Q_EMIT wallpaperRotationModeChanged();
        }
    }

    if (changes.contains(QStringLiteral("galleryImageAspectRatio"))) {
        const QString ratio = changes.value(QStringLiteral("galleryImageAspectRatio")).toString();
        if (!ratio.isEmpty() && m_galleryImageAspectRatio != ratio) {
            m_galleryImageAspectRatio = ratio;
            Q_EMIT galleryImageAspectRatioChanged();
        }
    }

    if (changes.contains(QStringLiteral("currentWallpaperImageId"))) {
        const QVariant idVar = changes.value(QStringLiteral("currentWallpaperImageId"));
        const QString imageId = idVar.isNull() ? QString() : idVar.toString().trimmed();
        if (!imageId.isEmpty()) {
            requestCurrentWallpaperPathByImageId(imageId);
        }
    }
}

void WallpaperBackend::setCurrentWallpaper(const QString &path, bool requestQmlSwitch)
{
    if (path.isEmpty()) {
        return;
    }

    if (m_currentWallpaper == path) {
        return;
    }

    m_currentWallpaper = path;
    Q_EMIT currentWallpaperChanged();
    Q_EMIT imageConfigSyncRequested(path);

    if (requestQmlSwitch) {
        Q_EMIT wallpaperChangeRequested(path);
    }
}

QString WallpaperBackend::toFileUrl(const QString &path) const
{
    if (path.isEmpty()) {
        return QString();
    }
    return QUrl::fromLocalFile(path).toString();
}

QString WallpaperBackend::cborToString(const QCborValue &value) const
{
    if (value.isString()) {
        return value.toString();
    }
    if (value.isByteArray()) {
        return QString::fromUtf8(value.toByteArray());
    }
    return QString();
}

QVariantMap WallpaperBackend::cborToVariantMap(const QCborValue &value) const
{
    if (value.isMap()) {
        return value.toMap().toVariantMap();
    }
    return QVariantMap();
}
