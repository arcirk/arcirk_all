#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QScreen>
#include <QQuickStyle>
#include <QQmlContext>
#include "src/iwebsocketclient.h"

#include "src/qtandroidservice.h"

#include "iface/iface.hpp"
#include "sort/treesortmodel.h"
#include "src/wssettings.h"
#include "src/notificationqueue.h"
#include "src/barcode_parser.hpp"
#include "src/barcode_info.hpp"

#include <QtSql>
#include "src/verify_database.hpp"

void verifyDatabase(){

    using namespace arcirk::database;

    QSqlDatabase sql = QSqlDatabase::addDatabase("QSQLITE");

#ifndef Q_OS_WINDOWS
    sql.setDatabaseName("pricechecker.sqlite");
#else
    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if(!dir.exists())
        dir.mkpath(path);

    auto fileName= path + "/pricechecker.sqlite";
    sql.setDatabaseName(fileName);
#endif

    if (!sql.open()) {
        qDebug() << sql.lastError().text();
        return;
    }

    verify_database(sql);

    sql.close();

}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    verifyDatabase();

    QQuickStyle::setStyle("Material");

    QQmlApplicationEngine engine;

    qmlRegisterType<IWebsocketClient>("WebSocketClient", 1, 0, "WebSocketClient");
    qmlRegisterType<arcirk::tree_model::IQMLTreeItemModel>("QmlTableModel", 1, 2, "QmlTableModel");
    qmlRegisterType<arcirk::tree_model::TreeSortModel>("QmlSortModel", 1, 0, "QmlSortModel");
    qmlRegisterType<arcirk::Settings>("Settings", 1, 0, "Settings");
    qmlRegisterType<NotificationQueue>("NotificationQueue", 1, 0, "NotificationQueue");
    qmlRegisterType<BarcodeParser>("BarcodeParser", 1, 0, "BarcodeParser");
    qmlRegisterType<BarcodeInfo>("BarcodeInfo", 1, 0, "BarcodeInfo");

#ifdef Q_OS_ANDROID
    QScreen *screen = app.primaryScreen();
    int width = screen->size().width();
    int height = screen->size().height();
    qDebug() << "screen" << width << "x" << height;
#else
    //QScreen *screen = app.primaryScreen();
    int width = 720; //screen->size().width();
    int height = 1280; //screen->size().height();
#endif


    QQmlContext* context = engine.rootContext();
    context->setContextProperty("screenWidth", width);
    context->setContextProperty("screenHeight", height);
    context->setContextProperty("Theme", "Dark");

    QtAndroidService *qtAndroidService = new QtAndroidService(&app);
    context->setContextProperty(QLatin1String("qtAndroidService"), qtAndroidService);
    context->setContextProperty(QLatin1String("machineUniqueId"), qtAndroidService->getUuid());


    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
