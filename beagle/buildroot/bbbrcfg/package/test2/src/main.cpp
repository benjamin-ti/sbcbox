#include <QGuiApplication>
#include <QQmlApplicationEngine>


#include "backend.h"
#include "tcpsocket.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    qmlRegisterType<BackEnd>("io.qt.examples.backend", 1, 0, "BackEnd");
    qmlRegisterType<TCPSocket>();

/*    QList<QObject*> dataList;
     dataList.append(new DataObject("Item 1", "red"));
     dataList.append(new DataObject("Item 2", "green"));
     dataList.append(new DataObject("Item 3", "blue"));
     dataList.append(new DataObject("Item 4", "yellow"));*/



    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
