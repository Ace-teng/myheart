#ifndef HEARTBEATCLIENT_H
#define HEARTBEATCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>

class HeartbeatClient : public QObject {
    Q_OBJECT
public:
    explicit HeartbeatClient(QObject *parent = nullptr);
    ~HeartbeatClient();

    void connectToServer(const QString &host, quint16 port, const QString &deviceId);
    void disconnectFromServer();
    void setConnectionTimeoutInterval(int interval); // 接口化超时时间
    void setReconnectInterval(int interval);         // 接口化重连间隔

    QString deviceId() const { return m_deviceId; }

signals:
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);
    void heartbeatReceived(const QDateTime &timestamp);
    void connectionTimeout();
    void statusMessage(const QString &msg);

private slots:
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    void checkConnection();
    void attemptReconnect();

private:
    QTcpSocket *m_socket;
    QTimer *m_connectionTimer;
    QTimer *m_reconnectTimer; // 新增重连定时器
    QString m_deviceId;
    bool m_connected;
    QDateTime m_lastHeartbeat;
    int m_reconnectAttempts;
    int m_connectionTimeoutInterval;
    int m_reconnectInterval; // 重连间隔
};

#endif // HEARTBEATCLIENT_H
