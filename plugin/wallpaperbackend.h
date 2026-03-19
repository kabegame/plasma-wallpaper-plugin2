#ifndef WALLPAPERBACKEND_H
#define WALLPAPERBACKEND_H

#include "ipcclient.h"

#include <QObject>
#include <QProcess>
#include <QUrl>
#include <QVariantList>

class WallpaperBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString currentWallpaper READ currentWallpaper NOTIFY currentWallpaperChanged)
    Q_PROPERTY(QString wallpaperStyle READ wallpaperStyle NOTIFY wallpaperStyleChanged)
    Q_PROPERTY(QString wallpaperTransition READ wallpaperTransition NOTIFY wallpaperTransitionChanged)
    Q_PROPERTY(double wallpaperVolume READ wallpaperVolume NOTIFY wallpaperVolumeChanged)
    Q_PROPERTY(double wallpaperVideoPlaybackRate READ wallpaperVideoPlaybackRate NOTIFY wallpaperVideoPlaybackRateChanged)
    Q_PROPERTY(int wallpaperRotationIntervalMinutes READ wallpaperRotationIntervalMinutes NOTIFY wallpaperRotationIntervalMinutesChanged)
    Q_PROPERTY(QString wallpaperRotationMode READ wallpaperRotationMode NOTIFY wallpaperRotationModeChanged)
    Q_PROPERTY(QString galleryImageAspectRatio READ galleryImageAspectRatio NOTIFY galleryImageAspectRatioChanged)
    Q_PROPERTY(QVariantList galleryImages READ galleryImages NOTIFY galleryImagesChanged)
    Q_PROPERTY(int galleryTotal READ galleryTotal NOTIFY galleryTotalChanged)
    Q_PROPERTY(int galleryPage READ galleryPage NOTIFY galleryPageChanged)
    Q_PROPERTY(int galleryPageSize READ galleryPageSize CONSTANT)

public:
    explicit WallpaperBackend(QObject *parent = nullptr);
    ~WallpaperBackend() override = default;

    bool connected() const { return m_connected; }
    QString currentWallpaper() const { return m_currentWallpaper; }
    QString wallpaperStyle() const { return m_wallpaperStyle; }
    QString wallpaperTransition() const { return m_wallpaperTransition; }
    double wallpaperVolume() const { return m_wallpaperVolume; }
    double wallpaperVideoPlaybackRate() const { return m_wallpaperVideoPlaybackRate; }
    int wallpaperRotationIntervalMinutes() const { return m_wallpaperRotationIntervalMinutes; }
    QString wallpaperRotationMode() const { return m_wallpaperRotationMode; }
    QString galleryImageAspectRatio() const { return m_galleryImageAspectRatio; }
    QVariantList galleryImages() const { return m_galleryImages; }
    int galleryTotal() const { return m_galleryTotal; }
    int galleryPage() const { return m_galleryPage; }
    int galleryPageSize() const { return m_galleryPageSize; }

    Q_INVOKABLE void connectToKabegame();
    Q_INVOKABLE void openKabegame();
    Q_INVOKABLE void loadGalleryPage(int page);
    Q_INVOKABLE void setWallpaperByImageId(const QString &imageId);
    /// 将本地路径转为带编码的 file:// URL，供 QML Image/Video 正确加载含中文等字符的路径
    Q_INVOKABLE QString toFileUrl(const QString &path) const;

public Q_SLOTS:
    void syncImageConfig(const QString &path);

Q_SIGNALS:
    void connectedChanged();
    void currentWallpaperChanged();
    void wallpaperStyleChanged();
    void wallpaperTransitionChanged();
    void wallpaperVolumeChanged();
    void wallpaperVideoPlaybackRateChanged();
    void wallpaperRotationIntervalMinutesChanged();
    void wallpaperRotationModeChanged();
    void galleryImageAspectRatioChanged();
    void galleryImagesChanged();
    void galleryTotalChanged();
    void galleryPageChanged();

    void wallpaperChangeRequested(const QString &newWallpaper);
    void imageConfigSyncRequested(const QString &path);
    void hintMessage(const QString &message);

private Q_SLOTS:
    void onIpcConnectedChanged(bool connected);
    void onIpcEventReceived(const QString &eventType, const QCborMap &payload);

private:
    void requestInitialState();
    void requestCurrentWallpaperPathByImageId(const QString &imageId);
    void requestSettingValue(const QString &cmd);
    void applySettingChanges(const QVariantMap &changes);
    void setCurrentWallpaper(const QString &path, bool requestQmlSwitch);
    QString cborToString(const QCborValue &value) const;
    QVariantMap cborToVariantMap(const QCborValue &value) const;

    IpcClient *m_ipc = nullptr;

    bool m_connected = false;
    QString m_currentWallpaper;
    QString m_wallpaperStyle = QStringLiteral("fill");
    QString m_wallpaperTransition = QStringLiteral("fade");
    double m_wallpaperVolume = 1.0;
    double m_wallpaperVideoPlaybackRate = 1.0;
    int m_wallpaperRotationIntervalMinutes = 60;
    QString m_wallpaperRotationMode = QStringLiteral("random");
    QString m_galleryImageAspectRatio = QStringLiteral("16:9");

    QVariantList m_galleryImages;
    int m_galleryTotal = 0;
    int m_galleryPage = 1;
    const int m_galleryPageSize = 100;
};

#endif // WALLPAPERBACKEND_H
