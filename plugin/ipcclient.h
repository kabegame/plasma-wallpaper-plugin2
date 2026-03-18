#ifndef IPCCLIENT_H
#define IPCCLIENT_H

#include <QCborArray>
#include <QCborMap>
#include <QObject>
#include <QLocalSocket>
#include <QTimer>

#include <functional>

class IpcClient : public QObject
{
    Q_OBJECT

public:
    using ReplyHandler = std::function<void(const QCborMap &, const QString &)>;

    explicit IpcClient(QObject *parent = nullptr);
    ~IpcClient() override = default;

    bool isConnected() const;
    QString socketPath() const { return m_socketPath; }

    void connectToDaemon();
    void disconnectFromDaemon();

    quint64 sendRequest(const QCborMap &request, const ReplyHandler &handler = ReplyHandler());
    void subscribeEvents(const QCborArray &kinds);

Q_SIGNALS:
    void connectedChanged(bool connected);
    void eventReceived(const QString &eventType, const QCborMap &eventPayload);
    void responseReceived(quint64 requestId, const QCborMap &response);

private Q_SLOTS:
    void onConnected();
    void onDisconnected();
    void onSocketError(QLocalSocket::LocalSocketError socketError);
    void onReadyRead();
    void onHeartbeatTimer();
    void onReconnectTimer();

private:
    bool writeFrame(const QByteArray &payload);
    void processFrames();
    void processPayload(const QByteArray &payload);
    void scheduleReconnect();
    QString cborToString(const QCborValue &value) const;
    quint64 readRequestId(const QCborMap &map) const;
    QCborMap addRequestEnvelope(const QCborMap &request, quint64 requestId) const;

    QLocalSocket *m_socket = nullptr;
    QTimer *m_heartbeatTimer = nullptr;
    QTimer *m_reconnectTimer = nullptr;

    QString m_socketPath;
    QByteArray m_readBuffer;
    quint64 m_nextRequestId = 1;
    int m_reconnectBackoffMs = 1000;
    bool m_shouldReconnect = true;
    bool m_eventSubscribed = false;

    QHash<quint64, ReplyHandler> m_pendingHandlers;
};

#endif // IPCCLIENT_H
