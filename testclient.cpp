#include <QCoreApplication>
#include <QDebug>
#include "heartbeatclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 4)
    {
        qDebug() << "Usage: testclient <host> <port> <deviceId>";
        return 1;
    }

    HeartbeatClient client;

    QObject::connect(&client, &HeartbeatClient::connectionStatusChanged, [](bool connected) {
#ifdef DEBUG
        qDebug() << "Client connection status:" << (connected? "Connected" : "Disconnected");
#endif
    });

    QObject::connect(&client, &HeartbeatClient::errorOccurred, [](const QString &error) {
#ifdef DEBUG
        qDebug() << "Client error:" << error;
#endif
    });

    QObject::connect(&client, &HeartbeatClient::heartbeatReceived, [](qint64 timestamp) {
#ifdef DEBUG
        qDebug() << "Client heartbeat received at:" << QDateTime::fromMSecsSinceEpoch(timestamp).toString();
#endif
    });

    QObject::connect(&client, &HeartbeatClient::connectionTimeout, []() {
#ifdef DEBUG
        qDebug() << "Client connection timeout!";
#endif
    });

    // 连接 statusMessage 信号并输出消息
    QObject::connect(&client, &HeartbeatClient::statusMessage, [](const QString &msg) {
#ifdef DEBUG
        qDebug() << "Client status message:" << msg;
#endif
    });

    QString host = argv[1];
    QString portStr = argv[2];
    quint16 port = portStr.toUShort();
    QString deviceId = argv[3];

    client.connectToServer(host, port, deviceId);

    return a.exec();
}
