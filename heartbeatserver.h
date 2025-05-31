#ifndef HEARTBEATSERVER_H
#define HEARTBEATSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QDateTime>

class HeartbeatServer : public QObject
{
    Q_OBJECT
public:
    explicit HeartbeatServer(QObject *parent = nullptr);
    ~HeartbeatServer();

    bool startServer(quint16 port);
    void stopServer();

signals:
    void serverStarted(bool success);
    void serverStopped();
    void deviceConnected(const QString &deviceId);
    void deviceDisconnected(const QString &deviceId);
    void heartbeatReceived(const QString &deviceId, qint64 timestamp);
    void errorOccurred(const QString &error);

private slots:
    void onNewConnection();
    void onClientDisconnected(QTcpSocket *socket);
    void onReadyRead(QTcpSocket *socket);

private:
    QTcpServer *m_server;
    QMap<QTcpSocket*, QString> m_deviceSockets; // socket -> deviceId
    QMap<QString, QTcpSocket*> m_clientSockets; // deviceId -> socket
    QMap<QString, qint64> m_lastHeartbeats;     // deviceId -> timestamp
};

#endif // HEARTBEATSERVER_H
