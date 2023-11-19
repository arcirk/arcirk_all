#ifndef IWEBSOCKETCLIENT_H
#define IWEBSOCKETCLIENT_H

#include <websocketclient.h>
#include <QObject>
#include "src/wssettings.h"
#include "barcode_info.hpp"
#include "iface/iface.hpp"
#include "synch_data.hpp"

using namespace arcirk::tree_model;

typedef std::function<void()> async_await;

class IWebsocketClient : public WebSocketClient
{
    Q_OBJECT
public:
    explicit IWebsocketClient(QObject *parent = nullptr);
    ~IWebsocketClient(){m_reconnect->stop();};

    Q_INVOKABLE void checkConnection();
    Q_INVOKABLE void open(arcirk::Settings * sett);

    Q_INVOKABLE void startReconnect(){
        if(!wsSettings)
            return;
        if (wsSettings->autoConnect())
            m_reconnect->start(1000 * 60);
    };
    Q_INVOKABLE void startSynchronize(){
        qDebug() << __FUNCTION__;
        //ToDo: Добавить в настройки периодичность запуска.
        m_tmr_synchronize->start(10 * 1000 * 60);
    };

    Q_INVOKABLE bool barcode_info_from_1c_service(const QString &barcode, BarcodeInfo *bInfo);
    Q_INVOKABLE void get_barcode_information(const QString& barcode, BarcodeInfo* bInfo);
    Q_INVOKABLE void get_image_data(BarcodeInfo *bInfo);


    Q_INVOKABLE void init_data_options();

    Q_INVOKABLE void setQrCode(const QString &qr, const QString &doc_ref, const QString &row_ref, IQMLTreeItemModel* model);
    Q_INVOKABLE QString parseQr(const QString &qr) ;

    Q_INVOKABLE void getDocuments();
    Q_INVOKABLE void getDocumentContent(const QString& ref);
    Q_INVOKABLE void documentUpdate(const QString& number, const QString& date, const QString& comment, const QString& source);
    Q_INVOKABLE QString documentGenerateNewNumber(const int id);
    Q_INVOKABLE QString documentDate(const int value) const;
    Q_INVOKABLE void deleteDocument(const QString& ref, const int ver);
    Q_INVOKABLE void documentContentUpdate(BarcodeInfo *bInfo, IQMLTreeItemModel* model, const QString& documentRef);
    Q_INVOKABLE void documentModelUpdateItem(BarcodeInfo *bInfo, IQMLTreeItemModel* model);
    Q_INVOKABLE void documentTableSetQuantity(const QString& ref, int quantity, IQMLTreeItemModel* model);
    Q_INVOKABLE void synchronizeBase();
    Q_INVOKABLE void getDocumentMarkedContent(const QString& ref);

    Q_INVOKABLE int dataMax();

    //проверяет номенклатуру в документе
    Q_INVOKABLE void verifyDocument(const QString& documentRef);

    Q_INVOKABLE void removeRow(const QString& ref, IQMLTreeItemModel* model);

private:
    Settings * wsSettings;
    QTimer * m_reconnect;
    QTimer * m_tmr_synchronize;
    QQueue<async_await> m_async_await;
    QSqlDatabase sqlDatabase;
    QVector<QString> m_vecOperations;
    bool is_pool_run;

    void asyncAwait(){
        if(m_async_await.size() > 0){
            auto f = m_async_await.dequeue();
            f();
        }
    };
    void reconnect();

    void set_new_version_document(const QString& ref);
    bool get_barcode_info_local(const QString &barcode, BarcodeInfo *bInfo);
    bool get_barcode_info_from_ws_server_synh(const QString &barcode, BarcodeInfo *bInfo);
    void get_barcode_info_from_ws_server_asynh(const QString &barcode);
    void parse_exec_query_result(const arcirk::server::server_response &resp);
    void update_barcode_lite(json table);

private slots:
    void onReconnect();
    void onSynchronize();
    void onSynchronizeBaseNext(const arcirk::server::server_response &resp);
    void synh_lambda(arcirk::synchronize::SynchronizeBase * syncClass);
    void onUpdateBarcodeInformation(const arcirk::server::server_response &resp);
    void onServerResponse(const arcirk::server::server_response &resp);

signals:
    void readDocuments(const QString& jsonModel);
    void readDocument(const QString& jsonModel);
    void readDocumentTable(const QString& jsonModel);
    void updateBarcode(const QString& barcode);
    void readDocumentMarkedTable(const QString& jsonModel);

    void startAsyncSynchronize(const QString& operationName);
    void endAsyncSynchronize(const QString& operationName);
};

#endif // IWEBSOCKETCLIENT_H
