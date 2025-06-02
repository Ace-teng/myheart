#include "heartbeatdevice.h"
#include <QNetworkInterface>
#include <QDebug>

HeartbeatDevice::HeartbeatDevice(QObject *parent)
    : QObject(parent), m_connected(false), m_reconnectAttempts(0), m_heartbeatInterval(8000) {
    m_socket = new QTcpSocket(this);

    connect(m_socket, &QAbstractSocket::connected, this, &HeartbeatDevice::onConnected);
    connect(m_socket, &QAbstractSocket::disconnected, this, &HeartbeatDevice::onDisconnected);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &HeartbeatDevice::onErrorOccurred);

    // 生成设备 ID
    foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
        if (!interface.hardwareAddress().isEmpty()) {
            m_deviceId = interface.hardwareAddress();
            break;
        }
    }
    if (m_deviceId.isEmpty()) {
        m_deviceId = "DEV - " + QString::number(QDateTime::currentSecsSinceEpoch());
    }

    m_heartbeatTimer = new QTimer(this);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &HeartbeatDevice::sendHeartbeat);
}

HeartbeatDevice::~HeartbeatDevice() {
    disconnectFromServer();
    delete m_socket;
}

void HeartbeatDevice::connectToServer(const QString &host, quint16 port) {
    if (m_connected)
        return;

    QHostAddress address(host);
    if (address.isNull()) {
        emit errorOccurred("Invalid host address");
        return;
    }

    m_socket->connectToHost(address, port);
}

void HeartbeatDevice::disconnectFromServer() {
    if (m_connected) {
        m_heartbeatTimer->stop();
        m_socket->disconnectFromHost();
    }
}

void HeartbeatDevice::onConnected() {
    m_connected = true;
    m_reconnectAttempts = 0;
    m_heartbeatTimer->start(m_heartbeatInterval);
    emit connectionStatusChanged(true);
}

void HeartbeatDevice::onDisconnected() {
    m_connected = false;
    m_heartbeatTimer->stop();
    emit connectionStatusChanged(false);

    // 触发自动重连
    QTimer::singleShot(5000, this, &HeartbeatDevice::attemptReconnect);
}

void HeartbeatDevice::onErrorOccurred(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError);
    emit errorOccurred(m_socket->errorString());
}

void HeartbeatDevice::sendHeartbeat() {
    if (!m_connected)
        return;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    QDateTime timestamp = QDateTime::currentDateTimeUtc(); // 使用 UTC 时间
    stream << QString("HEARTBEAT") << m_deviceId << timestamp;

    if (m_socket->write(data) == -1) {
        emit errorOccurred("Failed to send heartbeat: " + m_socket->errorString());
    } else {
        emit heartbeatSent(timestamp);
    }
}

void HeartbeatDevice::attemptReconnect() {
    if (m_reconnectAttempts < 5) {
        m_reconnectAttempts++;
        emit errorOccurred(QString("Attempting reconnect (%1)...").arg(m_reconnectAttempts));
        m_socket->connectToHost(m_socket->peerAddress(), m_socket->peerPort());
    } else {
        emit errorOccurred("Max reconnection attempts reached");
    }
}

void HeartbeatDevice::setHeartbeatInterval(int interval) {
    m_heartbeatInterval = interval;
    if (m_heartbeatTimer->isActive()) {
        m_heartbeatTimer->setInterval(interval);
    }
}
