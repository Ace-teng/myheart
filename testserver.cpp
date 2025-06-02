#include <QCoreApplication>
#include <QDebug>
#include "heartbeatserver.h"

#define HeartbeatServerDebugOn

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    if (argc < 2) {
#ifdef HeartbeatServerDebugOn
        qDebug() << "Usage: testserver <port>";
#endif
        return 1;
    }

    QString portStr = argv[1];
    quint16 port = portStr.toUShort();

    HeartbeatServer server;

    QObject::connect(&server, &HeartbeatServer::serverStarted, [](bool success) {
#ifdef HeartbeatServerDebugOn
        qDebug() << "Server started:" << (success ? "Yes" : "No");
#endif
    });

    QObject::connect(&server, &HeartbeatServer::deviceConnected, [](const QString &deviceId) {
#ifdef HeartbeatServerDebugOn
        qDebug() << "Device connected:" << deviceId;
#endif
    });

    QObject::connect(&server, &HeartbeatServer::deviceDisconnected, [](const QString &deviceId) {
#ifdef HeartbeatServerDebugOn
        qDebug() << "Device disconnected:" << deviceId;
#endif
    });

    QObject::connect(&server, &HeartbeatServer::heartbeatReceived, [](const QString &deviceId, const QDateTime &timestamp) {
#ifdef HeartbeatServerDebugOn
        qDebug() << "Heartbeat received from" << deviceId << "at" << timestamp.toString(Qt::ISODate);
#endif
    });

    QObject::connect(&server, &HeartbeatServer::errorOccurred, [](const QString &error) {
#ifdef HeartbeatServerDebugOn
        qDebug() << "Server error:" << error;
#endif
    });

    server.setHeartbeatTimeoutInterval(20000); // 设置心跳超时时间为 20 秒
    if (!server.startServer(port)) {
        return 1;
    }

    return a.exec();
}
