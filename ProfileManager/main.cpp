#include "dialogmain.h"

#include <QApplication>
#include <QStyleFactory>

#ifdef Q_OS_WINDOWS
#include <Windows.h>
#pragma comment(lib, "advapi32")
#endif

#include "singleapplication/singleapplication.h"

int main(int argc, char *argv[])
{
    SingleApplication app( argc, argv );
    app.setQuitOnLastWindowClosed(false);

    qApp->setStyle(QStyleFactory::create("Fusion"));

    QStringList cmdline_args = QCoreApplication::arguments();

    bool bHidden = cmdline_args.indexOf("-h") != -1;
    DialogMain w;   
    if(bHidden)
        w.hide();
    else
       w.show();
    return app.exec();
}
