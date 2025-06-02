#ifndef HEARTBEATDEVICE_H
#define HEARTBEATDEVICE_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>

class HeartbeatDevice : public QObject {
    Q_OBJECT
public:
    explicit HeartbeatDevice(QObject *parent = nullptr);
    ~HeartbeatDevice();

    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    void setHeartbeatInterval(int interval); // 接口化心跳间隔

signals:
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);
    void heartbeatSent(const QDateTime &timestamp);

private slots:
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void sendHeartbeat();
    void attemptReconnect();

private:
    QTcpSocket *m_socket;
    QTimer *m_heartbeatTimer;
    QString m_deviceId;
    bool m_connected;
    int m_reconnectAttempts;
    int m_heartbeatInterval; // 心跳间隔
};

#endif // HEARTBEATDEVICE_H
