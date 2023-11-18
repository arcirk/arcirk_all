#include "iwebsocketclient.h"
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "query_builder.hpp"
#include "verify_database.hpp"

IWebsocketClient::IWebsocketClient(QObject *parent)
    : WebSocketClient{parent}
{
    sqlDatabase = QSqlDatabase::addDatabase("QSQLITE");
#ifndef Q_OS_WINDOWS
    sqlDatabase.setDatabaseName("pricechecker.sqlite");
#else
    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if(!dir.exists())
        dir.mkpath(path);

    auto fileName= path + "/pricechecker.sqlite";
    sqlDatabase.setDatabaseName(fileName);
#endif

    if (!sqlDatabase.open()) {
        qCritical() << sqlDatabase.lastError().text();
    }


    wsSettings = nullptr;

    m_reconnect = new QTimer(this);
    connect(m_reconnect,SIGNAL(timeout()),this,SLOT(onReconnect()));
    m_tmr_synchronize = new QTimer(this);
    connect(m_tmr_synchronize,SIGNAL(timeout()),this,SLOT(onSynchronize()));

    connect(this, &WebSocketClient::syncGetDiscrepancyInData, this, &IWebsocketClient::onSynchronizeBaseNext);

    startSynchronize();
}

void IWebsocketClient::open(Settings *sett)
{
    if(!sett->url().isValid()){
        emit displayError("Error", "Не задан адрес сервера!");
        return;
    }

    if(isStarted())
        return;

    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    wsSettings = sett;
    conf_.device_id = wsSettings->deviceId().toStdString();
    conf_.app_name = "PriceChecker";
    conf_.user_name = wsSettings->userName().toStdString();
    conf_.hash = wsSettings->hash().toStdString();
    conf_.server_host = wsSettings->url().toString().toStdString();
    m_client->open(wsSettings->url());
}

void IWebsocketClient::onReconnect()
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;
    if(isStarted()){
        if(m_reconnect->isActive())
            m_reconnect->stop();
    }else{
        m_async_await.append(std::bind(&IWebsocketClient::reconnect, this));
        asyncAwait();
    }
}

void IWebsocketClient::checkConnection()
{
    if(!m_reconnect->isActive())
        startReconnect();
}

void IWebsocketClient::reconnect()
{
    open(wsSettings);
}

void IWebsocketClient::set_new_version_document(const QString &ref)
{
    using namespace arcirk::database;

    //int count = 0;
    QSqlQuery rs;
    auto br = builder::query_builder();
    rs.exec(br.select().from(arcirk::enum_synonym(arcirk::database::tables::tbDocuments)).where(json{{"ref", ref.toStdString()}}, true).prepare().c_str());
    int version = 1;
    while (rs.next()) {
        version += rs.value("version").toInt();
    }
    rs.clear();
    br.clear();
    br.use(nlohmann::json{
        {"version", version}
    });
    rs.exec(br.update(arcirk::enum_synonym(arcirk::database::tables::tbDocuments), true).where(json{{"ref", ref.toStdString()}}, true).prepare().c_str());

}

bool IWebsocketClient::barcode_info_from_1c_service(const QString &barcode, BarcodeInfo *bInfo)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    if(wsSettings->workplace_options().warehouse.empty() || wsSettings->workplace_options().price_type.empty()){
        const QString err = "Не инициализированы настройки рабочего места!";
        qCritical() << err;
        emit displayError("WebSocketClient::get_barcode_inf_from_http_service", err);
        return false;
    }

    if(wsSettings->getHttpService().isEmpty()){
        qCritical() << "HTTP сервис 1С предприятия не указан!";
        emit displayError("Ошибка", "HTTP сервис 1С предприятия не указан!");
        return false;
    }

    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;
    QString error_string;

    auto finished = [&loop, &httpStatus, &httpData, &error_string](QNetworkReply* reply) -> void
    {
       QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
       if(status_code.isValid()){
           httpStatus = status_code.toInt();
           if(httpStatus != 200){
               error_string = reply->errorString();
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + error_string ;
           }else
           {
               httpData = reply->readAll();
           }
       }
       loop.quit();

    };

    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

    QNetworkRequest request(wsSettings->getHttpService() + "/info");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString pwd = crypt(wsSettings->getHttpPassword(), "my_key").c_str();
    QString concatenated = wsSettings->getHttpUser() + ":" + pwd;
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    auto bi = arcirk::client::barcode_info();

    nlohmann::json param = {
        {"barcode", barcode.toStdString()},
        {"command", "BarcodeInfo"},
        {"barcode_info", pre::json::to_json<arcirk::client::barcode_info>(bi)},
        {"price", wsSettings->workplace_options().price_type},
        {"warehouse", wsSettings->workplace_options().warehouse},
        {"image_data", false} //wsSettings->isShowImage() тормозит
    };

    httpService.post(request, QByteArray::fromStdString(param.dump()));
    loop.exec();

    if(httpStatus != 200){
        if(!error_string.isEmpty())
            emit displayError("Ошибка", error_string);
        return false;
    }

    if(httpData.isEmpty())
        return false;

    if(httpData == "error"){
         bInfo->clear("Ошибка чтения штрихкода");
         emit displayError("get_barcode_inf_from_http_service", "Не верный формат данных.");
         return false;
    }

    try {
        bInfo->set_barcode_info_object(httpData.toStdString());
    } catch (const std::exception& e) {
        qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << e.what();
        emit displayError("get_barcode_inf_from_http_service", "Не верный формат данных.");
        bInfo->clear("Ошибка чтения штрихкода");
        return false;
    }

    return true;
}

void IWebsocketClient::init_data_options()
{
    if(!wsSettings)
        return;


    auto ref = wsSettings->deviceId();
    if(ref.isEmpty())
        return;

    auto query_param = nlohmann::json{
        {"table_name", arcirk::enum_synonym(database::tbDevices)},
        {"where_values", json{{"ref", ref.toStdString()}}},
        {"query_type", "select"},
        {"values", json{}}
    };

    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
    auto resp = exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                                                                                            {"query_param", base64_param}
                                                                                                        });

    if(resp  != WS_RESULT_ERROR){
        auto rows = resp.value("rows", json{});
        if(rows.size() > 0){
            auto item = rows[0];
            if(item.is_object()){
                std::string id = item.value("ref","");
                if(wsSettings->deviceId().toStdString() == id){
                    wsSettings->update_workplace_data(item);
                }
            }
        }
    }else{
        qCritical() << __FUNCTION__ << resp.get<std::string>().c_str();
        return;
    }

    query_param = json{
        {"table_name", arcirk::enum_synonym(database::dvDevicesView)},
        {"where_values", json{{"ref", ref.toStdString()}}},
        {"query_type", "select"},
        {"values", json{}}
    };

    base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
    resp = exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                             {"query_param", base64_param}
                                             });

    if(resp  != WS_RESULT_ERROR){
        auto rows = resp.value("rows", json{});
        if(rows.size() > 0){
            auto item = rows[0];
            if(item.is_object()){
                std::string id = item.value("ref","");
                if(wsSettings->deviceId().toStdString() == id){
                    wsSettings->update_workplace_view(item);
                }
            }
        }
    }else{
        qCritical() << __FUNCTION__ << resp.get<std::string>().c_str();
    }
}

QString IWebsocketClient::documentDate(const int value) const
{
    if(value > 0){
        return QDateTime::fromSecsSinceEpoch(value).toString("dd.MM.yyyy hh:mm:ss");
    }else{
        return {};
    }
}

void IWebsocketClient::setQrCode(const QString &qr, const QString &doc_ref, const QString &row_ref, tree_model::IQMLTreeItemModel *model)
{
    using namespace arcirk::database;

    auto ind = model->getColumnIndex("qr_code");
    if(ind == -1)
        return;

    auto row = model->findInTable(qr, ind, false);
    if(row.isValid()){
        emit displayError("Подбор", "QR уже есть в списке!");
    }else{
        if(doc_ref.isEmpty() || row_ref.isEmpty()){
            emit displayError("Qr код", "Ошибка в данных!");
            return;
        }
        int count = 0;
        auto str = builder::query_builder().row_count().from(arcirk::enum_synonym(tables::tbDocumentsMarkedTables)).where(json{
                                                                                                                              {"qr_code", qr.toStdString()},
                                                                                                                              {"document_ref", doc_ref.toStdString()}
                                                                                                                          }, true).prepare();
        QSqlQuery rs(str.data());
        rs.exec();
        while (rs.next()) {
            count = rs.value(0).toInt();
        }

        if(count > 0){
            displayError("Ошибка", "QR Код уже подобран к другой позиции номенклатуры!");
            return;
        }

        auto tmp = document_marked_table();
        tmp.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        tmp.parent = row_ref.toStdString();
        tmp.document_ref = doc_ref.toStdString();
        tmp.qr_code = qr.toStdString();

        auto values = pre::json::to_json(tmp);
        auto query = builder::query_builder();
        query.use(values);
        rs.clear();
        rs.exec(query.insert(arcirk::enum_synonym(tables::tbDocumentsMarkedTables), true).prepare().data());
        model->addRow(QString(values.dump().c_str()));
        model->reset();

        set_new_version_document(doc_ref);
    }
}

QString IWebsocketClient::parseQr(const QString &qr)
{
    if(qr.isEmpty())
        return {};

    if(qr.left(4) == "(01)") //короткий
        return qr;

    //    auto on = QChar(1);
    //    auto tw = QChar(17);
    //    auto n = QChar(29);

    //    qDebug() << qr.indexOf(on);
    //    qDebug() << qr.indexOf(tw);
    //    qDebug() << qr.indexOf(n);

    if(qr.length() < 80){
        emit displayError("Ошибка", "Не верный Qr код!");
        return {};
    }

    auto full = qr.left(31);
    auto pref = full.left(2);
    if(pref != "01")
        emit displayError("Ошибка", "Не верный Qr код!");
    auto gtin = full.mid(2, 14);
    auto serial = full.mid(18, 13);

    return "(01)" + gtin + "(21)" + serial;
}

void IWebsocketClient::getDocuments()
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    if(!sqlDatabase.isOpen())
        return;

    using namespace arcirk::database;
    nlohmann::json result{};
    arcirk::database::execute(
        builder::query_builder().select(nlohmann::json{"*"})
            .from(arcirk::enum_synonym(tables::tbDocuments))
            .where(nlohmann::json{
                       {"device_id", wsSettings->deviceId().toStdString()},
                       {"deletion_mark", 0}
                   }, true).prepare(), sqlDatabase, result);
    emit readDocuments(QString::fromStdString(result.dump()));
}

void IWebsocketClient::get_image_data(BarcodeInfo *bInfo)
{
    if(!wsSettings->isShowImage() || bInfo->uuid().empty())
        return;

    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;

    auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
    {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if(status_code.isValid()){
            httpStatus = status_code.toInt();
            if(httpStatus != 200){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
            }else
            {
                httpData = reply->readAll();
            }
        }
        loop.quit();

    };

    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

    QNetworkRequest request(wsSettings->getHttpService() + "/info");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString pwd = crypt(wsSettings->getHttpPassword(), "my_key").c_str();
    QString concatenated = wsSettings->getHttpUser() + ":" + pwd;
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    nlohmann::json param = {
        {"uuid", bInfo->uuid()},
        {"command", "GetImage"}
    };
    httpService.post(request, QByteArray::fromStdString(param.dump()));
    loop.exec();

    if(httpStatus != 200){
        emit bInfo->imageSourceChanged();
        return;
    }

    if(httpData.isEmpty())
        return;

    bInfo->set_image(httpData);


}

void IWebsocketClient::synchronizeBase()
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    auto i = m_vecOperations.indexOf("SynchDocumentsBase");
    if(i != -1){ //уже запущен
        asyncAwait();
        return;
    }

    if(!isStarted()){
        asyncAwait();
        return;
    }

    //Получаем список документов с севрвера
    QUuid devId = QUuid::fromString(wsSettings->deviceId());
    if(devId.isNull()){
        qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Не верный идентификатор устройства!";
        asyncAwait();
        return;
    }

    if(!sqlDatabase.isOpen()){
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "База данных не открыта!";
        asyncAwait();
        return;
    }

    QSqlQuery rs;
    rs.exec("select ref,version from Documents");
    nlohmann::json t_docs{};
    while (rs.next()) {
        t_docs += nlohmann::json{
            {"ref", rs.value("ref").toString().toStdString()},
            {"version", rs.value("version").toInt()}
        };
    };

    std::string base64_param = QByteArray::fromStdString(t_docs.dump()).toBase64().toStdString();

    using namespace arcirk::database;

    send_command(arcirk::server::server_commands::SyncGetDiscrepancyInData, {
                                                                                {"base64_param", base64_param},
                                                                                {"device_id", wsSettings->deviceId().toStdString()},
                                                                                {"table_name", arcirk::enum_synonym(tables::tbDocuments)}
                                                                            });

    asyncAwait();
}

void IWebsocketClient::onSynchronize()
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;
    if(!isStarted())
        return;

    if(m_async_await.size() > 0){
        m_async_await.append(std::bind(&IWebsocketClient::synchronizeBase, this));
    }else
        synchronizeBase();

}

void IWebsocketClient::onSynchronizeBaseNext(const arcirk::server::server_response &resp)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;


    if(resp.result == "error")
        return;

    using namespace arcirk::database;

    if(!sqlDatabase.isOpen())
        return;

    auto result = json::parse(QByteArray::fromBase64(resp.result.data()).toStdString());

}
