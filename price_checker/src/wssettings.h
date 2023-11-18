#ifndef WS_SETTINGS_HPP
#define WS_SETTINGS_HPP

#include <QObject>
#include <QUrl>
#include <QUuid>
#include "shared_struct.hpp"
#include "database_struct.hpp"
#include "websocketclient.h"

namespace arcirk{

    class Settings : public QObject{
        Q_OBJECT
        Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
        Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
        Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
        Q_PROPERTY(QString hash READ hash WRITE setHash NOTIFY hashChanged)
        Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)
        Q_PROPERTY(QString product READ product WRITE setProduct NOTIFY productChanged)
        Q_PROPERTY(QString httpService READ httpService WRITE setHttpService NOTIFY httpServiceChanged)
        Q_PROPERTY(QString davService READ davService WRITE setDavService NOTIFY davServiceChanged)
        Q_PROPERTY(QString httpPwd READ httpPwd WRITE setHttpPwd NOTIFY httpPwdChanged)
        Q_PROPERTY(QString httpUser READ httpPwd WRITE setHttpUser NOTIFY httpUserChanged)
        Q_PROPERTY(bool showImage READ showImage WRITE setShowImage NOTIFY showImageChanged)
        Q_PROPERTY(bool keyboardInputMode READ keyboardInputMode WRITE setKeyboardInputMode NOTIFY keyboardInputModeChanged)
        Q_PROPERTY(bool priceCheckerMode READ priceCheckerMode WRITE setPriceCheckerMode NOTIFY priceCheckerModeChanged)
        Q_PROPERTY(bool autoConnect READ autoConnect WRITE setAutoConnect NOTIFY autoConnectChanged)
    public:
        explicit
        Settings(QObject *parent = nullptr);

        Q_INVOKABLE void setHost(const QString& value);
        Q_INVOKABLE void setPort(int value);
        Q_INVOKABLE void setUserName(const QString& value);
        Q_INVOKABLE void setHash(const QString& value);
        Q_INVOKABLE void setProduct(const QString& value);

        Q_INVOKABLE QUrl url() const;
        Q_INVOKABLE void setUrl(const QString& url);
        Q_INVOKABLE void save();
        Q_INVOKABLE void setDefault(const QString& url);

        Q_INVOKABLE void setDeviceId(const QString& device_id);

        Q_INVOKABLE void setDavService(const QString& value);
        Q_INVOKABLE void setHttpService(const QString& value);
        Q_INVOKABLE void setHttpPwd(const QString& value);
        Q_INVOKABLE void setHttpUser(const QString& value);

        Q_INVOKABLE bool isSSL() const;

        void setKeyboardInputMode(bool value); //Используется клавиатурный ввод (прайс-чекер, или подключенный сканер)
        void setPriceCheckerMode(bool value);

        void setShowImage(bool value);
        void setAutoConnect(bool value);

        QString userName() const;
        QString hash() const;
        QString deviceId() const;
        QString product() const;
        bool autoConnect() const;


        void update_workplace_data(const nlohmann::json& object);
        void update_workplace_view(const nlohmann::json& object);

        arcirk::database::devices& workplace_options();

        QString getDavService() const;
        QString getHttpService() const;
        QString getHttpPassword()  const;
        QString getHttpUser();

        bool davSsl(){
            return conf.WebDavSSL;
        }

        QString davUser() const{
            return QString(conf.WebDavUser.data());
        }

        QString davPassword() const{
            return QString(conf.WebDavPwd.data());
        }

        bool isShowImage();

        bool keyboardInputMode();
        bool priceCheckerMode();

        QString host() const;
        int port();

    private:
        QString m_host;
        int m_port;
        QString m_userName;
        QString m_hash;
        QUuid m_device_id;
        QString m_product;
        QString m_httpService;
        QString m_davService;
        QString m_httpPwd;
        bool m_showImage;
        bool m_keyboardInputMode;
        bool m_priceCheckerMode;

        arcirk::server::server_config conf;
        arcirk::client::client_private_config client_conf;
        arcirk::database::devices m_workplace;
        arcirk::database::devices_view m_workplace_view;

        QString davService() const;
        QString httpService() const;
        QString httpPwd() const;
        QString httpUser() const;

        bool showImage();        

        void read_conf();
        void read_private_conf();
        void write_conf();
        void write_private_conf();

        void init_device_id();

    signals:
        void hostChanged();
        void portChanged();
        void userNameChanged();
        void hashChanged();
        void deviceIdChanged();
        void productChanged();
        void httpServiceChanged();
        void davServiceChanged();
        void httpPwdChanged();
        void showImageChanged();
        void keyboardInputModeChanged();
        void priceCheckerModeChanged();
        void httpUserChanged();
        void autoConnectChanged();
        void updateWorkplaceView(const QString& org, const QString& suborg, const QString& stock, const QString& price);
        void optionsChanged();
    };

}

#endif // WS_SETTINGS_H
