#ifndef HEARTBEATSERVER_H
#define HEARTBEATSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QDateTime>
#include <QThread>

class HeartbeatChecker : public QObject {
    Q_OBJECT
public:
    explicit HeartbeatChecker(QObject *parent = nullptr);

public slots:
    void checkHeartbeats(const QMap<QString, QDateTime> &heartbeats);

signals:
    void deviceTimedOut(const QString &deviceId);
};

class HeartbeatServer : public QObject {
    Q_OBJECT
public:
    explicit HeartbeatServer(QObject *parent = nullptr);
    ~HeartbeatServer();

    bool startServer(quint16 port);
    void stopServer();
    void setHeartbeatTimeoutInterval(int interval);
    void setHeartbeatCheckInterval(int interval);

signals:
    void serverStarted(bool success);
    void serverStopped();
    void deviceConnected(const QString &deviceId);
    void deviceDisconnected(const QString &deviceId);
    void heartbeatReceived(const QString &deviceId, const QDateTime &timestamp);
    void errorOccurred(const QString &error);

private slots:
    void onNewConnection();
    void onClientDisconnected(QTcpSocket *socket);
    void onReadyRead(QTcpSocket *socket);
    void handleDeviceTimeout(const QString &deviceId);

private:
    QTcpServer *m_server;
    QMap<QTcpSocket *, QString> m_deviceSockets;
    QMap<QString, QTcpSocket *> m_clientSockets;
    QMap<QString, QDateTime> m_lastHeartbeats;

    int m_heartbeatTimeoutInterval;
    int m_heartbeatCheckInterval;

    QThread *m_checkerThread; // 心跳检查线程
    HeartbeatChecker *m_checker; // 心跳检查器
};

#endif // HEARTBEATSERVER_H
