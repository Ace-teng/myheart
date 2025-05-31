#include "heartbeatdevice.h"
#include <QNetworkInterface>

HeartbeatDevice::HeartbeatDevice(QObject *parent)
    : QObject(parent), m_connected(false)
{
    m_socket = new QTcpSocket(this);

    connect(m_socket, &QAbstractSocket::connected, this, &HeartbeatDevice::onConnected);
    connect(m_socket, &QAbstractSocket::disconnected, this, &HeartbeatDevice::onDisconnected);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &HeartbeatDevice::onErrorOccurred);

    // Generate device ID based on MAC address
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
    m_heartbeatTimer->start(5000);
}

HeartbeatDevice::~HeartbeatDevice()
{
    disconnectFromServer();
    delete m_socket;
}

void HeartbeatDevice::connectToServer(const QString &host, quint16 port)
{
    if (m_connected) return;

    m_socket->connectToHost(host, port);
}

void HeartbeatDevice::disconnectFromServer()
{
    if (m_connected) {
        m_heartbeatTimer->stop();// 停止心跳发送
        m_socket->disconnectFromHost();
    }
}

void HeartbeatDevice::onConnected()
{
    m_connected = true;
    emit connectionStatusChanged(true);
}

void HeartbeatDevice::onDisconnected()
{
    m_connected = false;
    emit connectionStatusChanged(false);
}

void HeartbeatDevice::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit errorOccurred(m_socket->errorString());
}

void HeartbeatDevice::sendHeartbeat()
{
    if (!m_connected) return;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << QString("HEARTBEAT") << m_deviceId << QDateTime::currentMSecsSinceEpoch();

    if (m_socket->write(data) == -1) {
        emit errorOccurred("Failed to send heartbeat: " + m_socket->errorString());
    } else {
        emit heartbeatSent(QDateTime::currentMSecsSinceEpoch());
    }
}
