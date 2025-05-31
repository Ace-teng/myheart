#include <QCoreApplication>
#include <QDebug>
#include "heartbeatserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 2) {
        qDebug() << "Usage: testserver <port>";
        return 1;
    }

    // 修正：将 char* 转换为 QString 后再调用 toUShort()
    QString portStr = argv[1];
    quint16 port = portStr.toUShort();

    HeartbeatServer server;

    QObject::connect(&server, &HeartbeatServer::serverStarted, [](bool success) {
        qDebug() << "Server started:" << (success? "Yes" : "No");
    });

    QObject::connect(&server, &HeartbeatServer::deviceConnected, [](const QString &deviceId) {
        qDebug() << "Device connected:" << deviceId;
    });

    QObject::connect(&server, &HeartbeatServer::deviceDisconnected, [](const QString &deviceId) {
        qDebug() << "Device disconnected:" << deviceId;
    });

    QObject::connect(&server, &HeartbeatServer::heartbeatReceived, [](const QString &deviceId, qint64 timestamp) {
        qDebug() << "Heartbeat received from" << deviceId << "at" << QDateTime::fromMSecsSinceEpoch(timestamp).toString();
    });

    QObject::connect(&server, &HeartbeatServer::errorOccurred, [](const QString &error) {
        qDebug() << "Server error:" << error;
    });

    if (!server.startServer(port)) {
        return 1;
    }

    return a.exec();
}
