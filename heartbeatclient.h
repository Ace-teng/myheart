#ifndef HEARTBEATCLIENT_H
#define HEARTBEATCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>

class HeartbeatClient : public QObject
{
    Q_OBJECT
public:
    explicit HeartbeatClient(QObject *parent = nullptr);
    ~HeartbeatClient();

    void connectToServer(const QString &host, quint16 port, const QString &deviceId);
    void disconnectFromServer();

    QString deviceId() const { return m_deviceId; }

signals:
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);
    void heartbeatReceived(qint64 timestamp);
    void connectionTimeout();
    void statusMessage(const QString &msg); // 新增的携带 QString 消息的信号

private slots:
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    void checkConnection();

private:
    QTcpSocket *m_socket;
    QTimer *m_connectionTimer;
    QString m_deviceId;
    bool m_connected;
    qint64 m_lastHeartbeat;
};

#endif // HEARTBEATCLIENT_H
