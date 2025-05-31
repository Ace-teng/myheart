#include "heartbeatclient.h"

HeartbeatClient::HeartbeatClient(QObject *parent)
    : QObject(parent), m_connected(false), m_lastHeartbeat(0)
{
    m_socket = new QTcpSocket(this);

    connect(m_socket, &QAbstractSocket::connected, this, &HeartbeatClient::onConnected);
    connect(m_socket, &QAbstractSocket::disconnected, this, &HeartbeatClient::onDisconnected);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &HeartbeatClient::onErrorOccurred);
    connect(m_socket, &QAbstractSocket::readyRead, this, &HeartbeatClient::onReadyRead);

    m_connectionTimer = new QTimer(this);
    m_connectionTimer->setInterval(10000);
    connect(m_connectionTimer, &QTimer::timeout, this, &HeartbeatClient::checkConnection);
}

HeartbeatClient::~HeartbeatClient()
{
    disconnectFromServer();
}

void HeartbeatClient::connectToServer(const QString &host, quint16 port, const QString &deviceId)
{
    if (m_connected) return;

    m_deviceId = deviceId;
    m_socket->connectToHost(host, port);
}

void HeartbeatClient::disconnectFromServer()
{
    if (m_connected) {
        m_connectionTimer->stop();
        m_socket->disconnectFromHost();
    }
}

void HeartbeatClient::onConnected()
{
    m_connected = true;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << QString("CLIENT_REGISTER") << m_deviceId;

    if (m_socket->write(data) == -1) {
        emit errorOccurred("Failed to register client: " + m_socket->errorString());
    } else {
        m_connectionTimer->start();
        emit connectionStatusChanged(true);
    }
}

void HeartbeatClient::onDisconnected()
{
    m_connected = false;
    m_connectionTimer->stop();
    emit connectionStatusChanged(false);
}

void HeartbeatClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit errorOccurred(m_socket->errorString());
}

void HeartbeatClient::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    QDataStream stream(data);
    QString messageType, deviceId;
    qint64 timestamp;

    stream >> messageType;

    if (messageType == "HEARTBEAT") {
        stream >> deviceId >> timestamp;

        if (deviceId == m_deviceId) {
            m_lastHeartbeat = timestamp;
            emit heartbeatReceived(timestamp);
        }
    }
}

void HeartbeatClient::checkConnection()
{
    if (!m_connected) return;

    if (m_lastHeartbeat > 0 && QDateTime::currentMSecsSinceEpoch() - m_lastHeartbeat > 20000) {
        emit connectionTimeout();
        disconnectFromServer();
    }
}
