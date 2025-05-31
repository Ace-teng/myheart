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
#ifdef DEBUG
        qDebug() << "Server started:" << (success? "Yes" : "No");
#endif
    });

    QObject::connect(&server, &HeartbeatServer::deviceConnected, [](const QString &deviceId) {
#ifdef DEBUG
        qDebug() << "Device connected:" << deviceId;
#endif
    });

    QObject::connect(&server, &HeartbeatServer::deviceDisconnected, [](const QString &deviceId) {
#ifdef DEBUG
        qDebug() << "Device disconnected:" << deviceId;
#endif
    });

    QObject::connect(&server, &HeartbeatServer::heartbeatReceived, [](const QString &deviceId, qint64 timestamp) {
#ifdef DEBUG
        qDebug() << "Heartbeat received from" << deviceId << "at" << QDateTime::fromMSecsSinceEpoch(timestamp).toString();
#endif
    });

    QObject::connect(&server, &HeartbeatServer::errorOccurred, [](const QString &error) {
#ifdef DEBUG
        qDebug() << "Server error:" << error;
#endif
    });

    if (!server.startServer(port)) {
        return 1;
    }

    return a.exec();
}
