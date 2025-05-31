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
        qDebug() << "Client connection status:" << (connected? "Connected" : "Disconnected");
    });

    QObject::connect(&client, &HeartbeatClient::errorOccurred, [](const QString &error) {
        qDebug() << "Client error:" << error;
    });

    QObject::connect(&client, &HeartbeatClient::heartbeatReceived, [](qint64 timestamp) {
        qDebug() << "Client heartbeat received at:" << QDateTime::fromMSecsSinceEpoch(timestamp).toString();
    });

    QObject::connect(&client, &HeartbeatClient::connectionTimeout, []() {
        qDebug() << "Client connection timeout!";
    });

    QString host = argv[1];
    QString portStr = argv[2];
    quint16 port = portStr.toUShort();
    QString deviceId = argv[3];

    client.connectToServer(host, port, deviceId);

    return a.exec();
}
