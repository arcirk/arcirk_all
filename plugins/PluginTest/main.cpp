#include "maindialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("TestPlugin");
    QCoreApplication::setApplicationName("Star Test");

    MainDialog w;
    w.show();
    return a.exec();
}
