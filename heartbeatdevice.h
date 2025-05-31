#ifndef HEARTBEATDEVICE_H
#define HEARTBEATDEVICE_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>

class HeartbeatDevice : public QObject
{
    Q_OBJECT
public:
    explicit HeartbeatDevice(QObject *parent = nullptr);
    ~HeartbeatDevice();

    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();

signals:
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);
    void heartbeatSent(qint64 timestamp);

private slots:
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void sendHeartbeat();

private:
    QTcpSocket *m_socket;
    QTimer *m_heartbeatTimer;
    QString m_deviceId;
    bool m_connected;
};

#endif // HEARTBEATDEVICE_H
