#include "heartbeatserver.h"

HeartbeatServer::HeartbeatServer(QObject *parent)
    : QObject(parent)
{
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection,
            this, &HeartbeatServer::onNewConnection);
}

HeartbeatServer::~HeartbeatServer()
{
    stopServer();
}

bool HeartbeatServer::startServer(quint16 port)
{
    if (m_server->isListening()) {
        emit serverStarted(true);
        return true;
    }

    if (!m_server->listen(QHostAddress::Any, port)) {
        emit errorOccurred(m_server->errorString());
        emit serverStarted(false);
        return false;
    }

    emit serverStarted(true);
    return true;
}

void HeartbeatServer::stopServer()
{
    if (m_server->isListening()) {
        m_server->close();
        for (auto socket : m_deviceSockets.keys()) {
            socket->disconnectFromHost();
        }
        for (auto socket : m_clientSockets) {
            socket->disconnectFromHost();
        }
        m_deviceSockets.clear();
        m_clientSockets.clear();
        m_lastHeartbeats.clear();
        emit serverStopped();
    }
}

void HeartbeatServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *socket = m_server->nextPendingConnection();
        connect(socket, &QAbstractSocket::disconnected,
                this, [this, socket]() {
            onClientDisconnected(socket);
        });
        connect(socket, &QAbstractSocket::readyRead,
                this, [this, socket]() {
            onReadyRead(socket);
        });
    }
}

void HeartbeatServer::onClientDisconnected(QTcpSocket *socket)
{
    if (!socket) return;

    if (m_deviceSockets.contains(socket)) {
        QString deviceId = m_deviceSockets.value(socket);
        m_deviceSockets.remove(socket);
        m_lastHeartbeats.remove(deviceId);
        emit deviceDisconnected(deviceId);
    } else {
        // Find if this was a client socket
        QString deviceId = m_clientSockets.key(socket);
        if (!deviceId.isEmpty()) {
            m_clientSockets.remove(deviceId);
            // 对于客户端断开，也触发设备断开信号（因为客户端关联了设备ID）
            emit deviceDisconnected(deviceId);
        }
    }

    socket->deleteLater();
}

void HeartbeatServer::onReadyRead(QTcpSocket *socket)
{
    QByteArray data = socket->readAll();
    QDataStream stream(data);
    QString messageType, deviceId;
    qint64 timestamp;

    stream >> messageType;

    if (messageType == "HEARTBEAT") {
        stream >> deviceId >> timestamp;

        if (!m_deviceSockets.contains(socket)) {
            m_deviceSockets.insert(socket, deviceId);
            emit deviceConnected(deviceId);
        }

        m_lastHeartbeats[deviceId] = timestamp;
        emit heartbeatReceived(deviceId, timestamp);

        // Forward to client if connected
        if (m_clientSockets.contains(deviceId)) {
            QByteArray forwardData;
            QDataStream forwardStream(&forwardData, QIODevice::WriteOnly);
            forwardStream << QString("HEARTBEAT") << deviceId << timestamp;
            m_clientSockets[deviceId]->write(forwardData);
        }
    } else if (messageType == "CLIENT_REGISTER") {
        stream >> deviceId;

        if (!m_clientSockets.contains(deviceId)) {
            m_clientSockets[deviceId] = socket;
            emit deviceConnected(deviceId);
        } else {
            // Replace existing client socket
            QTcpSocket *oldSocket = m_clientSockets[deviceId];
            oldSocket->disconnectFromHost();
            m_clientSockets[deviceId] = socket;
        }
    }
}
