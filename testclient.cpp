#include <QCoreApplication>
#include <QDebug>
#include "heartbeatclient.h"

#define HeartbeatClientDebugOn

void testStatusMessageSlot(const QString &msg) {
#ifdef HeartbeatClientDebugOn
    qDebug() << "Test slot received status message:" << msg;
#endif
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    if (argc < 4) {
#ifdef HeartbeatClientDebugOn
        qDebug() << "Usage: testclient <host> <port> <deviceId>";
#endif
        return 1;
    }

    HeartbeatClient client;

    QObject::connect(&client, &HeartbeatClient::connectionStatusChanged, [](bool connected) {
#ifdef HeartbeatClientDebugOn
        qDebug() << "Client connection status:" << (connected? "Connected" : "Disconnected");
#endif
    });

    QObject::connect(&client, &HeartbeatClient::errorOccurred, [](const QString &error) {
#ifdef HeartbeatClientDebugOn
        qDebug() << "Client error:" << error;
#endif
    });

    QObject::connect(&client, &HeartbeatClient::heartbeatReceived, [](qint64 timestamp) {
#ifdef HeartbeatClientDebugOn
        qDebug() << "Client heartbeat received at:" << QDateTime::fromMSecsSinceEpoch(timestamp).toString();
#endif
    });

    QObject::connect(&client, &HeartbeatClient::connectionTimeout, []() {
#ifdef HeartbeatClientDebugOn
        qDebug() << "Client connection timeout!";
#endif
    });

    QObject::connect(&client, &HeartbeatClient::statusMessage,
                     testStatusMessageSlot);

    QString host = argv[1];
    QString portStr = argv[2];
    quint16 port = portStr.toUShort();
    QString deviceId = argv[3];

    client.connectToServer(host, port, deviceId);

    return a.exec();
}
