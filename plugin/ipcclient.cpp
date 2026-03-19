#include "ipcclient.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <cstring>

namespace {
constexpr int kMaxFrameSize = 64 * 1024 * 1024;
constexpr int kHeartbeatMs = 5 * 60 * 1000;
constexpr int kReconnectMaxMs = 60 * 1000;
}

IpcClient::IpcClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QLocalSocket(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
{
    // 与 Rust unix_socket_path() 一致：temp_dir/Kabegame/kabegame.sock
    QString base = QDir::tempPath();
    if (base.isEmpty()) {
        base = QStringLiteral("/tmp");
    }
    base = QDir::fromNativeSeparators(base);
    if (!base.endsWith(QLatin1Char('/'))) {
        base += QLatin1Char('/');
    }
    m_socketPath = base + QStringLiteral("Kabegame/kabegame.sock");

    m_heartbeatTimer->setInterval(kHeartbeatMs);
    m_heartbeatTimer->setSingleShot(false);
    m_reconnectTimer->setSingleShot(true);

    connect(m_socket, &QLocalSocket::connected, this, &IpcClient::onConnected);
    connect(m_socket, &QLocalSocket::disconnected, this, &IpcClient::onDisconnected);
    connect(m_socket, &QLocalSocket::readyRead, this, &IpcClient::onReadyRead);
    connect(m_socket, &QLocalSocket::errorOccurred, this, &IpcClient::onSocketError);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &IpcClient::onHeartbeatTimer);
    connect(m_reconnectTimer, &QTimer::timeout, this, &IpcClient::onReconnectTimer);
}

bool IpcClient::isConnected() const
{
    return m_socket->state() == QLocalSocket::ConnectedState;
}

void IpcClient::connectToDaemon()
{
    m_shouldReconnect = true;

    if (isConnected() || m_socket->state() == QLocalSocket::ConnectingState) {
        return;
    }

    if (m_socketPath.isEmpty()) {
        qWarning() << "[Kabegame] IPC socket path is empty, cannot connect";
        return;
    }

    m_socket->abort();
    m_socket->connectToServer(m_socketPath);
}

void IpcClient::disconnectFromDaemon()
{
    m_shouldReconnect = false;
    m_heartbeatTimer->stop();
    m_reconnectTimer->stop();

    if (m_socket->state() != QLocalSocket::UnconnectedState) {
        m_socket->disconnectFromServer();
    }
}

quint64 IpcClient::sendRequest(const QCborMap &request, const ReplyHandler &handler)
{
    if (!isConnected()) {
        if (handler) {
            handler(QCborMap(), QStringLiteral("IPC not connected"));
        }
        return 0;
    }

    const quint64 requestId = m_nextRequestId++;
    const QCborMap payloadMap = addRequestEnvelope(request, requestId);
    const QByteArray payload = QCborValue(payloadMap).toCbor();

    if (!writeFrame(payload)) {
        if (handler) {
            handler(QCborMap(), QStringLiteral("IPC write failed"));
        }
        scheduleReconnect();
        return 0;
    }

    if (handler) {
        m_pendingHandlers.insert(requestId, handler);
    }

    return requestId;
}

void IpcClient::subscribeEvents(const QCborArray &kinds)
{
    QCborMap request;
    request.insert(QStringLiteral("cmd"), QStringLiteral("subscribe-events"));
    request.insert(QStringLiteral("kinds"), kinds);

    sendRequest(request, [this](const QCborMap &response, const QString &error) {
        if (!error.isEmpty()) {
            qWarning() << "[Kabegame] 事件订阅失败:" << error;
            return;
        }

        if (!response.value(QStringLiteral("ok")).toBool()) {
            qWarning() << "[Kabegame] 事件订阅返回失败:" << cborToString(response.value(QStringLiteral("message")));
            return;
        }

        m_eventSubscribed = true;
    });
}

void IpcClient::onConnected()
{
    m_readBuffer.clear();
    m_reconnectBackoffMs = 1000;
    m_eventSubscribed = false;

    if (!m_heartbeatTimer->isActive()) {
        m_heartbeatTimer->start();
    }

    Q_EMIT connectedChanged(true);
}

void IpcClient::onDisconnected()
{
    if (m_heartbeatTimer->isActive()) {
        m_heartbeatTimer->stop();
    }

    m_eventSubscribed = false;
    Q_EMIT connectedChanged(false);

    if (m_shouldReconnect) {
        scheduleReconnect();
    }
}

void IpcClient::onSocketError(QLocalSocket::LocalSocketError socketError)
{
    Q_UNUSED(socketError)
    if (m_shouldReconnect) {
        scheduleReconnect();
    }
}

void IpcClient::onReadyRead()
{
    m_readBuffer.append(m_socket->readAll());
    processFrames();
}

void IpcClient::onHeartbeatTimer()
{
    QCborMap statusReq;
    statusReq.insert(QStringLiteral("cmd"), QStringLiteral("status"));
    sendRequest(statusReq, [this](const QCborMap &response, const QString &error) {
        if (!error.isEmpty() || !response.value(QStringLiteral("ok")).toBool()) {
            qWarning() << "[Kabegame] IPC 心跳失败，准备重连";
            m_socket->abort();
            scheduleReconnect();
        }
    });
}

void IpcClient::onReconnectTimer()
{
    connectToDaemon();
}

bool IpcClient::writeFrame(const QByteArray &payload)
{
    if (payload.size() > kMaxFrameSize) {
        qWarning() << "[Kabegame] IPC payload 太大:" << payload.size();
        return false;
    }

    QByteArray frame;
    frame.reserve(4 + payload.size());

    const quint32 len = static_cast<quint32>(payload.size());
    frame.append(reinterpret_cast<const char *>(&len), 4);
    frame.append(payload);

    const qint64 written = m_socket->write(frame);
    if (written != frame.size()) {
        return false;
    }
    return m_socket->flush();
}

void IpcClient::processFrames()
{
    while (m_readBuffer.size() >= 4) {
        quint32 frameLen = 0;
        memcpy(&frameLen, m_readBuffer.constData(), sizeof(quint32));

        if (frameLen > static_cast<quint32>(kMaxFrameSize)) {
            qWarning() << "[Kabegame] IPC frame 长度异常:" << frameLen;
            m_readBuffer.clear();
            return;
        }

        if (m_readBuffer.size() < 4 + static_cast<int>(frameLen)) {
            return;
        }

        const QByteArray payload = m_readBuffer.mid(4, static_cast<int>(frameLen));
        m_readBuffer.remove(0, 4 + static_cast<int>(frameLen));
        processPayload(payload);
    }
}

void IpcClient::processPayload(const QByteArray &payload)
{
    const QCborValue value = QCborValue::fromCbor(payload);
    if (!value.isMap()) {
        return;
    }

    const QCborMap map = value.toMap();
    const QString eventType = cborToString(map.value(QStringLiteral("type")));
    if (!eventType.isEmpty()) {
        Q_EMIT eventReceived(eventType, map);
        return;
    }

    const quint64 requestId = readRequestId(map);
    if (requestId > 0 && m_pendingHandlers.contains(requestId)) {
        const ReplyHandler handler = m_pendingHandlers.take(requestId);
        if (handler) {
            handler(map, QString());
        }
    }

    if (requestId > 0) {
        Q_EMIT responseReceived(requestId, map);
    }
}

void IpcClient::scheduleReconnect()
{
    if (!m_shouldReconnect || m_reconnectTimer->isActive()) {
        return;
    }

    m_reconnectTimer->start(m_reconnectBackoffMs);
    m_reconnectBackoffMs = qMin(m_reconnectBackoffMs * 2, kReconnectMaxMs);
}

QString IpcClient::cborToString(const QCborValue &value) const
{
    if (value.isString()) {
        return value.toString();
    }
    if (value.isByteArray()) {
        return QString::fromUtf8(value.toByteArray());
    }
    return QString();
}

quint64 IpcClient::readRequestId(const QCborMap &map) const
{
    // Daemon 响应使用 serde rename_all = "kebab-case"，键名为 "request-id"
    QCborValue idValue = map.value(QStringLiteral("request-id"));
    if (idValue.isUndefined()) {
        idValue = map.value(QStringLiteral("request_id"));
    }
    if (idValue.isInteger()) {
        return static_cast<quint64>(idValue.toInteger());
    }
    if (idValue.isDouble()) {
        return static_cast<quint64>(idValue.toDouble());
    }
    return 0;
}

QCborMap IpcClient::addRequestEnvelope(const QCborMap &request, quint64 requestId) const
{
    QCborMap envelope = request;
    envelope.insert(QStringLiteral("request_id"), static_cast<qint64>(requestId));
    return envelope;
}
