#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <iostream>

#include "backend.h"
#include "tcpsocket.h"

const char *g;

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QStringList args = app.arguments();
    if (args.count() != 2)
      {

          std::cerr << "1st argument (IP Address) along executable file name is required '\n'" << endl;
          std::cerr << "\n For Example: ./test2 10.102.1.127 \n" << endl;
          return 1;
      }

    g = argv[1];
/*
    std::cout << "argv[0]=" << argv[0] <<  "argv[1]=" << argv[1]  << std::endl;
    std::cout<< "ip address = " <<g << '\n';
*/




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
