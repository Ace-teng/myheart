#include <QCoreApplication>
#include <QDebug>
#include "heartbeatclient.h"
#define ResourcesUsageChartDebugOn

// 测试槽函数
void testStatusMessageSlot(const QString &msg) {
#ifdef DEBUG
    qDebug() << "Test slot received status message:" << msg;
#endif
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    if (argc < 4) {
#ifdef DEBUG
        qDebug() << "Usage: testclient <host> <port> <deviceId>";
#endif
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

    // 连接到测试槽函数
    QObject::connect(&client, &HeartbeatClient::statusMessage, testStatusMessageSlot);

    QString host = argv[1];
    QString portStr = argv[2];
    quint16 port = portStr.toUShort();
    QString deviceId = argv[3];

    client.connectToServer(host, port, deviceId);

    return a.exec();
}
