#include "mainwindow.h"
#include "server.h"
#include <QApplication>
#include <QObject>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    Server server;
    QObject::connect(&server, SIGNAL(newUser(QString)), &w, SLOT(newUser(QString)));
    QObject::connect(&server, SIGNAL(newSolution(QString)), &w, SLOT(newSolution(QString)));

    w.show();

    return a.exec();
}
