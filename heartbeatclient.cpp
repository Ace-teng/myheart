#include "heartbeatclient.h"
#include <QDebug>

HeartbeatClient::HeartbeatClient(QObject *parent)
    : QObject(parent), m_connected(false), m_reconnectAttempts(0),
    m_connectionTimeoutInterval(15000), m_reconnectInterval(5000) {
    m_socket = new QTcpSocket(this);

    connect(m_socket, &QAbstractSocket::connected, this, &HeartbeatClient::onConnected);
    connect(m_socket, &QAbstractSocket::disconnected, this, &HeartbeatClient::onDisconnected);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &HeartbeatClient::onErrorOccurred);
    connect(m_socket, &QAbstractSocket::readyRead, this, &HeartbeatClient::onReadyRead);

    m_connectionTimer = new QTimer(this);
    m_connectionTimer->setInterval(m_connectionTimeoutInterval);
    connect(m_connectionTimer, &QTimer::timeout, this, &HeartbeatClient::checkConnection);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(m_reconnectInterval);
    connect(m_reconnectTimer, &QTimer::timeout, this, &HeartbeatClient::attemptReconnect);
}

HeartbeatClient::~HeartbeatClient() {
    disconnectFromServer();
}

void HeartbeatClient::connectToServer(const QString &host, quint16 port, const QString &deviceId) {
    if (m_connected)
        return;

    QHostAddress address(host);
    if (address.isNull()) {
        emit errorOccurred("Invalid host address");
        return;
    }

    m_deviceId = deviceId;
    m_socket->connectToHost(address, port);
}

void HeartbeatClient::disconnectFromServer() {
    if (m_connected) {
        m_connectionTimer->stop();
        m_reconnectTimer->stop();
        m_socket->disconnectFromHost();
        QString disconnectMsg = QString("Disconnected from server for device: %1").arg(m_deviceId);
        emit statusMessage(disconnectMsg);
    }
}

void HeartbeatClient::setConnectionTimeoutInterval(int interval) {
    m_connectionTimeoutInterval = interval;
    m_connectionTimer->setInterval(interval);
}

void HeartbeatClient::setReconnectInterval(int interval) {
    m_reconnectInterval = interval;
    m_reconnectTimer->setInterval(interval);
}

void HeartbeatClient::onConnected() {
    m_connected = true;
    m_reconnectAttempts = 0;
    m_lastHeartbeat = QDateTime::currentDateTimeUtc(); // 初始化心跳时间
    m_connectionTimer->start();
    emit connectionStatusChanged(true);
    QString connectMsg = "Successfully connected to server";
    emit statusMessage(connectMsg);
}

void HeartbeatClient::onDisconnected() {
    m_connected = false;
    m_connectionTimer->stop();
    emit connectionStatusChanged(false);

    // 触发自动重连
    m_reconnectTimer->start();
}

void HeartbeatClient::onErrorOccurred(QAbstractSocket::SocketError socketError) {
    QString errorMessage = QString("Socket error: %1 - %2").arg(socketError).arg(m_socket->errorString());
    emit errorOccurred(errorMessage);
    emit statusMessage(errorMessage);
}

void HeartbeatClient::onReadyRead() {
    QByteArray data = m_socket->readAll();
    QDataStream stream(data);
    QString messageType, deviceId;
    QDateTime timestamp;

    stream >> messageType;

    if (messageType == "HEARTBEAT") {
        stream >> deviceId >> timestamp;
        qDebug() << "Received heartbeat from device:" << deviceId << "at" << timestamp.toString(Qt::ISODate);

        if (deviceId == m_deviceId) {
            m_lastHeartbeat = timestamp; // 更新心跳时间
            emit heartbeatReceived(timestamp);
        }
    } else {
        qDebug() << "Unknown message type received:" << messageType;
    }
}

void HeartbeatClient::checkConnection() {
    if (!m_connected)
        return;

    if (m_lastHeartbeat.isValid()) {
        qDebug() << "Checking connection for device:" << m_deviceId
                 << "Last heartbeat:" << m_lastHeartbeat.toString(Qt::ISODate)
                 << "Current time:" << QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

        if (QDateTime::currentDateTimeUtc() > m_lastHeartbeat.addMSecs(m_connectionTimeoutInterval)) {
            emit connectionTimeout();
            QString timeoutMsg = QString("Connection timeout for device: %1").arg(m_deviceId);
            emit statusMessage(timeoutMsg);
            disconnectFromServer();
        }
    }
}

void HeartbeatClient::attemptReconnect() {
    if (m_reconnectAttempts < 5) {
        m_reconnectAttempts++;
        emit statusMessage(QString("Attempting reconnect (%1)...").arg(m_reconnectAttempts));
        m_socket->connectToHost(m_socket->peerAddress(), m_socket->peerPort());
    } else {
        emit errorOccurred("Max reconnection attempts reached");
        m_reconnectTimer->stop();
    }
}
