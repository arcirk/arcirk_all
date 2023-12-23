#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include "arcirk.hpp"
#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QUuid>
#include <QQueue>
#include <functional>
#include <QTimer>

using namespace arcirk;
using json = nlohmann::json;

#define ARCIRK_VERSION "1.1.1"
#define CONF_FILENAME "app_conf.json"
#define CLIENT_VERSION 3

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), client_conf,
    (std::string, app_name)
    (bool, is_auto_connect)
    (std::string, server_host)
    (std::string, user_name)
    (std::string, hash)
    (std::string, device_id)
    (ByteArray, servers)
    (std::string, price_checker_repo)
    (std::string, server_repo)
    (std::string, system_user)
    (std::string, firefox)
    (bool, use_sid)
    );

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::server), server_config,
    (std::string, ServerHost)
    (int, ServerPort)
    (std::string, ServerUser)
    (std::string, ServerUserHash)
    (std::string, ServerName)
    (std::string, ServerHttpRoot)
    (std::string, ServerWorkingDirectory)
    (bool, AutoConnect)
    (bool, UseLocalWebDavDirectory)
    (std::string, LocalWebDavDirectory)
    (std::string, WebDavHost)
    (std::string, WebDavUser)
    (std::string, WebDavPwd)
    (std::string, WebDavRoot)
    (bool, WebDavSSL)
    (int, SQLFormat)
    (std::string, SQLHost)
    (std::string, SQLUser)
    (std::string, SQLPassword)
    (std::string, HSHost)
    (std::string, HSUser)
    (std::string, HSPassword)
    (bool, ServerSSL)
    (std::string, SSL_crt_file)
    (std::string, SSL_key_file)
    (bool, UseAuthorization)
    (std::string, ApplicationProfile)
    (int, ThreadsCount)
    (std::string, Version)
    (bool, ResponseTransferToBase64)
    (bool, AllowDelayedAuthorization)
    (bool, AllowHistoryMessages)
    (std::string, ExchangePlan)
    (std::string, ServerProtocol)
    (bool, WriteJournal)
    (bool, AllowIdentificationByWINSID)
    );

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), session_info,
    (std::string, session_uuid)
    (std::string, user_name)
    (std::string, user_uuid)
    (std::string, start_date)
    (std::string, app_name)
    (std::string, role)
    (std::string, device_id)
    (std::string, address)
    (std::string, info_base)
    (std::string, host_name)
    (std::string, product)
    (std::string, system_user)
    (std::string, sid)
    )

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), client_param,
    (std::string, app_name)
    (std::string, user_uuid)
    (std::string, user_name)
    (std::string, hash)
    (std::string, host_name)
    (std::string, password)
    (std::string, session_uuid)
    (std::string, system_user)
    (std::string, device_id)
    (std::string, info_base)
    (std::string, product)
    (std::string, sid)
    (int, version)
    )


BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::server), server_response,
    (std::string, command)
    (std::string, message)
    (std::string, param)
    (std::string, result)
    (std::string, sender)
    (std::string, receiver)
    (std::string, uuid_form)
    (std::string, app_name)
    (std::string, sender_name)
    (std::string, sender_uuid)
    (std::string, receiver_name)
    (std::string, receiver_uuid)
    (std::string, version)
    (ByteArray, data)
    )

namespace arcirk::server{

    enum server_commands{
        ServerVersion, //Версия сервера
        ServerOnlineClientsList, //Список активных пользователей
        SetClientParam, //Параметры клиента
        ServerConfiguration, //Конфигурация сервера
        UserInfo, //Информация о пользователе (база данных)
        InsertOrUpdateUser, //Обновить или добавить пользователя (база данных)
        CommandToClient, //Команда клиенту (подписчику)
        ServerUsersList, //Список пользователей (база данных)
        ExecuteSqlQuery, //выполнить запрос к базе данных
        GetMessages, //Список сообщений
        UpdateServerConfiguration, //Обновить конфигурацию сервера
        HttpServiceConfiguration, //Получить конфигурацию http сервиса 1С
        InsertToDatabaseFromArray, //Добавить массив записей в базу //устарела удалить
        SetNewDeviceId, //Явная установка идентификатора на устройствах где не возможно его получить
        ObjectSetToDatabase, //Синхронизация объекта 1С с базой
        ObjectGetFromDatabase, //Получить объект типа 1С из базы данных для десериализации
        SyncGetDiscrepancyInData, //Получить расхождения в данных между базами на клиенте и на Сервере
        SyncUpdateDataOnTheServer, //Обновляет данные на сервере по запросу клиента
        WebDavServiceConfiguration, //Получить настройки WebDav
        SyncUpdateBarcode, //синхронизирует на сервере штрихкод и номенклатуру по запросу клиента с сервером 1с
        DownloadFile, //Загружает файл на сервер
        GetInformationAboutFile, //получить информацию о файле
        CheckForUpdates, //поиск фалов обрновления
        UploadFile, //скачать файл
        GetDatabaseTables,
        FileToDatabase, //Загрузка таблицы базы данных из файла
        ProfileDirFileList,
        ProfileDeleteFile,
        DeviceGetFullInfo,
        GetTasks,
        UpdateTaskOptions,
        TasksRestart,
        RunTask,
        StopTask,
        SendNotify,
        GetCertUser,
        VerifyAdministrator,
        UserMessage,
        GetChannelToken,
        IsChannel,
        GetDatabaseStructure,
        Run1CScript,
        CreateDirectories,
        DeleteDirectory,
        bDeleteFile,
        CMD_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(server_commands, {
        {CMD_INVALID, nullptr}    ,
        {ServerVersion, "ServerVersion"}  ,
        {ServerOnlineClientsList, "ServerOnlineClientsList"}    ,
        {SetClientParam, "SetClientParam"}    ,
        {ServerConfiguration, "ServerConfiguration"}    ,
        {UserInfo, "UserInfo"}    ,
        {InsertOrUpdateUser, "InsertOrUpdateUser"}    ,
        {CommandToClient, "CommandToClient"}    ,
        {ServerUsersList, "ServerUsersList"}    ,
        {ExecuteSqlQuery, "ExecuteSqlQuery"}    ,
        {GetMessages, "GetMessages"}    ,
        {UpdateServerConfiguration, "UpdateServerConfiguration"}    ,
        {HttpServiceConfiguration, "HttpServiceConfiguration"}    ,
        {InsertToDatabaseFromArray, "InsertToDatabaseFromArray"}    ,
        {SetNewDeviceId, "SetNewDeviceId"}    ,
        {ObjectSetToDatabase, "ObjectSetToDatabase"}    ,
        {ObjectGetFromDatabase, "ObjectGetFromDatabase"}    ,
        {SyncGetDiscrepancyInData, "SyncGetDiscrepancyInData"}    ,
        {SyncUpdateDataOnTheServer, "SyncUpdateDataOnTheServer"}    ,
        {WebDavServiceConfiguration, "WebDavServiceConfiguration"}    ,
        {SyncUpdateBarcode, "SyncUpdateBarcode"}    ,
        {DownloadFile, "DownloadFile"}    ,
        {GetInformationAboutFile, "GetInformationAboutFile"}    ,
        {CheckForUpdates, "CheckForUpdates"}    ,
        {UploadFile, "UploadFile"}    ,
        {GetDatabaseTables, "GetDatabaseTables"}    ,
        {FileToDatabase, "FileToDatabase"}    ,
        {ProfileDirFileList, "ProfileDirFileList"}    ,
        {ProfileDeleteFile, "ProfileDeleteFile"}    ,
        {DeviceGetFullInfo, "DeviceGetFullInfo"}    ,
        {GetTasks, "GetTasks"}    ,
        {UpdateTaskOptions, "UpdateTaskOptions"}    ,
        {TasksRestart, "TasksRestart"}    ,
        {RunTask, "RunTask"}    ,
        {StopTask, "StopTask"}    ,
        {SendNotify, "SendNotify"}    ,
        {GetCertUser, "GetCertUser"}    ,
        {VerifyAdministrator, "VerifyAdministrator"}    ,
        {UserMessage, "UserMessage"}    ,
        {GetChannelToken, "GetChannelToken"}    ,
        {IsChannel, "IsChannel"}    ,
        {GetDatabaseStructure, "GetDatabaseStructure"}    ,
        {Run1CScript, "Run1CScript"}    ,
        {CreateDirectories, "CreateDirectories"}    ,
        {DeleteDirectory, "DeleteDirectory"}    ,
        {bDeleteFile, "DeleteFile"}    ,
    });


    enum server_objects{
        OnlineUsers,
        Root,
        Services,
        Database,
        DatabaseTables,
        Devices,
        DatabaseUsers,
        ProfileDirectory,
        CertManager,
        Containers,
        Certificates,
        CertUsers,
        LocalhostUser,
        LocalhostUserCertificates,
        LocalhostUserContainers,
        LocalhostUserContainersRegistry,
        LocalhostUserContainersVolume,
        AvailableCertificates,
        QueryBuilderStructure,
        QueryBuilderTables,
        QueryBuilderJnners,
        OBJ_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(server_objects, {
        {OBJ_INVALID, nullptr} ,
        {OnlineUsers, "OnlineUsers"} ,
        {Root, "Root"} ,
        {Services, "Services"} ,
        {Database, "Database"} ,
        {DatabaseTables, "DatabaseTables"},
        {Devices, "Devices"},
        {DatabaseUsers, "DatabaseUsers"},
        {ProfileDirectory, "ProfileDirectory"},
        {CertManager, "CertManager"},
        {Containers, "Containers"},
        {Certificates, "Certificates"},
        {CertUsers, "CertUsers"},
        {LocalhostUser, "LocalhostUser"},
        {LocalhostUserCertificates, "LocalhostUserCertificates"},
        {LocalhostUserContainers, "LocalhostUserContainers"},
        {LocalhostUserContainersRegistry, "LocalhostUserContainersRegistry"},
        {LocalhostUserContainersVolume, "LocalhostUserContainersVolume"},
        {AvailableCertificates, "AvailableCertificates"},
    });
}
class WebSocketClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)

public:
    explicit WebSocketClient(QObject *parent = nullptr);
    explicit WebSocketClient(const QUrl& url, QObject *parent = nullptr);
    ~WebSocketClient();

    Q_INVOKABLE static QString generateHash(const QString& usr, const QString& pwd);
    static QString get_sha1(const QByteArray& p_arg);

    Q_INVOKABLE bool isStarted();

    client::client_conf& conf();
    server::server_config& server_conf();
    client::client_param &client_server_param();
    void set_client_conf(const json& value);
    void set_server_conf(const json& value);
    void set_client_param(const json& value);

    bool isConnected();

    QUrl url() const;
    Q_INVOKABLE void setUrl(const QUrl& url);
    QUrl http_url() const;

    void open(const std::string& sid = "");

    Q_INVOKABLE void close();
    void write_conf();
    void set_conf(const client::client_conf& value);

    void send_command(arcirk::server::server_commands cmd, const json& param = {});
    void command_to_client(const std::string &receiver, const std::string &command,
                                       const json &param = {});
    void send_message(const std::string &receiver, const std::string &message,
                      const json &param = {});

    json exec_http_query(const std::string& command, const json& param, const ByteArray& data = {}, bool returnAllMessage = false);
    static json http_query(const QUrl& ws, const QString& token, const std::string& command, const json& param, const ByteArray& data = {});

    QByteArray exec_http_query_get(const std::string& command, const json& param);

    static std::string crypt(const QString &source, const QString &key);

    static arcirk::client::version_application get_version(){
         QStringList vec = QString(ARCIRK_VERSION).split(".");
         auto ver = arcirk::client::version_application();
         ver.major = vec[0].toInt();
         ver.minor = vec[1].toInt();
         ver.path = vec[2].toInt();
        return ver;
    }

    void register_device(const arcirk::client::session_info& sess_info);

    void set_system_user(const QString& value);

    QUuid currentSession() const;
    QUuid currentUserUuid() const;

protected:
    QWebSocket* m_client;
    client::client_conf conf_;
    client::client_param client_param_;

private:

    server::server_config server_conf_;

    QString system_user_;
    std::string m_sid;

    QUuid m_currentSession;
    QUuid m_currentUserUuid;

    bool m_isConnected;

    void read_conf(const QString& app_name = "ServerManager");

    void initialize();

    void parse_response(const QString& resp);

    void doDisplayError(const QString& what, const QString& err);
    void doConnectionSuccess(); //при успешной авторизации
    void doConnectionChanged(bool state);

    static QString get_hash(const QString& first, const QString& second);
    void get_server_configuration_sync();
    static arcirk::client::client_param parse_client_param(const std::string& response);

signals:
    void displayError(const QString& what, const QString& err);
    void error(const QString& what, const QString& command, const QString& err);
    void connectionSuccess(); //при успешной авторизации
    void connectionChanged(bool state);
    void serverResponse(const arcirk::server::server_response& message);
    void notify(const QString& message);
    void commandToClientResponse(const arcirk::server::server_response& message);
    void userMessage(const arcirk::server::server_response& message);
    void userInfo(const json& info);
    void urlChanged();

    void syncGetDiscrepancyInData(const arcirk::server::server_response& resp);
    void updateBarcodeInformation(const arcirk::server::server_response& resp);

private slots:

    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error, const QString& errorString);
    void onReconnect();

};

#define PATH_TO_RELEASE "arcirk_server/update/"

typedef std::function<void()> async_await_;

class IWClient : public WebSocketClient{

    Q_OBJECT

public:
    explicit IWClient(QObject *parent = nullptr): WebSocketClient{parent}{
        m_reconnect = new QTimer(this);
        connect(m_reconnect,SIGNAL(timeout()),this,SLOT(onReconnect()));
    };

    ~IWClient(){m_reconnect->stop();};

    Q_INVOKABLE void checkConnection(){
        if(!m_reconnect->isActive())
            startReconnect();
    }
    Q_INVOKABLE void openConnection(){
        this->open();
    }
    Q_INVOKABLE void startReconnect(){
        m_reconnect->start(1000 * 60);
    };

    QString token() const{
        return conf_.hash.c_str();
    }

private:
    QTimer * m_reconnect;
    QTimer * m_tmr_synchronize;
    QQueue<async_await_> m_async_await;

    void asyncAwait(){
        if(m_async_await.size() > 0){
            auto f = m_async_await.dequeue();
            f();
        }
    };

    void reconnect(){
        open();
    }

private slots:
    void onReconnect(){
        //qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;
        if(isStarted()){
            if(m_reconnect->isActive())
                m_reconnect->stop();
        }else{
            m_async_await.append(std::bind(&IWClient::reconnect, this));
            asyncAwait();
        }
    }

};

#endif // WEBSOCKETCLIENT_H
