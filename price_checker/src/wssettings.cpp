#include "wssettings.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QSysInfo>


namespace arcirk{

    Settings::Settings(QObject *parent)
        : QObject{parent}
    {

        read_private_conf();
//        if(client_conf.deviceId.empty())
//            init_device_id();
//        else{
//            m_device_id = QUuid::fromString(QString::fromStdString(client_conf.deviceId));
//        }

        read_conf();

        m_product = QSysInfo::prettyProductName();

    }

    void Settings::read_conf(){

        qDebug() << __FUNCTION__;

        conf.ServerHost = "127.0.0.1";
        conf.ServerPort = 8080;
        conf.ServerUser = "IIS_1C";
        conf.HSHost = "http://localhost/trade/hs/http_trade";
        conf.Version = ARCIRK_VERSION;

        auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(path);
        if(!dir.exists())
            dir.mkpath(path);

        auto fileName= path + "/price_checker_conf.json";
        if(QFile::exists(fileName)){

            QFile f(fileName);
            f.open(QIODevice::ReadOnly);

            std::string m_text = f.readAll().toStdString();
            f.close();

            try {
                conf = pre::json::from_json<server::server_config>(m_text);
            } catch (std::exception& e) {
                qCritical() << __FUNCTION__ << e.what();
            }
        }
    }

    void Settings::read_private_conf(){

        qDebug() << __FUNCTION__;

        client_conf.showImage = false;
        client_conf.keyboardInputMode = true;
        client_conf.priceCheckerMode = true;

        auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(path);
        if(!dir.exists())
            dir.mkpath(path);

        auto fileName= path + "/price_checker_private_conf.json";
        if(QFile::exists(fileName)){
            QFile f(fileName);
            f.open(QIODevice::ReadOnly);

            std::string m_text = f.readAll().toStdString();
            f.close();

            try {
                client_conf = pre::json::from_json<client::client_private_config>(m_text);
            } catch (std::exception& e) {
                qCritical() << __FUNCTION__ << e.what();
            }
        }
    }

    void Settings::write_conf(){
        try {
            std::string result = pre::json::to_json(conf).dump() ;
            auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            auto fileName= path + "/price_checker_conf.json";
            QFile f(fileName);
            qDebug() << fileName;
            f.open(QIODevice::WriteOnly);
            f.write(QByteArray::fromStdString(result));
            f.close();
        } catch (std::exception& e) {
            qCritical() << __FUNCTION__ << e.what();
        }
    }

    void Settings::write_private_conf(){
        try {
            client_conf.deviceId = deviceId().toStdString();
            std::string result = pre::json::to_json(client_conf).dump() ;
            auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            auto fileName= path + "/price_checker_private_conf.json";
            qDebug() << fileName;
            QFile f(fileName);
            f.open(QIODevice::WriteOnly);
            f.write(QByteArray::fromStdString(result));
            f.close();
        } catch (std::exception& e) {
            qCritical() << __FUNCTION__ << e.what();
        }
    }

    void Settings::init_device_id(){
        QString m_id = QSysInfo::machineUniqueId();
        if(m_id.isEmpty())
            m_device_id = QUuid::createUuid();
        else{
           m_device_id = QUuid::fromString(m_id);
        }
        client_conf.deviceId = m_device_id.toString().toStdString();
    }

    QString Settings::host() const
    {
        return QString::fromStdString(conf.ServerHost);//m_host;
    }

    int Settings::port()
    {
        return conf.ServerPort;//m_port;
    }

    QString Settings::userName() const
    {
        return QString::fromStdString(conf.ServerUser);//m_userName;
    }

    QString Settings::hash() const
    {
        return QString::fromStdString(conf.ServerUserHash);//m_hash;
    }

    void Settings::setHost(const QString &value)
    {
        conf.ServerHost = value.toStdString();
    }

    void Settings::setPort(int value)
    {
        conf.ServerPort = value;
    }

    void Settings::setUserName(const QString &value)
    {
        conf.ServerUser = value.toStdString();
    }

    void Settings::setHash(const QString &value)
    {
        conf.ServerUserHash = value.toStdString();
    }

    void Settings::setProduct(const QString &value)
    {
        m_product = value;
    }

    QUrl Settings::url() const
    {
        QUrl _url;
        _url.setHost(QString::fromStdString(conf.ServerHost));
        _url.setPort(conf.ServerPort);
        _url.setScheme(QString::fromStdString(conf.ServerProtocol));
        return _url;
    }

    void Settings::setUrl(const QString &url)
    {
        QUrl _url(url);
        conf.ServerHost = _url.host().toStdString();
        conf.ServerPort = _url.port();
        conf.ServerProtocol = _url.scheme().toStdString();
    }

    void Settings::save()
    {
        qDebug() << __FUNCTION__;

        write_conf();
        write_private_conf();
    }

    void Settings::setDefault(const QString &url_)
    {
        setUrl(url_);
        QString user = "IIS_1C";
        QString pass = "LbyFvj1";
        QString hash = WebSocketClient::generateHash(user, pass);
        setHash(hash);
        auto host_ = QString("http://%1/trade/hs/http_trade").arg(url().host());
        if(url_.indexOf("192.168.10.80") != -1)
           host_ = QString("http://%1/trade_exp/hs/http_trade").arg("192.168.10.9");
        if(url_.indexOf("192.168.43.18") != -1)
           host_ = QString("http://%1/trade_exp/hs/http_trade").arg("192.168.43.25");

        setHttpService(host_);
        setHttpPwd(crypt(pass.toLocal8Bit().toStdString(), std::string("my_key")).c_str());
        setHttpUser("IIS_1C");
        emit optionsChanged();
    }


    QString Settings::product() const
    {
        return m_product;
    }

    bool Settings::autoConnect() const
    {
        return conf.AutoConnect;
    }

    void Settings::update_workplace_data(const nlohmann::json &object)
    {
        try {
            m_workplace = pre::json::from_json<arcirk::database::devices>(object);
        } catch (const std::exception& e) {
            qCritical() << __FUNCTION__ << e.what();
        }

    }

    void Settings::update_workplace_view(const nlohmann::json &object)
    {
        try {
            m_workplace_view = pre::json::from_json<arcirk::database::devices_view>(object);
            emit updateWorkplaceView(QString::fromStdString(m_workplace_view.organization),
                                     QString::fromStdString(m_workplace_view.subdivision),
                                     QString::fromStdString(m_workplace_view.warehouse),
                                     QString::fromStdString(m_workplace_view.price));

        } catch (const std::exception& e) {
            qCritical() << __FUNCTION__ << e.what();
        }
    }

    arcirk::database::devices &Settings::workplace_options()
    {
        return m_workplace;
    }

    QString Settings::getDavService() const
    {
        return davService();
    }

    QString Settings::getHttpService() const
    {
        return httpService();
    }

    QString Settings::getHttpPassword() const
    {
        return httpPwd();
    }

    QString Settings::getHttpUser()
    {
        QString usr = httpUser();
        if(usr.isEmpty()){
            usr = userName();
            setHttpUser(usr);
            save();
        }
        return usr;
    }

    bool Settings::isShowImage()
    {
        return showImage();
    }

    QString Settings::davService() const
    {
        return QString::fromStdString(conf.WebDavHost);
    }

    QString Settings::deviceId() const
    {
        return m_device_id.toString(QUuid::WithoutBraces);
    }

    void Settings::setDeviceId(const QString &device_id)
    {
        m_device_id = QUuid(device_id);
    }

    void Settings::setDavService(const QString &value)
    {
        conf.WebDavHost = value.toStdString();
    }

    void Settings::setHttpService(const QString &value)
    {
        //m_httpService = value;
        conf.HSHost = value.toStdString();
    }

    void Settings::setHttpPwd(const QString &value)
    {
        //m_httpPwd = value;
        conf.HSPassword = value.toStdString();
    }

    void Settings::setHttpUser(const QString &value)
    {
        conf.HSUser = value.toStdString();
    }

    bool Settings::isSSL() const
    {
        return conf.ServerSSL;
    }

    void Settings::setKeyboardInputMode(bool value)
    {
        client_conf.keyboardInputMode = value;
    }

    void Settings::setPriceCheckerMode(bool value)
    {
       client_conf.priceCheckerMode = value;
    }

    bool Settings::keyboardInputMode()
    {
        return client_conf.keyboardInputMode;
    }

    bool Settings::priceCheckerMode()
    {
        return client_conf.priceCheckerMode;
    }

    void Settings::setShowImage(bool value)
    {
        client_conf.showImage = value;
    }

    void Settings::setAutoConnect(bool value)
    {
        conf.AutoConnect = value;
    }

    bool Settings::showImage()
    {
        return client_conf.showImage;// m_showImage;
    }

    QString Settings::httpService() const
    {
        return QString::fromStdString(conf.HSHost); //m_httpService;
    }

    QString Settings::httpPwd() const
    {
        return QString::fromStdString(conf.HSPassword); //m_httpPwd;
    }

    QString Settings::httpUser() const
    {
        return QString::fromStdString(conf.HSUser);
    }


}
