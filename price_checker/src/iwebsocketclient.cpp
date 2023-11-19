#include "iwebsocketclient.h"
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "query_builder.hpp"
#include "verify_database.hpp"
#include "synch_operations.hpp"

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
    is_pool_run = false;

    m_reconnect = new QTimer(this);
    connect(m_reconnect,SIGNAL(timeout()),this,SLOT(onReconnect()));
    m_tmr_synchronize = new QTimer(this);
    connect(m_tmr_synchronize,SIGNAL(timeout()),this,SLOT(onSynchronize()));

    connect(this, &WebSocketClient::syncGetDiscrepancyInData, this, &IWebsocketClient::onSynchronizeBaseNext);
    connect(this, &WebSocketClient::updateBarcodeInformation, this, &IWebsocketClient::onUpdateBarcodeInformation);
    connect(this, &WebSocketClient::serverResponse, this, &IWebsocketClient::onServerResponse);

    QThreadPool::globalInstance()->setMaxThreadCount(4);
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

bool IWebsocketClient::get_barcode_info_local(const QString &barcode, BarcodeInfo *bInfo)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    if(!sqlDatabase.isOpen())
        return false;

    if(barcode.isEmpty())
        return false;

    json fields = {
        {"barcode", "barcode"},
        {"parent", "parent"}
    };
    auto left_table = pre::json::to_json(database::nomenclature());
    json join_options = {
        {"parent", "ref"}
    };
    auto query = query_builder().select(fields)
                .from(arcirk::enum_synonym(database::tbBarcodes))
                .join(arcirk::enum_synonym(database::tbNomenclature), left_table, sql_join_type::joinLeft, join_options)
                .where(json{
                    {"barcode", barcode.toStdString()}
                }, true).prepare();

    //qDebug() << qPrintable(query.c_str());

    QSqlQuery rs(query.data());

    //QString table_first_alias = QString(arcirk::enum_synonym(database::tbBarcodes).data()) + "First";
    QString table_second_alias = QString(arcirk::enum_synonym(database::tbNomenclature).data()) + "Second";

    int count = 0;
    auto barcode_inf = arcirk::client::barcode_info();
    while (rs.next()) {
        barcode_inf.barcode = barcode.toStdString();
        QSqlRecord row = rs.record();
        barcode_inf.synonym = row.field(table_second_alias + "_first").value().toString().toStdString();
        barcode_inf.vendor_code = row.field(table_second_alias + "_vendor_code").value().toString().toStdString();
        barcode_inf.uuid = row.field(table_second_alias + "_ref").value().toString().toStdString();
        barcode_inf.trademark = row.field(table_second_alias + "_trademark").value().toString().toStdString();
        barcode_inf.is_marked = row.field(table_second_alias + "_is_marked").value().toString().toInt();
        count++;
    }

    if(count == 0){
        barcode_inf.barcode = barcode.toStdString();
        barcode_inf.unit = "шт";
    }

    bInfo->set_barcode_info(barcode_inf);

    return count > 0;
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

void IWebsocketClient::get_barcode_information(const QString &barcode, BarcodeInfo *bInfo)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    if(!wsSettings){
        qCritical() << "Не инициализирован объект настроек!";
        return;
    }

    auto result = get_barcode_info_local(barcode, bInfo);
    if(!result){
        //штрихкода не существует
        //отправляем асинхронно зарос на сервер для обновления
        if(isStarted()){
            get_barcode_info_from_ws_server_asynh(barcode);
        }
    }

}

void IWebsocketClient::init_data_options()
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

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

void IWebsocketClient::deleteDocument(const QString &ref, const int ver)
{
    int v = ver;

    if(v < 0)
        v = 0;

    if(!sqlDatabase.isOpen())
        return;

    using namespace arcirk::database;
    nlohmann::json result{};
    auto br = builder::query_builder();
    br.use(nlohmann::json{
        {"deletion_mark", 1},
        {"version", v + 1}
    });

    QSqlQuery rs;
    rs.exec(br.update(arcirk::enum_synonym(tables::tbDocuments), true)
                .where(nlohmann::json{
                           {"ref", ref.toStdString()}
                       }, true).prepare().c_str());


}

void IWebsocketClient::documentContentUpdate(BarcodeInfo *bInfo, IQMLTreeItemModel *model, const QString& documentRef)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    Q_ASSERT(bInfo!=0);
    Q_ASSERT(model!=0);
    Q_ASSERT(!documentRef.isEmpty());
    Q_ASSERT(sqlDatabase.isOpen());

    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    if(bInfo->barcode().isEmpty())
        return;

    auto row = arcirk::database::document_table();

    auto i = model->column_index("barcode");
    auto index = model->find(i, QString(bInfo->barcode()));

    if(index.isValid()){
        auto object = model->to_object(index);
        row = secure_serialization<arcirk::database::document_table>(object);
        row.quantity++;
    }else{
        row.barcode = bInfo->barcode().toStdString();
        row.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        row.parent = documentRef.toStdString();
        row.quantity = 1;
        row.first = "Штрихкоды";
        row.second = "Штрихкоды";
    }

    int count = 0;
    QSqlQuery rs(query_builder().row_count().from(arcirk::enum_synonym(arcirk::database::tables::tbDocumentsTables)).where(json{{"ref", row.ref}}, true).prepare().c_str());
    while (rs.next()) {
        count += rs.value(0).toInt();
    }
    rs.clear();
    auto br = query_builder();
    br.use(pre::json::to_json(row));
    if(count > 0){
        rs.exec(br.update(arcirk::enum_synonym(arcirk::database::tables::tbDocumentsTables), true).where(json{{"ref", row.ref}}, true).prepare().c_str());
    }else{
        rs.exec(br.insert(arcirk::enum_synonym(arcirk::database::tables::tbDocumentsTables), true).prepare().c_str());
    }

    rs.clear();
    br.clear();
    rs.exec(br.select().from(arcirk::enum_synonym(arcirk::database::tables::tbDocuments)).where(json{{"ref", row.parent}}, true).prepare().c_str());

    int version = 1;

    while (rs.next()) {
        version += rs.value("version").toInt();
    }
    rs.clear();
    br.clear();
    br.use(nlohmann::json{
        {"version", version}
    });
    rs.exec(br.update(arcirk::enum_synonym(arcirk::database::tables::tbDocuments), true).where(json{{"ref", row.parent}}, true).prepare().c_str());

    rs.clear();

    rs.exec(query_builder().select().from(arcirk::enum_synonym(arcirk::database::views::dvDocumentsTableView)).where(nlohmann::json{{"ref", row.ref}}, true).prepare().c_str());

    json row_obj;
    if(rs.next())
        row_obj = arcirk::database::get_json_object(rs);
    else{
        qCritical() << rs.lastError().text();
        return;
    }
    if(index.isValid()){
        model->updateRow(row_obj, index);
        if(index.row() != 0)
            model->moveTop(index);
    }else{
        model->insertRow(0, row_obj);
    }

    model->setCurrentRow(0);
    model->reset();
}

void IWebsocketClient::documentModelUpdateItem(BarcodeInfo *bInfo, IQMLTreeItemModel *model)
{

    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    auto index = model->find(model->column_index("barcode"), bInfo->barcode());
    if(index.isValid()){

        if(!sqlDatabase.isOpen()){
            bInfo->setUpdate(false);
            return;
        }

        auto object = model->to_object(index);

        auto item = secure_serialization<database::document_table>(object);
        item.product = bInfo->synonym().toStdString();
        item.vendor_code = bInfo->vendor_code().toStdString();

        using namespace arcirk::database;
        json result{};
        auto query = builder::query_builder();
        query.use(pre::json::to_json(item));

        QSqlQuery rs(query.update(enum_synonym(tables::tbDocumentsTables), true).where(json{{"ref", item.ref}}, true).prepare().c_str());
        rs.exec();

        query.clear();
        rs.clear();

        std::string query_str = query.select()
                                    .from(arcirk::enum_synonym(views::dvDocumentsTableView))
                                    .where(nlohmann::json{
                                               {"ref", item.ref}
                                           }, true).prepare();
        arcirk::database::execute(query_str
                                  , sqlDatabase, result);

        auto rows = result.value("rows", json::array());

        if(rows.size() > 0)
            model->set_object(index, rows[0]);

        bInfo->setUpdate(false);

        model->reset();
    }

}

void IWebsocketClient::documentTableSetQuantity(const QString &ref, int quantity, tree_model::IQMLTreeItemModel *model)
{
    using namespace arcirk::database;
    auto index = model->find(QUuid::fromString(ref));
    if(index.isValid()){
        auto query = query_builder();
        query.use(json{{"quantity", quantity}});
        QSqlQuery rs(query.update(enum_synonym(tables::tbDocumentsTables), true).where(json{{"ref", ref.toStdString()}}, true).prepare().c_str());
        rs.exec();
        if(rs.lastError().type() == QSqlError::NoError){
            auto object = model->to_object(index);
            object["quantity"] = quantity;
            model->set_object(index, object);
            model->reset();
        }
    }

}

void IWebsocketClient::setQrCode(const QString &qr, const QString &doc_ref, const QString &row_ref, tree_model::IQMLTreeItemModel *model)
{
    using namespace arcirk::database;

    auto ind = model->getColumnIndex("qr_code");
    if(ind == -1)
        return;

    auto row = model->findInTable(qr, ind);
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

void IWebsocketClient::getDocumentContent(const QString &ref)
{
    // || \", кол.: \" || CAST(%3 AS INTEGER)
    if(!sqlDatabase.isOpen())
        return;

    using namespace arcirk::database;
    nlohmann::json result{};
    auto query = builder::query_builder();

    std::string query_str = query.select()
                                .from(arcirk::enum_synonym(views::dvDocumentsTableView))
                                .where(nlohmann::json{
                                           {"parent", ref.toStdString()}
                                       }, true).prepare();
    arcirk::database::execute(query_str
                              , sqlDatabase, result);

    if(result["columns"].size() == 1){
        auto table_info = arcirk::database::table_info(sqlDatabase, arcirk::database::views::dvDocumentsTableView);
        for(auto itr = table_info.begin(); itr != table_info.end();  ++itr){
            std::string col = itr.key();
            result["columns"] += col;
        }

        result["rows"] = nlohmann::json{};
    }
    std::string d = result.dump();

    emit readDocumentTable(QString::fromStdString(d));

    verifyDocument(ref);

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

int IWebsocketClient::dataMax()
{
    if(!sqlDatabase.isOpen())
        return 0;

    QSqlQuery rc(QString("select max(_id) as max from Documents where device_id = '%1'").arg(wsSettings->deviceId()));
    int m_max = 0;
    while (rc.next()) {
        m_max = rc.value("max").toInt();
    }
    return m_max;
}

void IWebsocketClient::verifyDocument(const QString &documentRef)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    if(documentRef.isEmpty() || !isStarted() || !sqlDatabase.isOpen())
        return;

    QSqlQuery rs(QString("select * FROM DocumentsTableView where nomenclature = '' AND parent = '%1'").arg(documentRef));

    auto m_vec = json::array();
    while (rs.next()) {
        m_vec += rs.value("barcode").toString().toStdString();
    }

    if(m_vec.size() == 0)
        return;


    json fields = {
        {"barcode", "barcode"},
        {"parent", "parent"}
    };
    auto left_table = pre::json::to_json(database::nomenclature());
    json join_options = {
        {"parent", "ref"}
    };
    auto query = query_builder().select(fields)
                     .from(arcirk::enum_synonym(database::tbBarcodes))
                     .join(arcirk::enum_synonym(database::tbNomenclature), left_table, sql_join_type::joinLeft, join_options)
                     .where(json{
                                {"barcode", m_vec}
                            }, true).prepare();


    auto param = json{
        {"table_name", "barcode_info"}
    };

    std::string query_param = QByteArray::fromStdString(param.dump()).toBase64().toStdString();
    send_command(arcirk::server::server_commands::ExecuteSqlQuery, {
                                                                       {"query_text", query},
                                                                       {"query_param", query_param}
                                                                   });

}

void IWebsocketClient::removeRow(const QString &ref, IQMLTreeItemModel *model)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    if(!sqlDatabase.isOpen()){
        qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__  << "База данных не подключена!";
        return;
    }
    using namespace arcirk::database;
    auto query = builder::query_builder();

    std::string table_name;

    QSqlQuery rs;
    std::string parent_field = "parent";

    if(model->currentObjectName() == QString::fromStdString(arcirk::enum_synonym(tables::tbDocumentsTables))){
        table_name = arcirk::enum_synonym(tables::tbDocumentsTables);
    }if(model->currentObjectName() == QString::fromStdString(arcirk::enum_synonym(tables::tbDocumentsMarkedTables))){
        table_name = arcirk::enum_synonym(tables::tbDocumentsMarkedTables);
        parent_field = "document_ref";
    }

    rs.exec(query.remove().from(table_name).where(nlohmann::json{
                                                      {"ref", ref.toStdString()}
                                                  }, true).prepare().c_str());

    auto index = model->find(QUuid::fromString(ref));

    if(index.isValid()){
        auto object = model->to_object(index);
        std::string parent =  object[parent_field].get<std::string>().c_str();
        rs.clear();
        query.clear();
        rs.exec(query.select().from(arcirk::enum_synonym(arcirk::database::tables::tbDocuments)).where(json{{"ref", parent}}, true).prepare().c_str());
        int version = 1;
        while (rs.next()) {
            version += rs.value("version").toInt();
        }
        rs.clear();
        query.clear();
        query.use(json{
            {"version", version}
        });
        rs.exec(query.update(arcirk::enum_synonym(tables::tbDocuments), true).where(json{{"ref", parent}}, true).prepare().c_str());
        model->remove(index);
    }

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

    arcirk::synchronize::SynchDocumentsBase * syncClass;
    SynhOperations< arcirk::synchronize::SynchDocumentsBase> * runClass;

    syncClass = new arcirk::synchronize::SynchDocumentsBase(wsSettings, this);
    runClass = new SynhOperations<arcirk::synchronize::SynchDocumentsBase>(syncClass);
    synh_lambda(syncClass);

    auto result = nlohmann::json::parse(QByteArray::fromBase64(resp.result.data()).toStdString());

    syncClass->setComparisonOfDocuments(result);

    QThreadPool::globalInstance()->setExpiryTimeout(-1);

    QThreadPool::globalInstance()->start(runClass);
    is_pool_run = true;
    QThreadPool::globalInstance()->waitForDone();

}

void IWebsocketClient::synh_lambda(arcirk::synchronize::SynchronizeBase * syncClass)
{
    using namespace arcirk::synchronize;

    connect(syncClass, &SynchronizeBase::startSynchronize, this, [&](const QString& operationName){
        //qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "startSynchronize";
        emit startAsyncSynchronize(operationName);
    });

    connect(syncClass, &SynchronizeBase::endSynchronize, this, [&](const QString& operationName, bool result, const nlohmann::json& details){
        if(!result || details.empty() || !isStarted()){
            emit endAsyncSynchronize(operationName);
            //qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "endSynchronize" << result;
            return;
        }

        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "endSynchronize" << result;

        if(!details.empty()){
            std::string details_ = details.dump();
            send_command(arcirk::server::server_commands::SyncUpdateDataOnTheServer, {
                                                                                      {"device_id", wsSettings->deviceId().toStdString()},
                                                                                      {"table_name", arcirk::enum_synonym(arcirk::database::tables::tbDocuments)},
                                                                                      {"base64_param", QByteArray(details_.data()).toBase64().toStdString()},
                                                                                      });
            emit endAsyncSynchronize(operationName);
        }else{
            emit endAsyncSynchronize(operationName);
        }

    });

}

void IWebsocketClient::onUpdateBarcodeInformation(const arcirk::server::server_response &resp)
{

}

void IWebsocketClient::onServerResponse(const arcirk::server::server_response &resp)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    if(resp.command == arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery)){
        parse_exec_query_result(resp);
    }
}

bool IWebsocketClient::get_barcode_info_from_ws_server_synh(const QString &barcode, BarcodeInfo *bInfo)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    if(!sqlDatabase.isOpen())
        return false;

    //получаем информацию синхнонно с помощью http запроса
    //нам нужны две таблицы (штрихкоды и номенклатура) для обновления локальной базы

    //получим данные штрихкода
    auto sql_param = arcirk::synchronize::http_query_sql_param();
    sql_param.table_name = arcirk::enum_synonym(arcirk::database::tables::tbBarcodes);
    sql_param.query_type = "select";
    auto sql_param_json = pre::json::to_json(sql_param);

    sql_param_json["values"] = nlohmann::json{};
    sql_param_json["where_values"] = nlohmann::json{
        {"barcode", barcode.toStdString()}
    };

    auto param = http_param();
    param.command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);
    param.param = QByteArray(nlohmann::json{
                                      {"query_param", QByteArray(sql_param_json.dump().data()).toBase64().toStdString()}
                                  }.dump().data()).toBase64().toStdString();
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

    QString protocol = wsSettings->isSSL() ? "https://" : "http://";
    QString http_query = "/api/info";
    QUrl url(protocol + wsSettings->host() + ":" + QString::number(wsSettings->port()) + http_query);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString pwd = crypt(wsSettings->getHttpPassword(), "my_key").c_str();
    QString concatenated = wsSettings->getHttpUser() + ":" + pwd;
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    httpService.post(request, QByteArray::fromStdString(pre::json::to_json(param).dump()));
    loop.exec();

    if(httpStatus != 200){
        return false;
    }

    if(httpData.isEmpty())
        return false;

    if(httpData == "error"){
        bInfo->clear("Ошибка чтения штрихкода");
        emit displayError("get_barcode_inf_from_ws_server", "Не верный формат данных.");
        return false;
    }

    using namespace arcirk::database;
    auto msg = pre::json::from_json<arcirk::server::server_response>(nlohmann::json::parse(httpData.toStdString()));

    if(msg.result.empty())
        return false;

    auto table = nlohmann::json::parse(QByteArray::fromBase64(msg.result.data()).toStdString());
    auto rows = table["rows"];
    Q_ASSERT(rows.is_array());
    Q_ASSERT(rows.size() > 0);

    auto barcode_rec = pre::json::from_json<arcirk::database::barcodes>(rows[0]);

    std::string table_name = arcirk::enum_synonym(arcirk::database::tables::tbBarcodes);

    QSqlQuery rs(builder::query_builder().row_count().from(table_name).where(nlohmann::json{
                                                                                 {"ref", barcode_rec.ref}
                                                                             }, true).prepare().data());

    int count = 0;
    while (rs.next()) {
        count++;
    }

    auto query = builder::query_builder();
    query.use(pre::json::to_json(barcode_rec));
    std::string query_text;
    if(count > 0)
        query_text = query.update(table_name,true).where(nlohmann::json{
                                                              {"ref", barcode_rec.ref}
                                                          }, true).prepare();
    else
        query_text = query.insert(table_name, true).prepare();


    rs.clear();
    rs.exec(query_text.data());


    //получим данные номенклатуры
    sql_param = arcirk::synchronize::http_query_sql_param();
    sql_param.table_name = arcirk::enum_synonym(arcirk::database::tables::tbNomenclature);
    sql_param.query_type = "select";
    sql_param_json = pre::json::to_json(sql_param);

    sql_param_json["values"] = nlohmann::json{};
    sql_param_json["where_values"] = nlohmann::json{
        {"ref", barcode_rec.parent}
    };

    httpService.post(request, QByteArray::fromStdString(pre::json::to_json(param).dump()));
    loop.exec();

    if(httpStatus == 200 && !httpData.isEmpty()){
        auto nomenclature_rec = pre::json::from_json<arcirk::database::nomenclature>(httpData.toStdString());
        table_name = arcirk::enum_synonym(arcirk::database::tables::tbNomenclature);

        rs.clear();
        rs.exec(builder::query_builder().row_count().from(table_name).where(nlohmann::json{
                                                                                {"ref", nomenclature_rec.ref}
                                                                            }, true).prepare().data());

        count = 0;
        while (rs.next()) {
            count++;
        }
        query.use(pre::json::to_json(nomenclature_rec));
        std::string query_text;
        if(count > 0)
            query_text = query.update(table_name,true).where(nlohmann::json{
                                                                  {"ref", nomenclature_rec.ref}
                                                              }, true).prepare();
        else
            query_text = query.insert(table_name, true).prepare();


        rs.clear();
        rs.exec(query_text.data());

        auto bi = arcirk::client::barcode_info();
        bi.barcode = barcode.toStdString();
        //bi.image_base64 = nomenclature_rec.image;
        bi.synonym = nomenclature_rec.second;
        bi.vendor_code = nomenclature_rec.vendor_code;
        bi.unit = "шт";
        bi.uuid = nomenclature_rec.ref;
        bInfo->set_barcode_info(bi);
    }else{
        auto bi = arcirk::client::barcode_info();
        bi.barcode = barcode.toStdString();
        bInfo->set_barcode_info(bi);

    }

    return true;
}

void IWebsocketClient::get_barcode_info_from_ws_server_asynh(const QString &barcode)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    if(!sqlDatabase.isOpen())
        return;

    if(barcode.isEmpty())
        return;

    json fields = {
        {"barcode", "barcode"},
        {"parent", "parent"}
    };
    auto left_table = pre::json::to_json(database::nomenclature());
    json join_options = {
        {"parent", "ref"}
    };
    auto query = query_builder().select(fields)
                     .from(arcirk::enum_synonym(database::tbBarcodes))
                     .join(arcirk::enum_synonym(database::tbNomenclature), left_table, sql_join_type::joinLeft, join_options)
                     .where(json{
                                {"barcode", barcode.toStdString()}
                            }, true).prepare();


    auto param = json{
        {"table_name", "barcode_info"}
    };

    std::string query_param = QByteArray::fromStdString(param.dump()).toBase64().toStdString();
    send_command(arcirk::server::server_commands::ExecuteSqlQuery, {
                                                                       {"query_text", query},
                                                                       {"query_param", query_param}
                                                                   });
}

void IWebsocketClient::parse_exec_query_result(const arcirk::server::server_response &resp)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    json _p = nlohmann::json::parse(QByteArray::fromBase64(resp.param.data()).toStdString());
    if(_p.is_discarded()){
        return;
    }
    std::string p_str = _p.value("query_param", "");
    if(p_str.empty())
        return;
    std::string p = QByteArray::fromBase64(p_str.data()).toStdString();
    auto param = json::parse(p);
    std::string table_name = param.value("table_name", "");
    std::string query_type = param.value("query_type", "");
    std::string operation = param.value("operation", "");

    if(resp.result == WS_RESULT_ERROR)
        return;

    if(table_name == "barcode_info"){
        std::string table = QByteArray::fromBase64(resp.result.data()).toStdString();
        if(json::accept(table)){
            update_barcode_lite(json::parse(table));
        }
    }
}

void IWebsocketClient::update_barcode_lite(json table){

    qDebug() << __FUNCTION__;

    if(!table.is_object())
        return;

    auto rows = table.value("rows", json::array());
    if(rows.empty()){
        //на сервере зарошенный штрихкод не найден
    }else{
        for (auto itr = rows.begin(); itr != rows.end(); ++itr) {
            auto br = *itr;
            auto n = database::nomenclature();
            n.ref = br["NomenclatureSecond_ref"].get<std::string>();
            n.first = br["NomenclatureSecond_first"].get<std::string>();
            n.second = br["NomenclatureSecond_second"].get<std::string>();
            n.second = br["NomenclatureSecond_second"].get<std::string>();
            n.cache = br["NomenclatureSecond_cache"].get<std::string>();
            n.parent = br["NomenclatureSecond_parent"].get<std::string>();
            n.vendor_code = br["NomenclatureSecond_vendor_code"].get<std::string>();
            n.trademark = br["NomenclatureSecond_trademark"].get<std::string>();
            n.unit = br["NomenclatureSecond_unit"].get<std::string>();
            n.is_marked = br["NomenclatureSecond_is_marked"].get<int>();
            n.version = 1;

            using namespace arcirk::database;
            auto query = builder::query_builder();
            int count = 0;
            QSqlQuery rs(query.row_count().from(arcirk::enum_synonym(tables::tbNomenclature)).where(nlohmann::json{
                                                                                                        {"ref", n.ref}
                                                                                                    }, true).prepare().c_str());
            while (rs.next()) {
                count += rs.value(0).toInt();
            }
            query.clear();
            query.use(pre::json::to_json(n));
            if(count == 0){
                query.insert(arcirk::enum_synonym(tables::tbNomenclature), true);
            }else{
                query.update(arcirk::enum_synonym(tables::tbNomenclature), true).where(nlohmann::json{
                                                                                           {"ref", n.ref}
                                                                                       }, true);
            }
            rs.clear();
            rs.exec(query.prepare().c_str());

            if(rs.lastError().isValid()){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << rs.lastError().text();
                return;
            }

            auto b = database::barcodes();
            b.ref = generate_uuid().toStdString();
            b.barcode = br["BarcodesFirst_barcode"].get<std::string>();
            b.first_name = n.first;
            b.parent = n.ref;
            b.vendor_code = n.vendor_code;
            b.version = 1;

            rs.clear();
            query.clear();

            count = 0;
            rs.exec(query.row_count().from(arcirk::enum_synonym(tables::tbBarcodes)).where(nlohmann::json{
                                                                                               {"barcode", b.barcode}
                                                                                           }, true).prepare().c_str());
            while (rs.next()) {
                count += rs.value(0).toInt();
            }
            query.clear();
            query.use(pre::json::to_json(b));
            if(count == 0){
                query.insert(arcirk::enum_synonym(tables::tbBarcodes), true);
            }else{
                query.update(arcirk::enum_synonym(tables::tbBarcodes), true).where(nlohmann::json{
                                                                                       {"barcode", b.barcode}
                                                                                   }, true);
            }
            rs.clear();
            rs.exec(query.prepare().c_str());

            if(rs.lastError().isValid()){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << rs.lastError().text();
                return;
            }

            emit updateBarcode(b.barcode.c_str());
        }

    }

}

QString IWebsocketClient::documentGenerateNewNumber(const int id)
{
    return QString("%1").arg(id, 9, 'g', -1, '0');
}

void IWebsocketClient::documentUpdate(const QString &number, const QString &date, const QString& comment, const QString& source)
{

    auto source_ = arcirk::database::documents();
    if(json::accept(source.toStdString()))
        source_ = secure_serialization<arcirk::database::documents>(json::parse(source.toStdString()));

    json cache{};

    if(json::accept(source_.cache))
        cache = json::parse(source_.cache);

    if(source_.ref == arcirk::uuids::nil_string_uuid() || source_.ref.empty()){
        source_.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        source_.first = "ПодборШтрихкодов";
        source_.second = "Подбор штрихкодов";
        source_.xml_type = "DocumentRef.ПодборШтрихкодов";
        source_.device_id = wsSettings->deviceId().toStdString();
    }

    if(cache.is_discarded()){
        cache = {
            {"comment", comment.toStdString()}
        };
    }else{
        cache["comment"] = comment.toStdString();
    }

    source_.number = number.toStdString();

    auto dt = QDateTime::fromString(date, "dd.MM.yyyy hh:mm:ss");
    source_.date = dt.toSecsSinceEpoch();// + dt.offsetFromUtc();
    source_.cache = cache.dump();
    source_.version++;

    if(!sqlDatabase.isOpen()){
        qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__  << "База данных не подключена!";
        return;
    }

    using namespace arcirk::database;
    auto query = builder::query_builder();
    int count = 0;
    QSqlQuery rs(query.row_count().from(arcirk::enum_synonym(tables::tbDocuments)).where(nlohmann::json{
                                                                                             {"ref", source_.ref}
                                                                                         }, true).prepare().c_str());
    while (rs.next()) {
        count += rs.value(0).toInt();
    }
    query.clear();
    query.use(pre::json::to_json(source_));
    if(count == 0){
        query.insert(arcirk::enum_synonym(tables::tbDocuments), true);
    }else{
        query.update(arcirk::enum_synonym(tables::tbDocuments), true).where(nlohmann::json{
                                                                                {"ref", source_.ref}
                                                                            }, true);
    }
    rs.clear();
    rs.exec(query.prepare().c_str());

    if(rs.lastError().isValid())
        qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << rs.lastError().text();

    getDocuments();

}

void IWebsocketClient::getDocumentMarkedContent(const QString &ref)
{
    using namespace arcirk::database;

    if(!sqlDatabase.isOpen())
        return;
    using namespace arcirk::database;
    json result{};
    auto query = builder::query_builder();

    std::string query_str = query.select()
                                .from(arcirk::enum_synonym(tables::tbDocumentsMarkedTables))
                                .where(json{
                                           {"parent", ref.toStdString()}
                                       }, true).prepare();
    arcirk::database::execute(query_str
                              , sqlDatabase, result);

    if(result["columns"].size() == 1){
        auto table_info = arcirk::database::table_info(sqlDatabase, tables::tbDocumentsMarkedTables);
        for(auto itr = table_info.begin(); itr != table_info.end();  ++itr){
            std::string col = itr.key();
            result["columns"] += col;
        }

        result["rows"] = nlohmann::json{};
    }
    std::string d = result.dump();
    emit readDocumentMarkedTable(QString::fromStdString(d));

}

