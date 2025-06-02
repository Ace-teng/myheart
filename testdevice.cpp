#include <QCoreApplication>
#include <QDebug>
#include "heartbeatdevice.h"
#define HeartbeatDevicesDebugOn

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 3) {
#ifdef HeartbeatDevicesDebugOn
        qDebug() << "Usage: testdevice <host> <port>";
#endif
        return 1;
    }

    // 修正：将 char* 转换为 QString
    QString host = argv[1];
    QString portStr = argv[2];
    quint16 port = portStr.toUShort();

    HeartbeatDevice device;

    QObject::connect(&device, &HeartbeatDevice::connectionStatusChanged, [](bool connected) {
#ifdef HeartbeatDevicesDebugOn
        qDebug() << "Device connection status:" << (connected? "Connected" : "Disconnected");
#endif
    });

    QObject::connect(&device, &HeartbeatDevice::errorOccurred, [](const QString &error) {
#ifdef HeartbeatDevicesDebugOn
        qDebug() << "Device error:" << error;
#endif
    });

    QObject::connect(&device, &HeartbeatDevice::heartbeatSent, [](qint64 timestamp) {
#ifdef HeartbeatDevicesDebugOn
        qDebug() << "Device heartbeat sent at:" << QDateTime::fromMSecsSinceEpoch(timestamp).toString();
#endif
    });

    device.connectToServer(host, port);

    return a.exec();
}
