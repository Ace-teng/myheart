#include <QCoreApplication>
#include <QDebug>
#include "heartbeatdevice.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 3) {
        qDebug() << "Usage: testdevice <host> <port>";
        return 1;
    }

    // 修正：将 char* 转换为 QString
    QString host = argv[1];
    QString portStr = argv[2];
    quint16 port = portStr.toUShort();

    HeartbeatDevice device;

    QObject::connect(&device, &HeartbeatDevice::connectionStatusChanged, [](bool connected) {
        qDebug() << "Device connection status:" << (connected? "Connected" : "Disconnected");
    });

    QObject::connect(&device, &HeartbeatDevice::errorOccurred, [](const QString &error) {
        qDebug() << "Device error:" << error;
    });

    QObject::connect(&device, &HeartbeatDevice::heartbeatSent, [](qint64 timestamp) {
        qDebug() << "Device heartbeat sent at:" << QDateTime::fromMSecsSinceEpoch(timestamp).toString();
    });

    device.connectToServer(host, port);

    return a.exec();
}
