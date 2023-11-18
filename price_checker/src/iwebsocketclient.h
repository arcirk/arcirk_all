#ifndef IWEBSOCKETCLIENT_H
#define IWEBSOCKETCLIENT_H

#include <websocketclient.h>
#include <QObject>
#include "src/wssettings.h"
#include "barcode_info.hpp"
#include "iface/iface.hpp"

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
    Q_INVOKABLE void get_image_data(BarcodeInfo *bInfo);
    Q_INVOKABLE void init_data_options();
    Q_INVOKABLE QString documentDate(const int value) const;
    Q_INVOKABLE void setQrCode(const QString &qr, const QString &doc_ref, const QString &row_ref, IQMLTreeItemModel* model);
    Q_INVOKABLE QString parseQr(const QString &qr) ;


    Q_INVOKABLE void getDocuments();

    Q_INVOKABLE void synchronizeBase();

private:
    Settings * wsSettings;
    QTimer * m_reconnect;
    QTimer * m_tmr_synchronize;
    QQueue<async_await> m_async_await;
    QSqlDatabase sqlDatabase;
    QVector<QString> m_vecOperations;

    void asyncAwait(){
        if(m_async_await.size() > 0){
            auto f = m_async_await.dequeue();
            f();
        }
    };
    void reconnect();

    void set_new_version_document(const QString& ref);

private slots:
    void onReconnect();
    void onSynchronize();
    void onSynchronizeBaseNext(const arcirk::server::server_response &resp);

signals:
    void readDocuments(const QString& jsonModel);

};

#endif // IWEBSOCKETCLIENT_H
