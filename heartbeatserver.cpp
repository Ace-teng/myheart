#include "heartbeatserver.h"
#include <QDebug>
#include <QTimer>

HeartbeatChecker::HeartbeatChecker(QObject *parent)
    : QObject(parent) {}

void HeartbeatChecker::checkHeartbeats(const QMap<QString, QDateTime> &heartbeats) {
    QDateTime now = QDateTime::currentDateTimeUtc();
    for (auto it = heartbeats.begin(); it != heartbeats.end(); ++it) {
        QString deviceId = it.key();
        QDateTime lastHeartbeat = it.value();

        if (now > lastHeartbeat.addMSecs(20000)) { // 超时时间硬编码为 20 秒
            emit deviceTimedOut(deviceId);
        }
    }
}

HeartbeatServer::HeartbeatServer(QObject *parent)
    : QObject(parent), m_heartbeatTimeoutInterval(20000), m_heartbeatCheckInterval(5000) {
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &HeartbeatServer::onNewConnection);

    // 创建心跳检查线程
    m_checkerThread = new QThread(this);
    m_checker = new HeartbeatChecker();
    m_checker->moveToThread(m_checkerThread);

    // 启动线程
    connect(m_checkerThread, &QThread::started, [this]() {
        QTimer *timer = new QTimer();
        timer->setInterval(m_heartbeatCheckInterval); // 设置心跳检查间隔
        timer->moveToThread(m_checkerThread);

        connect(timer, &QTimer::timeout, this, [this]() {
            emit m_checker->checkHeartbeats(m_lastHeartbeats); // 通过信号传递心跳数据
        });

        timer->start();
    });

    // 处理设备超时信号
    connect(m_checker, &HeartbeatChecker::deviceTimedOut, this, &HeartbeatServer::handleDeviceTimeout);

    m_checkerThread->start();
}

HeartbeatServer::~HeartbeatServer() {
    stopServer();
    m_checkerThread->quit();
    m_checkerThread->wait();
    delete m_checker;
}

bool HeartbeatServer::startServer(quint16 port) {
    if (m_server->isListening()) {
        emit serverStarted(true);
        return true;
    }

    if (!m_server->listen(QHostAddress::AnyIPv6, port)) { // 绑定到 IPv6 地址
        emit errorOccurred(m_server->errorString());
        emit serverStarted(false);
        return false;
    }

    emit serverStarted(true);
    return true;
}

void HeartbeatServer::stopServer() {
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

void HeartbeatServer::onNewConnection() {
    while (m_server->hasPendingConnections()) {
        QTcpSocket *socket = m_server->nextPendingConnection();
        connect(socket, &QAbstractSocket::disconnected, this, [this, socket]() {
            onClientDisconnected(socket);
        });
        connect(socket, &QAbstractSocket::readyRead, this, [this, socket]() {
            onReadyRead(socket);
        });
    }
}

void HeartbeatServer::onClientDisconnected(QTcpSocket *socket) {
    if (!socket)
        return;

    if (m_deviceSockets.contains(socket)) {
        QString deviceId = m_deviceSockets.value(socket);
        m_deviceSockets.remove(socket);
        m_lastHeartbeats.remove(deviceId);
        emit deviceDisconnected(deviceId);
    } else {
        QString deviceId = m_clientSockets.key(socket);
        if (!deviceId.isEmpty()) {
            m_clientSockets.remove(deviceId);
            emit deviceDisconnected(deviceId);
        }
    }

    socket->deleteLater();
}

void HeartbeatServer::onReadyRead(QTcpSocket *socket) {
    QByteArray data = socket->readAll();
    QDataStream stream(data);
    QString messageType, deviceId;
    QDateTime timestamp;

    stream >> messageType;

    if (messageType == "HEARTBEAT") {
        stream >> deviceId >> timestamp;

        if (!m_deviceSockets.contains(socket)) {
            m_deviceSockets.insert(socket, deviceId);
            emit deviceConnected(deviceId);
        }

        m_lastHeartbeats[deviceId] = timestamp;
        emit heartbeatReceived(deviceId, timestamp);

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
            QTcpSocket *oldSocket = m_clientSockets[deviceId];
            oldSocket->disconnectFromHost();
            m_clientSockets[deviceId] = socket;
        }
    }
}

void HeartbeatServer::handleDeviceTimeout(const QString &deviceId) {
    emit deviceDisconnected(deviceId);

    if (m_clientSockets.contains(deviceId)) {
        QTcpSocket *clientSocket = m_clientSockets[deviceId];
        m_clientSockets.remove(deviceId);
        clientSocket->disconnectFromHost();
    }

    if (m_deviceSockets.key(deviceId) != nullptr) {
        QTcpSocket *deviceSocket = m_deviceSockets.key(deviceId);
        m_deviceSockets.remove(deviceSocket);
        deviceSocket->disconnectFromHost();
    }

    m_lastHeartbeats.remove(deviceId);
}

void HeartbeatServer::setHeartbeatTimeoutInterval(int interval) {
    m_heartbeatTimeoutInterval = interval;
}

void HeartbeatServer::setHeartbeatCheckInterval(int interval) {
    m_heartbeatCheckInterval = interval;
}
