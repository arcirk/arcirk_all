#include "dialogmain.h"
#include "iface/iface.hpp"
#include "ui_dialogmain.h"
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include "dialogmplitem.h"
#include "dialogmstsc.h"
#include "dialoginfo.h"
#include "commandline.h"
#include "cryptcontainer.h"
#include "cryptcertificate.h"
#include <QStandardPaths>
#include <QFileDialog>
#include <QNetworkInterface>
#include <QStorageInfo>
#include <QStringList>
#include <QSettings>
#include "dialogselectintree.h"
#include "dialogselectauth.h"
#include "dialogremoteusers.h"
#include "commandlineparser.h"
#ifdef Q_OS_WINDOWS
#include <Windows.h>
#include <shellapi.h>
#endif
#include <QTemporaryFile>
#include <QColor>
#include <QToolButton>
#include "dialogtask.h"
#include <QToolButton>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QPluginLoader>
#include <QTabWidget>
#include <QMovie>
#include "facelib.h"
//#include "taskparamdialog.h"


namespace arcirk::cryptography {

    inline QDir cryptoProDirectory(){
        QString programFilesPath(QDir::fromNativeSeparators(getenv("PROGRAMFILES")));
        QString programFilesPath_x86 = programFilesPath;
        programFilesPath_x86.append(" (x86)");

        QDir x64(programFilesPath + "/Crypto Pro/CSP");
        QDir x86(programFilesPath_x86 + "/Crypto Pro/CSP");
        QDir result;
        if(x86.exists()){
            QFile cryptcp(x86.path() + "/cryptcp.exe");
            if(cryptcp.exists()){
                result = x86;
            }
        }else
            if(x64.exists()){
                QFile cryptcp(x86.path() + "/cryptcp.exe");
                if(cryptcp.exists())
                result = x64;
            }

        return result;
    }

}

using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;


DialogMain::DialogMain(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogMain)
{
    ui->setupUi(this);

    m_show_alerts = false;

    treeViewMpl = new TreeViewWidget(this);
    ui->verticalGeneral->addWidget(treeViewMpl);
    treeCertificates = new TreeViewWidget(this);
    ui->verticalLayoutCerts->addWidget(treeCertificates);
    treeContainers = new TreeViewWidget(this);
    ui->verticalLayoutConts->addWidget(treeContainers);
    treeViewMstsc = new TreeViewWidget(this);
    treeViewMstsc->enable_sort(false);
    ui->verticalLayoutMstsc->addWidget(treeViewMstsc);
    treeAvailableCerts = new TreeViewWidget(this);
    ui->verticalLayoutAvCerts->addWidget(treeAvailableCerts);
    treeTasks = new TreeViewWidget(this);
    treeTasks->setObjectName("treeTasks");
    treeTasks->enable_sort(false);
    ui->verticalLayoutTsacs->addWidget(treeTasks);

    current_url = "ws://127.0.0.1:8080";
    column_aliases();
    is_localhost_ = true;

    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
            m_local_addresses.append(QString("ws://%1:8080").arg(address.toString()));
    }

    QMenu *menu = new QMenu();
    foreach(auto item, m_local_addresses){
        QAction *action = new QAction(item, this);
        connect(action, &QAction::triggered, this, &DialogMain::set_address);
        menu->addAction(action);
    }
    ui->btnHosts->setMenu(menu);
    ui->btnHosts->setPopupMode(QToolButton::InstantPopup);

    this->createTrayActions();
    this->createTrayIcon();
    this->trayIcon->show();

    current_user = new CertUser(this);
    current_user->setLocalhost(true);

    m_client = new IWClient(this);
    connect(m_client, &WebSocketClient::connectionChanged, this, &DialogMain::connectionChanged);
    connect(m_client, &WebSocketClient::connectionSuccess, this, &DialogMain::connectionSuccess);
    connect(m_client, &WebSocketClient::displayError, this, &DialogMain::displayError);
    connect(m_client, &WebSocketClient::serverResponse, this, &DialogMain::serverResponse);
    connect(m_client, &WebSocketClient::commandToClientResponse, this, &DialogMain::onCommandToClient);
    connect(m_client, &WebSocketClient::userInfo, this, &DialogMain::onUserInfo);

    create_tasks_model();

    read_data_chche();

    m_client->set_system_user(current_user->user_name());

    infoIco = new QLabel(this);
    auto mv = new QMovie(":/img/animation_loaded.gif");
    mv->setScaledSize(QSize(16,16));
    infoIco->setMovie(mv);
    infoIco->setMaximumHeight(16);
    infoIco->setVisible(false);
    infoBar = new QLabel(this);
    ui->statusbar->addWidget(infoIco);
    ui->statusbar->addWidget(infoBar);
    infoBar->setText("Не подключен");

    ui->chUseSid->setChecked(m_client->conf().use_sid);

    reconnect();

    setWindowTitle(QString("Менеджер профилей (%1)").arg(current_user->getInfo().user.c_str()));

    createDynamicMenu();

    ui->buttonBox->button(QDialogButtonBox::Save)->setText("Закрыть");

    mozillaApp = new QProcess(this);

    connect(ui->btnCertInfo, &QToolButton::clicked, this, &DialogMain::onBtnCertInfoClicked);
    connect(ui->btnMplItemEdit, &QToolButton::clicked, this, &DialogMain::onBtnMplItemEditClicked);
    connect(ui->btnMplItemAdd, &QToolButton::clicked, this, &DialogMain::onBtnMplItemAddClicked);
    connect(ui->btnMstscMoveUp, &QToolButton::clicked, this, &DialogMain::onBtnMstscMoveUpClicked);
    connect(ui->btnParam, &QToolButton::clicked, this, &DialogMain::onBtnParamClicked);
    connect(ui->btnMplItemMoveDown, &QToolButton::clicked, this, &DialogMain::onBtnMplItemMoveDownClicked);
    connect(ui->btnMplItemMoveUp, &QToolButton::clicked, this, &DialogMain::onBtnMplItemMoveUpClicked);
    connect(ui->btnMplItemDelete, &QToolButton::clicked, this, &DialogMain::onBtnMplItemDeleteClicked);
    connect(ui->btnProfileCopy, &QToolButton::clicked, this, &DialogMain::onBtnProfileCopyClicked);
    connect(ui->btnCertDelete, &QToolButton::clicked, this, &DialogMain::onBtnCertDeleteClicked);
    connect(ui->btnMstscCopy, &QToolButton::clicked, this, &DialogMain::onBtnMstscCopyClicked);
    connect(ui->btnMstscMoveDown, &QToolButton::clicked, this, &DialogMain::onBtnMstscMoveDownClicked);
    connect(ui->btnMstscRemove, &QToolButton::clicked, this, &DialogMain::onBtnMstscRemoveClicked);
    connect(ui->btnMstscEdit, &QToolButton::clicked, this, &DialogMain::onBtnMstscEditClicked);
    connect(ui->btnMstscAdd, &QToolButton::clicked, this, &DialogMain::onBtnMstscAddClicked);
    connect(ui->btnMstsc, &QToolButton::clicked, this, &DialogMain::onBtnMstscClicked);
    connect(ui->btnSelectUser, &QToolButton::clicked, this, &DialogMain::onBtnSelectUserClicked);
    connect(ui->btnImportSettings, &QToolButton::clicked, this, &DialogMain::onBtnImportSettingsClicked);
    connect(ui->btnResetCertIlst, &QToolButton::clicked, this, &DialogMain::onBtnResetCertIlstClicked);
    connect(ui->btnCertAdd, &QToolButton::clicked, this, &DialogMain::onBtnCertAddClicked);
    connect(ui->btnSelectPathFirefox, &QToolButton::clicked, this, &DialogMain::onBtnSelectPathFirefoxClicked);
    connect(ui->btnPwdView, &QToolButton::toggled, this, &DialogMain::onBtnPwdViewToggled);
    connect(ui->btnPwdEdit, &QToolButton::toggled, this, &DialogMain::onBtnPwdEditToggled);
    connect(ui->chUseSid, &QCheckBox::toggled, this, &DialogMain::onChUseSidToggled);
    connect(ui->chkWriteLog, &QCheckBox::toggled, this, &DialogMain::onChkWriteLogToggled);
    connect(ui->radioRunAsAdmin, &QRadioButton::toggled, this, &DialogMain::onRadioRunAsAdminToggled);
    connect(ui->radioRunAs, &QRadioButton::toggled, this, &DialogMain::onRadioRunAsToggled);
    connect(treeViewMpl, &QTreeView::doubleClicked, this, &DialogMain::onTreeViewMplDoubleClicked);
    connect(treeViewMstsc, &QTreeView::doubleClicked, this, &DialogMain::onTreeViewMstscDoubleClicked);
    connect(ui->txtServer, &QLineEdit::editingFinished, this, &DialogMain::onTxtServerEditingFinished);
    connect(ui->txtServerUser, &QLineEdit::editingFinished, this, &DialogMain::onTxtServerUserEditingFinished);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogMain::onButtonBoxAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DialogMain::onButtonBoxRejected);
    connect(ui->tabCrypt, &QTabWidget::tabBarClicked, this, &DialogMain::onTabCryptTabBarClicked);
    connect(ui->btnInstallPlugin, &QToolButton::clicked, this, &DialogMain::onBtnInstallBpugin);
}

void DialogMain::createTrayActions()
{
    qDebug() << __FUNCTION__;
    quitAction = new QAction(tr("&Выйти"), this);
    connect(quitAction, &QAction::triggered, this, &DialogMain::onAppExit);
    showAction = new QAction(tr("&Открыть менеджер профилей"), this);
    showAction->setIcon(QIcon(":/img/mpl.png"));
    connect(showAction, &QAction::triggered, this, &DialogMain::onWindowShow);
    checkIpAction = new QAction(tr("&Проверить IP адресс"), this);
    connect(checkIpAction, &QAction::triggered, this, &DialogMain::onCheckIP);
    openFiefox = new QAction(tr("&Открыть Firefox с выбором профиля"), this);
    connect(openFiefox, &QAction::triggered, this, &DialogMain::openMozillaFirefox);
    installCertAction = new QAction(tr("&Установить сертификат"), this);
    connect(installCertAction, &QAction::triggered, this, &DialogMain::onInstallCertificate);

    trayIconMenu = new QMenu(this);

}

void DialogMain::createTrayIcon()
{
    qDebug() << __FUNCTION__;
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    QIcon icon = QIcon(":/img/mpl.ico");
    trayIcon->setIcon(icon);
    setWindowIcon(icon);

    trayIcon->setToolTip("Менеджер профилей");

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &DialogMain::trayMessageClicked);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &DialogMain::trayIconActivated);

}

void DialogMain::createDynamicMenu()
{
    qDebug() << __FUNCTION__;
    trayIconMenu->clear();
    trayIconMenu->addAction(showAction);
    trayIconMenu->addAction(checkIpAction);
    trayIconMenu->addAction(openFiefox);
    trayIconMenu->addAction(installCertAction);

    auto cache = current_user->cache();
    auto mstsc_param = cache.value("mstsc_param", json::object());
    auto mpl_ = arcirk::internal_structure<arcirk::client::mpl_options>("mpl_options", cache);
    //auto use_firefox = mpl_.use_firefox;
    auto firefox_path = mpl_.firefox_path;
    auto is_connect_to_users = mstsc_param.value("enable_mstsc_users_sess", false);

    if(is_connect_to_users){
        trayIconMenu->addSeparator();
        auto action = new QAction("Подключиться к сеансу пользователя", this);
        action->setIcon(QIcon(":/img/mstscUsers.png"));
        trayIconMenu->addAction(action);
        connect(action, &QAction::triggered, this, &DialogMain::onTrayMstscConnectToSessionTriggered);
    }

    if(!mstsc_param.empty()){
        auto is_enable = mstsc_param.value("enable_mstsc", false);
        auto detailed_records = mstsc_param.value("detailed_records", json::array());
        if(is_enable && detailed_records.size() > 0){
            trayIconMenu->addSeparator();
            for(auto itr = detailed_records.begin(); itr != detailed_records.end(); ++itr){
                 auto object = *itr;
                 auto mstsc = arcirk::secure_serialization<arcirk::client::mstsc_options>(object, __FUNCTION__);
                 QString name = mstsc.name.c_str();
                 auto action = new QAction(name, this);
                 action->setProperty("data", object.dump().c_str());
                 action->setProperty("type", "mstsc");
                 action->setIcon(QIcon(":/img/mstsc.png"));
                 trayIconMenu->addAction(action);
                 connect(action, &QAction::triggered, this, &DialogMain::onTrayTriggered);
            }
        }
    }

    if(mpl_.mpl_list.size() > 0){
        auto table_str = arcirk::byte_array_to_string(mpl_.mpl_list);
        auto table_j = json::parse(table_str);
        if(table_j.is_object()){
            auto mpl_list = table_j.value("rows", json::array());
            trayIconMenu->addSeparator();
            for (auto itr = mpl_list.begin(); itr != mpl_list.end(); ++itr) {
                auto object = *itr;
                auto mpl = secure_serialization<arcirk::client::mpl_item>(object);
                QString name = mpl.name.c_str();
                auto action = new QAction(name, this);
                action->setProperty("data", object.dump().c_str());
                action->setProperty("type", "link");
                action->setIcon(get_link_icon(mpl.url.c_str()));
                trayIconMenu->addAction(action);
                connect(action, &QAction::triggered, this, &DialogMain::onTrayTriggered);
            }
        }
    }
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
}

void DialogMain::read_data_chche()
{
    qDebug() << __FUNCTION__;

    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if(!dir.exists())
        dir.mkpath(path);

    auto fileName= path + "/" + CACHE_FILENAME;
    if(QFile::exists(fileName)){

        QFile f(fileName);
        f.open(QIODevice::ReadOnly);

        std::string m_text = f.readAll().toStdString();
        f.close();

        try {
            auto m_cache = nlohmann::json::parse(m_text);
            current_user->set_database_cache(m_cache);
            read_cache(m_cache);
        } catch (std::exception& e) {
            qCritical() << __FUNCTION__ << e.what();
        }
    }else{
        read_cache(current_user->cache());
    }
}

DialogMain::~DialogMain()
{
    delete ui;
}

void DialogMain::accept()
{
    qDebug() << __FUNCTION__;

    write_cache();

    //QDialog::accept();
    QDialog::hide();

}

void DialogMain::write_cache(){

    if(m_write_log)
        write_temp_log(__FUNCTION__, "start write");

    if(ui->chkStandradPass->isChecked()){
        m_client->client_server_param().hash = WebSocketClient::generateHash(m_client->client_server_param().user_name.c_str(), m_client->client_server_param().user_uuid.c_str()).toStdString();
    }else{
        if(ui->btnPwdEdit->isChecked()){
            m_client->client_server_param().hash = WebSocketClient::generateHash(m_client->client_server_param().user_name.c_str(), ui->lblPwd->text()).trimmed().toStdString();
        }
    }
    auto m_cache = current_user->cache();
    m_cache["client_param"] = pre::json::to_json(m_client->client_server_param());
    m_cache["server_config"] = pre::json::to_json(m_client->server_conf());

    m_cache["standard_password"] = ui->chkStandradPass->isChecked();
    m_cache["auto_connect"] = ui->chkAutoConnect->isChecked();

    m_cache["use_sid"] = ui->chUseSid->isChecked();

    if(m_write_log)
        write_temp_log(__FUNCTION__, "write struct");
    if(m_write_log)
        write_temp_log(__FUNCTION__, "write_mstsc_param");
    write_mstsc_param(m_cache);
    if(m_write_log)
        write_temp_log(__FUNCTION__, "write_mpl_options");
    write_mpl_options(m_cache);
    if(m_write_log)
        write_temp_log(__FUNCTION__, "write_crypt_data");
    write_crypt_data(m_cache);
//    if(m_write_log)
//        write_temp_log(__FUNCTION__, "write_available_certificates");
//    write_available_certificates(m_cache);
    if(m_write_log)
        write_temp_log(__FUNCTION__, "save_cache_to_local");
    save_cache_to_local();

    if(m_write_log)
        write_temp_log(__FUNCTION__, "set to database");

    write_tasks_param(m_cache);
    current_user->set_database_cache(m_cache);

    m_cache["write_log"] = m_write_log;


    update_cache_on_server();

    if(m_write_log)
        write_temp_log(__FUNCTION__, "update files");

    auto detailed_records = mstsc_param.value("detailed_records", json::array());
    update_rdp_files(detailed_records);

    if(m_write_log)
        write_temp_log(__FUNCTION__, "create menu");

    createDynamicMenu();

    if(m_write_log)
        write_temp_log(__FUNCTION__, "end write");

}

void DialogMain::update_cache_on_server(){

    qDebug() << __FUNCTION__;

    if(!m_client->isConnected())
        return;

    auto m_object = current_user->cert_user_data();
    auto query_param = nlohmann::json{
        {"table_name", enum_synonym(tables::tbCertUsers)},
        {"where_values", nlohmann::json{
            {"uuid", m_object.uuid}, // ограничения доступа на запись по хосту и идетификатору
            {"host", m_object.host}}
        },
        {"values", pre::json::to_json(m_object)},
        {"query_type", "update"}
    };

    m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                               {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                           });
}

void DialogMain::update_rdp_files(const nlohmann::json &items)
{
    qDebug() << __FUNCTION__;

    QDir dir(cache_mstsc_directory());
    QStringList lstFiles = dir.entryList(QDir::Files);
    foreach (QString entry, lstFiles)
    {
        QString entryAbsPath = dir.absolutePath() + "/" + entry;
        QFile::setPermissions(entryAbsPath, QFile::ReadOwner | QFile::WriteOwner);
        QFile::remove(entryAbsPath);
    }
    if(items.is_array()){
        for (auto it = items.begin(); it != items.end(); ++it) {
            auto item = arcirk::secure_serialization<arcirk::client::mstsc_options>(*it, __FUNCTION__);
            if(item.uuid.empty()){
                continue;
            }else{
                QString file_name = dir.path() + "/";
                //file_name.append(item.uuid.c_str());
                file_name.append(item.name.c_str());
                file_name.append(".rdp");
                QFile f(file_name);
                if(f.open(QIODevice::WriteOnly)){
                    int screenMode = 2;
                    if(item.not_full_window){
                        screenMode = 1;
                    }
                    QString rdp = rdp_file_text().arg(QString::number(screenMode), QString::number(item.width), QString::number(item.height), item.user_name.c_str(), item.address.c_str());
                    f.write(rdp.toUtf8());
                    f.close();
                }
            }
        }

    }

}

QString DialogMain::cache_mstsc_directory()
{
    auto app_data = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    app_data.append("/mstsc");
    QDir f(app_data);
    if(!f.exists())
        f.mkpath(f.path());
    return f.path();
}

QString DialogMain::rdp_file_text()
{
    QString text = "screen mode id:i:%1\n"
            "use multimon:i:0\n"
            "desktopwidth:i:%2\n"
            "desktopheight:i:%3\n"
            "session bpp:i:32\n"
            "winposstr:s:0,3,0,0,%2,%3\n"
            "compression:i:1\n"
            "keyboardhook:i:2\n"
            "audiocapturemode:i:0\n"
            "videoplaybackmode:i:1\n"
            "connection type:i:7\n"
            "networkautodetect:i:1\n"
            "bandwidthautodetect:i:1\n"
            "displayconnectionbar:i:1\n"
            "username:s:%4\n"
            "enableworkspacereconnect:i:0\n"
            "disable wallpaper:i:0\n"
            "allow font smoothing:i:0\n"
            "allow desktop composition:i:0\n"
            "disable full window drag:i:1\n"
            "disable menu anims:i:1\n"
            "disable themes:i:0\n"
            "disable cursor setting:i:0\n"
            "bitmapcachepersistenable:i:1\n"
            "full address:s:%5\n"
            "audiomode:i:0\n"
            "redirectprinters:i:1\n"
            "redirectcomports:i:0\n"
            "redirectsmartcards:i:1\n"
            "redirectclipboard:i:1\n"
            "redirectposdevices:i:0\n"
            "autoreconnection enabled:i:1\n"
            "authentication level:i:2\n"
            "prompt for credentials:i:0\n"
            "negotiate security layer:i:1\n"
            "remoteapplicationmode:i:0\n"
            "alternate shell:s:\n"
            "shell working directory:s:\n"
            "gatewayhostname:s:\n"
            "gatewayusagemethod:i:4\n"
            "gatewaycredentialssource:i:4\n"
            "gatewayprofileusagemethod:i:0\n"
            "promptcredentialonce:i:0\n"
            "gatewaybrokeringtype:i:0\n"
            "use redirection server name:i:0\n"
            "rdgiskdcproxy:i:0\n"
            "kdcproxyname:s:";

    return text;
}

void DialogMain::send_notify(const std::string& command, const std::string& message, const nlohmann::json& param)
{
    qDebug() << __FUNCTION__;

    using json = nlohmann::json;

    qDebug() << param.dump().c_str();

    if(m_client->isConnected()){
        auto app_filter = json::array({"ServerManager"});
        auto query_param = nlohmann::json{
            {"app_filter", app_filter},
            {"command", command},
            {"message", message},
            {"param", param}
        };

        m_client->send_command(arcirk::server::server_commands::SendNotify, query_param);

    }
}

void DialogMain::save_cache_to_local()
{
    qDebug() << __FUNCTION__;

    auto m_cache = current_user->cache();
    try {
        std::string result = m_cache.dump() ;
        auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        auto fileName= path + "/" + CACHE_FILENAME;
        QFile f(fileName);
        qDebug() << fileName;
        f.open(QIODevice::WriteOnly);
        f.write(QByteArray::fromStdString(result));
        f.close();
    } catch (std::exception& e) {
        qCritical() << __FUNCTION__ << e.what();
    }
}

void DialogMain::open_user_mstsc_session(const json result)
{
    auto dlg = DialogRemoteUsers(result, this);
    dlg.setModal(true);
    dlg.exec();

    if(dlg.result() == QDialog::Accepted){

        int id = dlg.seletedId();
        bool management = dlg.management();

        QString cmd = "";
        bool run_as = false;

        std::string admin_mstsc = mstsc_param.value("mstsc_admin", "admin");
        if(admin_mstsc != "admin" || !admin_mstsc.empty()){
            QFile spc(QCoreApplication::applicationDirPath() + "/RunAsSpc.exe");
            if(!spc.exists()){
                QMessageBox::critical(this, "Ошибка", "Утилита запуска от имени пользователя не найдена!");
                return;
            }
            QString fileName = cache_mstsc_directory() + "/" + QString::fromStdString(crypt(admin_mstsc, CRYPT_KEY)) + ".spc";
            cmd = QString("%1 /cryptfile:\"%2\" param:").arg(QDir::toNativeSeparators(spc.fileName()), QDir::toNativeSeparators(fileName));
            run_as = true;
        }

        if(cmd.isEmpty())
            cmd = "mstsc ";

        QString param;

        if(management){
            param = QString("/shadow:%1 /control /noConsentPrompt").arg(id);
        }else
            param = QString("/shadow:%1 /noConsentPrompt").arg(id);

        if(!run_as)
            cmd.append(param);
        else{
            cmd.append("\"");
            cmd.append(param);
            cmd.append("\"");
        }

        if(!run_as){
            ShellExecute(0, L"runas", L"mstsc", param.toStdWString().c_str(), 0, SW_SHOWNORMAL);
        }else{
            auto terminal = CommandLine(this);
            QEventLoop loop;

            auto started = [&terminal, &cmd]() -> void
            {
                terminal.send(cmd, CmdCommand::COMMAND_INVALID);
            };
            loop.connect(&terminal, &CommandLine::started_process, started);

            QByteArray cmd_text;
            auto output = [&cmd_text](const QByteArray& data) -> void
            {
                cmd_text.append(data);
            };
            loop.connect(&terminal, &CommandLine::output, output);

            auto err = [&loop, &terminal](const QString& data, int command) -> void
            {
                qDebug() << __FUNCTION__ << data << command;
                terminal.stop();
                loop.quit();
            };
            loop.connect(&terminal, &CommandLine::error, err);

            auto state = [&loop]() -> void
            {
                loop.quit();
            };
            loop.connect(&terminal, &CommandLine::complete, state);

            terminal.start();
            loop.exec();
        }
    }
}

bool DialogMain::delete_certificate(const std::string &sha1)
{
    auto cmd = CommandLine(this);
    auto c_path = current_user->getCryptoProCSP();
    if(c_path.isEmpty())
        return false;
    cmd.setWorkingDirectory(c_path);
    QEventLoop loop;
    bool result = true;

    auto started = [&cmd, &sha1]() -> void
    {
        cmd.send(QString("certmgr -delete -thumbprint \"%1\" & exit").arg(sha1.c_str()), certmgrDeletelCert);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    auto output = [](const QByteArray& data) -> void
    {
        //std::string result_ = arcirk::to_utf(data.toStdString(), "cp866");
        //qDebug() << result_.c_str();
    };
    loop.connect(&cmd, &CommandLine::output, output);

    auto err = [&loop, &cmd, &result](const QString& data, int command) -> void
    {
        qDebug() << __FUNCTION__ << data << command;
        result = false;
        cmd.stop();
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::error, err);

    auto state = [&loop]() -> void
    {
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::complete, state);

    cmd.start();
    loop.exec();

    return result;
}

bool DialogMain::delete_container(const std::string &name)
{
    auto cmd = CommandLine(this);
    cmd.setWorkingDirectory(current_user->getCryptoProCSP());
    QEventLoop loop;
    bool result = true;

    auto started = [&cmd, &name]() -> void
    {
        auto text = QString("csptest -keyset -deletekeyset -container \"%1\" & exit").arg(name.c_str());
        qDebug() << text;
        cmd.send(text, csptestContainerDelete);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    auto output = [](const QByteArray& data) -> void
    {
        //std::string result_ = arcirk::to_utf(data.toStdString(), "cp866");
        //qDebug() << result_.c_str();
    };
    loop.connect(&cmd, &CommandLine::output, output);

    auto err = [&loop, &cmd, &result](const QString& data, int command) -> void
    {
        qDebug() << __FUNCTION__ << data << command;
        result = false;
        cmd.stop();
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::error, err);

    auto state = [&loop]() -> void
    {
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::complete, state);

    cmd.start();
    loop.exec();

    return result;
}

void DialogMain::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_MACOS
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif

    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}

void DialogMain::reconnect()
{
    if(m_client->isConnected())
        m_client->close();

    auto m_cache = current_user->cache();
    if(m_cache.is_null())
        m_client->open();
    else{
        //if(m_cache.value("use_sid", true))
            m_client->open(current_user->getInfo().sid.c_str());
//        else
//            m_client->open();
    }
}

void DialogMain::displayError(const QString &what, const QString &err)
{
    if(!m_show_alerts)
        return;

    qCritical() << __FUNCTION__ << what << ": " << err ;

    QIcon msgIcon(":/img/h.png");
    trayIcon->showMessage(what, err, msgIcon,
                          3 * 1000);
}

void DialogMain::connectionSuccess()
{
    qDebug() << __FUNCTION__;
    current_user->read_database_cache(m_client->url(), m_client->server_conf().ServerUserHash.c_str());
    reset_data();

}

void DialogMain::read_cache(const nlohmann::json &data)
{

    auto m_cache = data;
    if(m_cache.is_null())
        m_cache = json::object();

    current_user->set_database_cache(m_cache);

    if(!data.empty() && !data.is_null()){
        auto p = getStructData<arcirk::client::client_param>("client_param", data);
        if(!p.hash.empty()){
            m_client->client_server_param().user_name = p.user_name;
            m_client->client_server_param().hash = p.hash;
        }
        m_client->set_server_conf(m_cache.value("server_config", json::object()));
        mpl_ = getStructData<arcirk::client::mpl_options>("mpl_options", m_cache);
        crypt_data = getStructData<arcirk::client::cryptopro_data>("crypt_data", m_cache);
    }else{
        mpl_ = arcirk::client::mpl_options();
        crypt_data = arcirk::client::cryptopro_data();
    }

    if(!m_client->server_conf().ServerHost.empty()){
        if(!m_client->isConnected()){
            QString scheme = m_client->server_conf().ServerSSL ? "wss" : "ws";
            QUrl url{};
            url.setHost(m_client->server_conf().ServerHost.c_str());
            url.setPort(m_client->server_conf().ServerPort);
            url.setScheme(scheme);
            ui->txtServer->setText(url.toString());
        }else{
            ui->txtServer->setText(m_client->url().toString());
            onTxtServerEditingFinished();
        }
    }else{
        if(!m_client->isConnected()){
            QUrl url(current_url);
            m_client->server_conf().ServerSSL = url.scheme() == "wss" ? true : false;
            m_client->server_conf().ServerHost = url.host().toStdString();
            m_client->server_conf().ServerPort = url.port();
            ui->txtServer->setText(url.toString());
        }else{
            auto url = m_client->url();
            ui->txtServer->setText(url.toString());
        }

    }

    read_mstsc_param();

    ui->chkStandradPass->setCheckState(m_cache.value("standard_password", false) ? Qt::Checked : Qt::Unchecked);
    ui->chkAutoConnect->setCheckState(m_cache.value("auto_connect", true) ? Qt::Checked : Qt::Unchecked);
    ui->txtServerUser->setText(m_client->client_server_param().user_name.c_str());
    ui->chUseSid->setChecked(m_cache.value("use_sid", false));

    read_mpl_options();

    read_crypt_data();

    auto cli = client::client_conf();
    cli.is_auto_connect = ui->chkAutoConnect->isChecked();
    cli.app_name = APP_NAME;
    cli.user_name = m_client->client_server_param().user_name;
    cli.system_user = current_user->user_name().toStdString();
    cli.hash = m_client->client_server_param().hash;
    cli.server_host = ui->txtServer->text().toStdString();
    cli.use_sid = ui->chUseSid->isChecked();
    m_client->set_client_conf(pre::json::to_json(cli));

    read_available_certificates();

    m_write_log = m_cache.value("write_log", false);
    ui->chkWriteLog->setChecked(m_write_log);

    read_tasks_param();
}

void DialogMain::read_mstsc_param()
{

    QMap<QString, QString> m_aliases{
        qMakePair("name","Наименование"),
        qMakePair("address","Хост"),
        qMakePair("port","Порт"),
        qMakePair("def_port","Стандартный порт"),
        qMakePair("not_full_window","Режим окна"),
        qMakePair("width","Ширина"),
        qMakePair("height","Высота"),
        qMakePair("reset_user","Сброс пользователя"),
        qMakePair("user_name","Пользователь"),
        qMakePair("password","Пароль")
    };

    QVector<QString> m_order{
        "name",
        "address",
        "port",
        "user_name",
        "width",
        "height"
    };

    QVector<QString> m_hide{};
    for (auto itr = m_aliases.constBegin(); itr != m_aliases.constEnd(); ++itr) {
        if(m_order.indexOf(itr.key()) == -1)
           m_hide.append(itr.key());
    }

    auto m_cache = current_user->cache();
    mstsc_param = m_cache.value("mstsc_param", json::object());

    auto model = new ITree<arcirk::client::mstsc_options>(this);
    model->set_column_aliases(m_aliases);
    model->set_rows_icon(tree::Item, QIcon(":/img/mstsc.png"));
    model->set_hierarchical_list(false);

    if(!mstsc_param.empty()){
        ui->chkEnableMstsc->setCheckState(mstsc_param.value("enable_mstsc", false) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
        ui->chkEnableMstscUserSessions->setCheckState(mstsc_param.value("enable_mstsc_users_sess", false) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
        std::string admin_mstsc = mstsc_param.value("mstsc_admin", "admin");
        if(admin_mstsc == "admin" || admin_mstsc.empty()){
            ui->radioRunAsAdmin->setChecked(true);
        }else{
            ui->radioRunAs->setChecked(true);
            ui->txtRuAsName->setText(admin_mstsc.c_str());
        }

        auto detailed_records = mstsc_param.value("detailed_records", json::array());
        if(!detailed_records.is_array())
            detailed_records = json::array();

        model->set_columns_order(m_order);
        model->add_array(detailed_records);

    }else{
        ui->chkEnableMstsc->setCheckState( Qt::CheckState::Unchecked );
        ui->chkEnableMstscUserSessions->setCheckState(Qt::CheckState::Unchecked );
        ui->radioRunAsAdmin->setChecked(true);
    }

    treeViewMstsc->setModel(model);
    treeViewMstsc->hideColumn(model->column_index("ref"));
    treeViewMstsc->hideColumn(model->column_index("parent"));
    treeViewMstsc->hideColumn(model->column_index("uuid"));
    treeViewMstsc->hideColumn(model->column_index("password"));
    treeViewMstsc->hideColumn(model->column_index("width"));
    treeViewMstsc->hideColumn(model->column_index("height"));
    treeViewMstsc->hideColumn(model->column_index("reset_user"));
    treeViewMstsc->hideColumn(model->column_index("not_full_window"));
    treeViewMstsc->hideColumn(model->column_index("def_port"));
    treeViewMstsc->hideColumn(model->column_index("is_group"));

}

void DialogMain::write_mstsc_param(nlohmann::json &data)
{
    mstsc_param["enable_mstsc"] = ui->chkEnableMstsc->checkState() == Qt::CheckState::Checked;
    mstsc_param["enable_mstsc_users_sess"] = ui->chkEnableMstscUserSessions->checkState() == Qt::CheckState::Checked;
    auto mstsc_admin = ui->txtRuAsName->text().trimmed();
    mstsc_param["mstsc_admin"] = mstsc_admin.isEmpty() ? "admin" : mstsc_admin.toStdString();
    auto model = treeViewMstsc->get_model();
    if(model && model->columnCount(QModelIndex()) > 0)
        mstsc_param["detailed_records"] = model->to_array(QModelIndex());

    data["mstsc_param"] = mstsc_param;
}

void DialogMain::read_mpl_options()
{
    ui->chUseFirefox->setChecked(mpl_.use_firefox);
    if(mpl_.firefox_path.empty())
        ui->txtFirefoxPath->setText("C:/Program Files/Mozilla Firefox");
    else
        ui->txtFirefoxPath->setText(mpl_.firefox_path.c_str());

    m_moz_profiles.clear();
    if(mpl_.mpl_profiles.size() > 0){
        auto lst_str = arcirk::byte_array_to_string(mpl_.mpl_profiles);
        auto lst_j = json::parse(lst_str);
        if(lst_j.is_array()){
            for (auto itr = lst_j.begin(); itr != lst_j.end(); ++itr) {
                std::string s = *itr;
                m_moz_profiles.append(s.c_str());
            }
        }

    }

    auto model = new ITree<mpl_item>(this);
    model->set_hierarchical_list(false);
    model->set_column_aliases(QMap<QString,QString>{
                                  qMakePair("name", "Наименование"),
                                  qMakePair("url", "Url"),
                                  qMakePair("profile", "Профиль")
                              });
    if(mpl_.mpl_list.size() > 0){
        auto table_str = arcirk::byte_array_to_string(mpl_.mpl_list);
        if(json::accept(table_str)){
            auto table_j = json::parse(table_str);
            if(table_j.is_object())
                model->set_table(table_j);
        }
        model->set_columns_order(QVector<QString>{"name", "profile", "url"});
        //model->reset();
    }

    treeViewMpl->enable_sort(false);
    treeViewMpl->setModel(model);
    treeViewMpl->hide_default_columns();
    update_mpl_items_icons();

}

void DialogMain::write_mpl_options(nlohmann::json &data)
{
    mpl_.use_firefox = ui->chUseFirefox->isChecked();
    mpl_.firefox_path = ui->txtFirefoxPath->text().toStdString();

    auto model = treeViewMpl->get_model();
    mpl_.mpl_list.clear();
    if(model && model->columnCount(QModelIndex()) > 0){
        auto table = model->to_table_model(QModelIndex());
        auto table_ba = arcirk::string_to_byte_array(table.dump());
        mpl_.mpl_list = ByteArray(table_ba.size());
        std::copy(table_ba.begin(), table_ba.end(), mpl_.mpl_list.begin());
    }

    auto prof = json::array();
    mpl_.mpl_profiles.clear();
    foreach (auto it, m_moz_profiles) {
        prof += it.toStdString();
    }
    auto ba = arcirk::string_to_byte_array(prof.dump());
    mpl_.mpl_profiles = ByteArray(ba.size());
    std::copy(ba.begin(), ba.end(), mpl_.mpl_profiles.begin());

    data["mpl_options"] = pre::json::to_json(mpl_);
}

void DialogMain::read_crypt_data()
{
    if(crypt_data.cryptopro_path.empty()){
        crypt_data.cryptopro_path = "C:/Program Files (x86)/Crypto Pro/CSP";
    }
    ui->txtCryptoProPath->setText(crypt_data.cryptopro_path.c_str());
    onBtnResetCertIlstClicked();
}

void DialogMain::write_crypt_data(nlohmann::json &data)
{
    try {
        crypt_data.cryptopro_path = ui->txtCryptoProPath->text().toStdString();
        auto model = treeCertificates->get_model();
        Q_ASSERT(model != 0);
        crypt_data.certs.clear();
        if(model && model->columnCount(QModelIndex()) > 0){
            auto table = model->to_table_model(QModelIndex());
            auto table_ba = arcirk::string_to_byte_array(table.dump());
            crypt_data.certs = ByteArray(table_ba.size());
            std::copy(table_ba.begin(), table_ba.end(), crypt_data.certs.begin());
        }

        model = treeContainers->get_model();
        Q_ASSERT(model != 0);
        crypt_data.conts.clear();
        if(model && model->columnCount(QModelIndex()) > 0){
            auto table = model->to_table_model(QModelIndex());
            auto table_ba = arcirk::string_to_byte_array(table.dump());
            crypt_data.conts = ByteArray(table_ba.size());
            std::copy(table_ba.begin(), table_ba.end(), crypt_data.conts.begin());
        }

        data["crypt_data"] = pre::json::to_json(crypt_data);
    } catch (const QException& e) {
        if(m_write_log)
            write_temp_log(__FUNCTION__, e.what());
        qCritical() << e.what();
    }

}

void DialogMain::connectionChanged(bool state)
{
    if(!state){
        infoBar->setText("Не подключен");
        m_client->checkConnection();
    }else{
        infoBar->setText(QString("Подключен: %1 (%2)").arg(m_client->conf().server_host.c_str(), m_client->server_conf().ServerName.c_str()));
    }
    QPushButton* pApplyButton = ui->buttonBox->button(QDialogButtonBox::Save);
    if(pApplyButton){
        pApplyButton->setEnabled(state);
    }
}

template<typename T>
T DialogMain::getStructData(const std::string &name, const json &source)
{
    auto result = T();
//    try {
        auto obg = source.value(name, json::object());
        if(!obg.empty())
            result = secure_serialization<T>(obg);
//    } catch (const std::exception& e) {
//       qCritical() <<  e.what();
//    }

    return result;
}


void DialogMain::openConnection()
{
    if(m_client->isConnected())
            m_client->close();

    m_client->open();
}

void DialogMain::closeConnection()
{
    if(m_client->isConnected())
        m_client->close();
}

void DialogMain::serverResponse(const arcirk::server::server_response &message)
{
    //qDebug() << __FUNCTION__ << message.command.c_str() << message.message.c_str();
    using json = nlohmann::json;
    using namespace arcirk::server;

    if(message.command == arcirk::enum_synonym(server_commands::ExecuteSqlQuery)){
        if(message.result == WS_RESULT_SUCCESS){
            auto param = json::parse(QByteArray::fromBase64(message.param.c_str()));
            if(!param.is_object())
                return;
            auto query_param_base64 = param.value("query_param", "");
            if(query_param_base64.empty())
                return;

            auto query_param = json::parse(QByteArray::fromBase64(query_param_base64.c_str()));
            //qDebug() << param.dump().c_str();
            auto query_type = query_param.value("query_type", "");
            auto table_name = query_param.value("table_name", "");
            if(table_name == arcirk::enum_synonym(arcirk::database::tables::tbCertUsers)){
                if(query_type == "update"){
                    send_notify("update_certUsers", "", json{
                                    {"session_id", m_client->client_server_param().session_uuid},
                                    {"sid", m_client->client_server_param().sid},
                                    {"system_user", m_client->client_server_param().system_user},
                                    {"host", m_client->client_server_param().host_name}});
                }
            }
        }else if(message.result == WS_RESULT_ERROR){

        }
    }else if(message.command == arcirk::enum_synonym(arcirk::server::server_commands::SendNotify)){
        if(message.message != WS_RESULT_SUCCESS && !message.message.empty())
            qDebug() << message.message.c_str();
    }
}

void DialogMain::onCommandToClient(const arcirk::server::server_response &message)
{
    qDebug() << __FUNCTION__;
    using json = nlohmann::json;

    auto param = json::parse(QByteArray::fromBase64(QByteArray::fromStdString(message.param)).toStdString());
    auto command = param.value("command", "");
    qDebug() << command.c_str();
    if(command == "ResetCache"){
        current_user->read_database_cache(m_client->url(), m_client->client_server_param().hash.c_str());
        reset_data();
    }else if(command == "GetCertificatesList"){
        current_user->setLocalhost(true);
        auto certs = current_user->getCertificates(true);
        m_client->command_to_client(message.sender, "onGetCertificatesList", certs);
    }else if(command == "GetLocalUsers"){
        m_client->command_to_client(message.sender, "GetLocalUsers", get_local_users_details());
    }

}

void DialogMain::onTrayTriggered()
{
    qDebug() << __FUNCTION__;

    auto *action = dynamic_cast<QAction*>( sender() );
    QString type_action = action->property("type").toString();

    auto dt = action->property("data").toString().toStdString();

    if(type_action == "mstsc"){
        auto item = arcirk::secure_serialization<arcirk::client::mstsc_options>(json::parse(dt), __FUNCTION__);
        run_mstsc_link(item);
    }else if(type_action == "link"){
        auto item = arcirk::secure_serialization<arcirk::client::mpl_item>(json::parse(dt), __FUNCTION__);
        auto defPage = item.url;
        auto profName = item.profile;
        //открываем адрес указанный на флешке банка
        if(defPage == BANK_CLIENT_USB_KEY){
            foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
                if (storage.isValid()) {
                    qDebug() << storage.rootPath() + QDir::separator() + BANK_CLIENT_FILE;
                    QFile file = QFile(storage.rootPath() + QDir::separator() + BANK_CLIENT_FILE);
                    if(file.exists()){
                        QSettings lnk(file.fileName(), QSettings::IniFormat);
                        QStringList keys = lnk.allKeys();
                        foreach(const QString& key, keys){
                            if(key.compare("InternetShortcut")){
                                if(key.endsWith("/URL")){
                                    defPage = lnk.value(key).toString().toStdString();
                                    break;
                                }

                            }
                        }
                    }
                }
            }
        }

        QStringList args;

        //args.append("-new-instance");
        if(!profName.empty()){
            args.append("-P");
            args.append(profName.c_str());
            if(!defPage.empty()){
                 QFile file = QFile(defPage.c_str());
                 if(file.exists())
                     defPage = "file:///" + defPage;
                args.append("-URL");
                args.append(defPage.c_str());
            }
        }

        QDir dir(mpl_.firefox_path.c_str());
        QString exeFile = "firefox";
#ifdef Q_OS_WINDOWS
        exeFile.append(".exe");
#endif
        QFile exe(dir.path() + "/" + exeFile);
        if(!dir.exists() || !exe.exists()){
            QString file = QFileDialog::getOpenFileName(this, tr("Путь к firefox"),
                                                         QDir::homePath(),
                                                         exeFile);
            if(file != ""){
                QFileInfo fi(file);
                exe.setFileName(file);
                mpl_.firefox_path = QDir::toNativeSeparators(fi.absolutePath()).toStdString();
            }
        }

        if(exe.exists()){
            qDebug() << mozillaApp->state();
            mozillaApp->terminate();
            mozillaApp->kill();
            mozillaApp->waitForFinished();
            mozillaApp->start(exe.fileName(), args);
        }
    }
}

void DialogMain::onTrayMstscConnectToSessionTriggered()
{

    auto cmd = CommandLine(this);
    QEventLoop loop;

    auto started = [&cmd]() -> void
    {
        cmd.send("quser & exit", CmdCommand::quserList);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    QByteArray cmd_text;
    auto output = [&cmd_text](const QByteArray& data) -> void
    {
        cmd_text.append(data);
    };
    loop.connect(&cmd, &CommandLine::output, output);

    auto err = [&loop, &cmd](const QString& data, int command) -> void
    {
        qDebug() << __FUNCTION__ << data << command;
        cmd.stop();
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::error, err);

    auto state = [&loop]() -> void
    {
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::complete, state);

    cmd.start();
    loop.exec();

    std::string result_ = arcirk::to_utf(cmd_text.toStdString(), "cp866");

    auto result = CommandLineParser::parse_users_sessions(result_.c_str());

    open_user_mstsc_session(result);
}

void DialogMain::onAppExit()
{
    qDebug() << __FUNCTION__;

//    if(terminal)
//        terminal->stop();
//    if(m_client)
//        if(m_client->isStarted())
//            m_client->close(true);
//    if(db)
//        if(db->isOpen())
//            db->close();

    QApplication::exit();
}

void DialogMain::onWindowShow()
{
    qDebug() << __FUNCTION__;

    setVisible(true);
}

void DialogMain::trayMessageClicked()
{
 qDebug() << __FUNCTION__;
}

void DialogMain::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    qDebug() << __FUNCTION__ << reason;

    if(reason == QSystemTrayIcon::DoubleClick){
        setVisible(true);
    }else if(reason == QSystemTrayIcon::Context){
        qDebug() << "QSystemTrayIcon::Context";
    }

}

void DialogMain::trayShowMessage(const QString &msg, int isError)
{
    if(!m_show_alerts)
        return;

    if(!isError)
        trayIcon->showMessage("Менеджер сертификатов", msg);
    else{

    }
}


QModelIndex DialogMain::findInTable(QAbstractItemModel * model, const QString &value, int column, bool findData)
{
    int rows =  model->rowCount();
    for (int i = 0; i < rows; ++i) {
        auto index = model->index(i, column);
        if(findData){
            if(value == index.data(Qt::UserRole + 1).toString())
                return index;
        }else{
            QString data = index.data().toString();
            if(value == data)
                return index;
        }
    }

    return QModelIndex();
}

void DialogMain::onBtnImportSettingsClicked()
{
    QUrl ws(ui->txtServer->text());
    Q_ASSERT(ws.isValid());

    if(m_client->isConnected())
        m_client->close();
    m_client->server_conf().ServerHost = ws.host().toStdString();
    m_client->server_conf().ServerPort = ws.port();
    m_client->server_conf().ServerSSL = ws.scheme() == "ws" ? false : true;
    m_client->conf().server_host = ws.toString().toStdString();
    m_client->conf().user_name = ui->txtServerUser->text().toStdString();
    if(!ui->chkStandradPass->isChecked()){
        if(ui->btnPwdEdit->isChecked()){
           m_client->conf().hash = WebSocketClient::generateHash(ui->txtServerUser->text(), ui->lblPwd->text()).toStdString();
        }
    }
    reconnect();

}


void DialogMain::onChUseSidToggled(bool checked)
{
   m_client->conf().use_sid = checked;
}


void DialogMain::form_control()
{
    bool as_admin = ui->radioRunAsAdmin->isChecked();
    ui->txtRuAsName->setEnabled(!as_admin);
    ui->btnSetMstscPwd->setEnabled(!as_admin);

    update_mpl_items_icons();

    auto cache = current_user->cache();
    auto is_allow_profile_manager = cache.value("allow_profile_manager", false);
    auto is_allow_cryptopro = cache.value("allow_cryptopro", false);
    auto is_allow_mstsc = cache.value("allow_mstsc", false);
    auto is_allow_tasks = cache.value("enable_task", false);

    ui->tabWidget->setTabVisible(0, is_allow_profile_manager);
    ui->tabWidget->setTabVisible(1, is_allow_cryptopro);
    ui->tabWidget->setTabVisible(2, is_allow_mstsc);
    ui->tabWidget->setTabVisible(3, is_allow_tasks);
}

void DialogMain::edit_row(const QModelIndex &current_index)
{
    qDebug() << __FUNCTION__;

    QModelIndex index;

    if(!current_index.isValid())
        index = treeViewMstsc->current_index();
    else
        index = current_index;

    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = treeViewMstsc->get_model();
    auto object = model->to_object(index);
    Q_ASSERT(!object.empty());

    auto mstsc = arcirk::secure_serialization<arcirk::client::mstsc_options>(object, __FUNCTION__);
    auto dlg = DialogMstsc(mstsc, this);
    connect(&dlg, &DialogMstsc::selectHost, this, &DialogMain::doSelectHosts);
    connect(this, &DialogMain::setSelectHosts, &dlg, &DialogMstsc::onSelectHost);
    connect(&dlg, &DialogMstsc::selectDatabaseUser, this, &DialogMain::doSelectDatabaseUser);
    connect(this, &DialogMain::setSelectDatabaseUser, &dlg, &DialogMstsc::onSelectDatabaseUser);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto row = pre::json::to_json(mstsc);
        //auto model = treeViewMstsc->get_model();
        model->set_object(index, row);
    }
}

void DialogMain::reset_data()
{
    qDebug() << __FUNCTION__;

    read_cache(current_user->cache());
    save_cache_to_local();

    current_user->set_database_cache(current_user->cache());

    update_cache_on_server();
    auto detailed_records = mstsc_param.value("detailed_records", json::array());
    update_rdp_files(detailed_records);
    createDynamicMenu();
    form_control();
}

void DialogMain::update_mpl_items_icons()
{
    qDebug() << __FUNCTION__;

    auto model = (ITree<mpl_item>*)treeViewMpl->get_model();
    if(!model)
        return;

    for (int i = 0; i < model->rowCount(QModelIndex()); ++i) {
        auto index = model->index(i, 0, QModelIndex());
        auto object = model->object(index);
        //auto row = secure_serialization<arcirk::client::mpl_item>(object);
        QString link = object.url.c_str();
        if(link.indexOf("markirovka") != -1){
            model->set_row_image(index, QIcon(":/img/markirowka.png"));
        }else if(link.indexOf("diadoc.kontur.ru") != -1){
            model->set_row_image(index, QIcon(":/img/diadoc.png"));
        }else if(link.indexOf("ofd.kontur.ru") != -1){
            model->set_row_image(index, QIcon(":/img/ofd.png"));
        }else if(link.indexOf("extern.kontur.ru") != -1){
            model->set_row_image(index, QIcon(":/img/extern.png"));
        }else if(link.indexOf("sberbank.ru") != -1){
            model->set_row_image(index, QIcon(":/img/sberbank.png"));
        }else
            model->set_row_image(index, QIcon(":/img/link.png"));
    }

    //treeViewMpl->resizeColumnToContents(0);
}

void DialogMain::set_address()
{
    auto action = dynamic_cast<QAction*>(sender());
    ui->txtServer->setText(action->text());
    onTxtServerEditingFinished();

}

void DialogMain::onCertUserCache(const QString &host, const QString &system_user, const nlohmann::json &data)
{
    Q_UNUSED(host);
    Q_UNUSED(system_user);
    //m_cache = data;
    current_user->set_database_cache(data);
    read_cache(current_user->cache());
}

//void DialogMain::onMozillaProfiles(const QString& host, const QString& system_user)
//{

//            auto lst = CertUser::read_mozilla_profiles();

//}

//void DialogMain::doMozillaProfiles(const QStringList &lst)
//{
//    emit setMozillaProfiles(lst);
//}

void DialogMain::onProfilesChanged(const QStringList &lst)
{
    m_moz_profiles = lst;
}

void DialogMain::onCertificates(const arcirk::client::cryptopro_data &data)
{
    if(data.certs.size() > 0){
        QVector<QString> m_order{
            "first",
            "subject",
            "issuer",
            "not_valid_before",
            "not_valid_after",
            "parent_user",
            "sha1",
            "cache",
            "second",
            "suffix"
        };
        auto model = new TreeItemModel(this);
        model->set_hierarchical_list(false);
        auto data_s = arcirk::byte_array_to_string(data.certs);
        json table;
        if(json::accept(data_s))
          table = json::parse(data_s);
        model->set_columns_order(m_order);
        model->set_rows_icon(tree::item_icons_enum::Item,  QIcon(":/img/cert16NoKey.png"));
        model->set_column_aliases(m_colAliases);

        if(!table.is_null())
            model->set_table(table);

        verify_certificate(model);
        treeCertificates->setModel(model);
        treeCertificates->hideColumn(model->column_index("cache"));
        treeCertificates->hideColumn(model->column_index("second"));
        treeCertificates->hideColumn(model->column_index("suffix"));
        treeCertificates->hideColumn(model->column_index("parent"));
        treeCertificates->hideColumn(model->column_index("ref"));
        treeCertificates->hideColumn(model->column_index("sha1"));
        treeCertificates->setSortingEnabled(true);
        treeCertificates->sortByColumn(0, Qt::AscendingOrder);
        //treeCertificates->resizeColumnToContents(0);

    }
}

void DialogMain::onContainers(const arcirk::client::cryptopro_data &data)
{
    if(data.conts.size() > 0){
        QVector<QString> m_order{
            "name",
            "volume",
            "type"
        };
        auto model = new TreeItemModel(this);
        auto data_s = arcirk::byte_array_to_string(data.conts);
        json table;
        if(json::accept(data_s))
            table = json::parse(data_s);
        model->set_hierarchical_list(false);
        model->set_columns_order(m_order);
        model->set_column_aliases(m_colAliases);
        if(!table.is_null())
            model->set_table(table);
        //model->reset();
        treeContainers->setModel(model);

        using namespace arcirk::cryptography;
        int ind = model->column_index("type");
        auto parent = QModelIndex();
        for (int i = 0; i < model->rowCount(parent); ++i) {
            json vol = model->index(i, ind, parent).data(Qt::DisplayRole).toString().trimmed().toStdString();
            auto volume = vol.get<TypeOfStorgare>();
            if(volume == TypeOfStorgare::storgareTypeRegistry)
                model->set_row_image(model->index(i,0,parent), QIcon(":/img/registry16.png"));
            else if(volume == TypeOfStorgare::storgareTypeLocalVolume)
                model->set_row_image(model->index(i,0,parent), QIcon(":/img/desktop.png"));
        }
        treeContainers->hideColumn(model->column_index("parent"));
        treeContainers->hideColumn(model->column_index("ref"));
        //treeContainers->resizeColumnToContents(0);
        treeContainers->setSortingEnabled(true);
        treeContainers->sortByColumn(1, Qt::AscendingOrder);

    }
}

void DialogMain::onSelectHosts(const json &hosts)
{
    //emit setSelectHosts(hosts);
}

void DialogMain::doSelectHosts()
{
    if(!m_client->isConnected())
        return;
    auto types = json::array({arcirk::enum_synonym(arcirk::server::device_types::devDesktop), arcirk::enum_synonym(arcirk::server::device_types::devServer)});
    json query_param = {
        {"table_name", arcirk::enum_synonym(tables::tbDevices)},
        {"query_type", "select"},
        {"values", json{"first", "address", "ref", "deviceType"}},
        {"where_values", json{{"deviceType", types}}}
    };
    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
    auto dev = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                         {"query_param", base64_param}
                                     });
    emit setSelectHosts(dev);
}

void DialogMain::doSelectDatabaseUser()
{
    if(!m_client->isConnected())
        return;

//    auto d_pwd = DialogSelectAuth(this);
//    d_pwd.setModal(true);
//    d_pwd.setWindowTitle("Учетные данные");
//    d_pwd.exec();
//    if(d_pwd.result() != QDialog::Accepted)
//        return;

//    auto auth = d_pwd.get_result();
//    auto conf = arcirk::client::client_conf();
//    conf.user_name = auth["user"];
//    conf.hash = WebSocketClient::generateHash(conf.user_name.c_str(), auth["password"].get<std::string>().c_str()).toStdString();
//    conf.server_host = m_client->conf().server_host;

//    auto model = new TreeItemModel(this);
//    model->set_http_conf(m_client->conf().)
//    model->set_columns({"first","second","ref","parent", "is_group", "deletion_mark"});
//    model->use_hierarchy("first");
//    model->fetchRoot("Users");
//    auto dlg = DialogSelectInTree(model, {"ref", "parent", "is_group", "deletion_mark"}, this);
//    dlg.setModal(true);
//    dlg.exec();
//    if(dlg.result() == QDialog::Accepted){
//        auto sel_object = dlg.selectedObject();
//        emit setSelectDatabaseUser(sel_object);
//    }
}

void DialogMain::onSelectDatabaseUser(const json &user)
{
    //emit setSelectDatabaseUser(user);
}

void DialogMain::onTasksButtonClick()
{
    auto btn = qobject_cast<QToolButton*>(sender());
    Q_ASSERT(btn!=0);

    if(btn->objectName() == "btnTaskAdd"){
        auto task = services::task_options();
        task.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        task.name = "Новая задача 1";

        auto dlg = DialogTask(task, this);
        connect(&dlg, &DialogTask::doInstallPlugin, this, &DialogMain::onBtnInstallBpuginPrivate);
        connect(this, &DialogMain::doEndInstallPlugin, &dlg, &DialogTask::onEndInstallPlugin);
        if(dlg.exec() == QDialog::Accepted){
            auto model = (ITree<services::task_options>*)treeTasks->get_model();
            model->add_struct(task);
            if(!dlg.currentScript().isEmpty()){
                onSavePluginFile(current_user->cert_user_data().ref.c_str(), dlg.currentScript());
            }
        }
    }else if(btn->objectName() == "btnTaskEdit"){
        auto index = treeTasks->current_index();
        if(!index.isValid()) return;
        auto model = (ITree<services::task_options>*)treeTasks->get_model();
        auto task = model->object(index);
        auto dlg = DialogTask(task, this);
        connect(&dlg, &DialogTask::doInstallPlugin, this, &DialogMain::onBtnInstallBpuginPrivate);
        connect(this, &DialogMain::doEndInstallPlugin, &dlg, &DialogTask::onEndInstallPlugin);
        if(dlg.exec() == QDialog::Accepted){
            model->set_struct(task, index);
            if(!dlg.currentScript().isEmpty()){
                onSavePluginFile(current_user->cert_user_data().ref.c_str(), dlg.currentScript());
            }
        }
    }
}

void DialogMain::onTxtServerEditingFinished()
{
    QUrl url(ui->txtServer->text());
    m_client->server_conf().ServerHost = url.host().toStdString();
    m_client->server_conf().ServerPort = url.port();
    m_client->server_conf().ServerSSL = url.scheme() == "wss" ? true : false;

}

void DialogMain::onTxtServerUserEditingFinished()
{
    m_client->client_server_param().user_name = ui->txtServerUser->text().trimmed().toStdString();
}

void DialogMain::onBtnSelectUserClicked()
{
//    if(!users_model_)
//        return;

//    auto dlg = DialogSelectInTree(users_model_, {"ref", "parent", "is_group", "deletion_mark"}, this);
//    dlg.setModal(true);
//    dlg.exec();
//    if(dlg.result() == QDialog::Accepted){
//        auto sel_object = dlg.selectedObject();
//        client_server_param.user_uuid = sel_object["ref"];
//        ui->txtServerUser->setText(sel_object["first"].get<std::string>().c_str());
//        on_txtServerUser_editingFinished();
    //    }
}

void DialogMain::onBtnPwdViewToggled(bool checked)
{
    auto echoMode = checked ? QLineEdit::Normal : QLineEdit::Password;
    QString image = checked ? ":/img/viewPwd.svg" : ":/img/viewPwd1.svg";
    auto btn = dynamic_cast<QToolButton*>(sender());
    btn->setIcon(QIcon(image));
    ui->lblPwd->setEchoMode(echoMode);
}

void DialogMain::onBtnPwdEditToggled(bool checked)
{
    if(checked){
        if(ui->lblPwd->text() == "***"){
            ui->lblPwd->setText("");
        }
    }else{
        if(ui->lblPwd->text() == ""){
            ui->lblPwd->setText("***");
        }
    }

    ui->lblPwd->setEnabled(checked);
    ui->btnPwdView->setEnabled(checked);
    if(!checked){
        auto echoMode = checked ? QLineEdit::Normal : QLineEdit::Password;
        QString image = checked ? ":/img/viewPwd.svg" : ":/img/viewPwd1.svg";
        auto btn = dynamic_cast<QToolButton*>(sender());
        btn->setIcon(QIcon(image));
        ui->lblPwd->setEchoMode(echoMode);
        ui->btnPwdView->setChecked(false);
    }
}


void DialogMain::onRadioRunAsAdminToggled(bool checked)
{
    Q_UNUSED(checked);
    form_control();
}


void DialogMain::onRadioRunAsToggled(bool checked)
{
    Q_UNUSED(checked);
    form_control();
}


void DialogMain::onBtnMstscAddClicked()
{
    auto mstsc = arcirk::client::mstsc_options();
    auto dlg = DialogMstsc(mstsc, this);
    connect(&dlg, &DialogMstsc::selectHost, this, &DialogMain::doSelectHosts);
    connect(this, &DialogMain::setSelectHosts, &dlg, &DialogMstsc::onSelectHost);
    connect(&dlg, &DialogMstsc::selectDatabaseUser, this, &DialogMain::doSelectDatabaseUser);
    connect(this, &DialogMain::setSelectDatabaseUser, &dlg, &DialogMstsc::onSelectDatabaseUser);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto row = pre::json::to_json(mstsc);
        auto model = treeViewMstsc->get_model();
        model->add(row);
    }
}


void DialogMain::onBtnMstscEditClicked()
{
    edit_row();
}


void DialogMain::onTreeViewMstscDoubleClicked(const QModelIndex &index)
{
    edit_row(treeViewMstsc->get_index(index));
}


void DialogMain::onBtnMstscMoveUpClicked()
{
    auto index = treeViewMstsc->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    if(index.row() < 1)
        return;

    auto model = treeViewMstsc->get_model();
    model->move_up(index);
}


void DialogMain::onBtnMstscMoveDownClicked()
{
    auto index = treeViewMstsc->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = treeViewMstsc->get_model();

    if(index.row() >= model->rowCount(index.parent()) - 1)
        return;

    model->move_down(index);
}

void DialogMain::onBtnMplItemAddClicked()
{
    auto row = arcirk::client::mpl_item();
    row.ref = quuid_to_string(QUuid::createUuid()).toStdString();
    m_moz_profiles = current_user->read_mozilla_profiles();
    m_moz_profiles.append("");
    auto dlg = DialogMplItem(row, m_moz_profiles, this);

    connect(&dlg, &DialogMplItem::profilesChanged, this, &DialogMain::onProfilesChanged);
    dlg.setWindowTitle("Новая запись");
    dlg.setModal(true);
    dlg.exec();

    if(dlg.result() == QDialog::Accepted){
        auto model = (TreeItemModel*)treeViewMpl->get_model();
        if(!model){
            model = new TreeItemModel(this);
            model->set_column_aliases(QMap<QString,QString>{
                                          qMakePair("name", "Наименование"),
                                          qMakePair("url", "Ссылка"),
                                          qMakePair("profile", "Профиль")
                                      });
        }
        auto object = pre::json::to_json(row);
        if(model->columnCount(QModelIndex()) == 0){
            auto columns = json::array();
            auto rows = json::array();
            rows += object;
            for (auto it = object.items().begin(); it != object.items().end(); ++it) {
                columns += it.key();
            }
            json table = json::object();
            table["columns"]  = columns;
            table["rows"] = rows;
            model->set_table(table);
            model->set_columns_order(QVector<QString>{"name", "profile", "url"});
        }else
            model->add(object);
    }

    update_mpl_items_icons();
}


void DialogMain::onBtnMplItemEditClicked()
{
    auto model = (TreeItemModel*)treeViewMpl->get_model();
    if(!model)
        return;
    auto index = treeViewMpl->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto row =  secure_serialization<arcirk::client::mpl_item>(model->to_object(index));
    m_moz_profiles = current_user->read_mozilla_profiles();
    m_moz_profiles.append("");
    auto dlg = DialogMplItem(row, m_moz_profiles, this);
//    connect(&dlg, &DialogMplItem::getMozillaProfiles, this, &DialogMain::onMozillaProfiles);
//    connect(this, &DialogMain::setMozillaProfiles, &dlg, &DialogMplItem::mozillaProfiles);
    connect(&dlg, &DialogMplItem::profilesChanged, this, &DialogMain::onProfilesChanged);
    dlg.setWindowTitle(row.name.c_str());
    dlg.setModal(true);
    dlg.exec();

    if(dlg.result() == QDialog::Accepted){
        auto model = (TreeItemModel*)treeViewMpl->model();
        auto object = pre::json::to_json(row);
        model->set_object(index, object);
    }

    update_mpl_items_icons();
}


void DialogMain::onBtnMplItemDeleteClicked()
{
    auto model = (TreeItemModel*)treeViewMpl->get_model();
    if(!model)
        return;
    auto index = treeViewMpl->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto row = secure_serialization<arcirk::client::mpl_item>(model->to_object(index));
    if(QMessageBox::question(this, "Удаление", QString("Удалить настройку '%1'").arg(row.name.c_str())) == QMessageBox::Yes){
        model->remove(index);
    }
}


void DialogMain::onBtnMplItemMoveUpClicked()
{
    auto index = treeViewMpl->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    if(index.row() < 1)
        return;

    auto model = (TreeItemModel*)treeViewMpl->get_model();
    model->move_up(index);
    update_mpl_items_icons();
}


void DialogMain::onBtnMplItemMoveDownClicked()
{
    auto index = treeViewMpl->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = (TreeItemModel*)treeViewMpl->get_model();

    if(index.row() >= model->rowCount(index.parent()) - 1)
        return;

    model->move_down(index);
    update_mpl_items_icons();
}


void DialogMain::onTreeViewMplDoubleClicked(const QModelIndex &index)
{
    onBtnMplItemEditClicked();
}


void DialogMain::onBtnResetCertIlstClicked()
{
    auto tableCerts = current_user->getCertificates(true);
    auto tableConts = current_user->getContainers();
    auto baCerts = arcirk::string_to_byte_array(tableCerts.dump());
    auto baConts = arcirk::string_to_byte_array(tableConts.dump());

    crypt_data.conts = ByteArray(baConts.size());
    std::copy(baConts.begin(), baConts.end(), crypt_data.conts.begin());
    crypt_data.certs = ByteArray(baCerts.size());
    std::copy(baCerts.begin(), baCerts.end(), crypt_data.certs.begin());

    onCertificates(crypt_data);
    onContainers(crypt_data);

    if(m_client->isConnected()){
        write_cache();
    }
}


void DialogMain::onBtnCertInfoClicked()
{
    auto currentTab = ui->tabCrypt->currentIndex();

    if(currentTab == 0){

        auto model = treeCertificates->get_model();
        auto index = treeCertificates->current_index();
        if(!index.isValid()){
            QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
            return;
        }
        auto ind = model->column_index("cache");
        auto ind_first = model->column_index("first");
        if(ind == -1 || ind_first == -1)
            return;
        auto cache = model->index(index.row(), ind, index.parent()).data(Qt::DisplayRole).toString().toStdString();
        auto first = model->index(index.row(), ind_first, index.parent()).data(Qt::DisplayRole).toString();

        if(json::accept(cache)){
            auto cache_ = nlohmann::json::parse(cache);
            auto dlg = DialogInfo(cache_, first, this);
            dlg.setModal(true);
            dlg.exec();
        }
    }else if(currentTab == 1){
        auto model = treeContainers->get_model();
        Q_ASSERT(model != 0);
        if(!model)
            return;

        auto index = treeContainers->current_index();

        if(!index.isValid()){
            QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
            return;
        }
        auto object = model->to_object(index);
        QString vol = object.value("volume", "").c_str();
        vol.append(object.value("name", "").c_str());
        auto info = current_user->get_container_info(vol);
        auto dlg = DialogInfo(info, object.value("name", "").c_str(), this);
        dlg.setModal(true);
        dlg.exec();

    }else if(currentTab == 2){

        auto model = treeAvailableCerts->get_model();
        Q_ASSERT(model != 0);
        if(!model)
            return;

        auto index = treeAvailableCerts->current_index();
        if(!index.isValid()){
            QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
            return;
        }

        auto object = arcirk::secure_serialization<arcirk::database::certificates_view>(model->to_object(index), __FUNCTION__);
        if(!object.cache.empty()){
            auto cert_obj = json::parse(object.cache);
            auto dlg = DialogInfo(cert_obj, object.first.c_str(), this);
            dlg.setModal(true);
            dlg.setWindowTitle(object.first.c_str());
            dlg.exec();
        }
    }
}

void DialogMain::column_aliases()
{
    m_colAliases.insert("uuid", "ID");
    m_colAliases.insert("session_uuid", "ID сессии");
    m_colAliases.insert("name", "Имя");
    m_colAliases.insert("uuid_user", "ID пользователя");
    m_colAliases.insert("user_uuid", "ID пользователя");
    m_colAliases.insert("app_name", "Приложение");
    m_colAliases.insert("user_name", "Имя пользователя");
    m_colAliases.insert("ip_address", "IP адрес");
    m_colAliases.insert("address", "IP адрес");
    m_colAliases.insert("host_name", "Host");
    m_colAliases.insert("Ref", "Ссылка");
    m_colAliases.insert("ref", "Ссылка");
    m_colAliases.insert("FirstField", "Имя");
    m_colAliases.insert("SecondField", "Представление");
    m_colAliases.insert("first", "Имя");
    m_colAliases.insert("second", "Представление");
    m_colAliases.insert("privateKey", "Контейнер");
    m_colAliases.insert("private_key", "Контейнер");
    m_colAliases.insert("_id", "Иднекс");
    m_colAliases.insert("sid", "SID");
    m_colAliases.insert("cache", "cache");
    m_colAliases.insert("subject", "Кому выдан");
    m_colAliases.insert("issuer", "Кем выдан");
    m_colAliases.insert("container", "Контейнер");
    m_colAliases.insert("notValidBefore", "Начало действия");
    m_colAliases.insert("parentUser", "Владелец");
    m_colAliases.insert("notValidAfter", "Окончание дейтствия");
    m_colAliases.insert("serial", "Серийный номер");
    m_colAliases.insert("volume", "Хранилище");
    m_colAliases.insert("cache", "Кэш");
    m_colAliases.insert("role", "Роль");
    m_colAliases.insert("device_id", "ID устройства");
    m_colAliases.insert("ipadrr", "IP адрес");
    m_colAliases.insert("product", "Продукт");
    m_colAliases.insert("typeIp", "Тип адреса");
    m_colAliases.insert("defPort", "Стандартный порт");
    m_colAliases.insert("notFillWindow", "Оконный режим");
    m_colAliases.insert("password", "Пароль");
    m_colAliases.insert("width", "Ширина");
    m_colAliases.insert("height", "Высота");
    m_colAliases.insert("port", "Порт");
    m_colAliases.insert("user", "Пользователь");
    m_colAliases.insert("updateUser", "Обновлять учетку");
    m_colAliases.insert("start_date", "Дата подключения");
    m_colAliases.insert("organization", "Организация");
    m_colAliases.insert("subdivision", "Подразделение");
    m_colAliases.insert("warehouse", "Склад");
    m_colAliases.insert("price", "Тип цен");
    m_colAliases.insert("workplace", "Рабочее место");
    m_colAliases.insert("device_type", "Тип устройства");
    m_colAliases.insert("is_group", "Это группа");
    m_colAliases.insert("size", "Размер");
    m_colAliases.insert("rows_count", "Количество записей");
    m_colAliases.insert("interval", "Интервал");
    m_colAliases.insert("start_task", "Начало выполнения");
    m_colAliases.insert("end_task", "Окончание выполнения");
    m_colAliases.insert("allowed", "Разрешено");
    m_colAliases.insert("predefined", "Предопределенный");
    m_colAliases.insert("days_of_week", "По дням недели");
    m_colAliases.insert("synonum", "Представление");
    m_colAliases.insert("script", "Скрипт");
    m_colAliases.insert("parent_user", "Владелец");
    m_colAliases.insert("not_valid_before", "Начало действия");
    m_colAliases.insert("not_valid_after", "Окончание дейтствия");
    m_colAliases.insert("type", "Тип");
}

void DialogMain::install_certificate(const std::string& sha1, const std::string& cont){
    auto cnt = CryptContainer(this);
    auto cert = CryptCertificate(this);
    if(!cont.empty()){
        //Получаем данные контейнера
        json query_param{
            {"table_name", arcirk::enum_synonym(tables::tbContainers)},
            {"where_values", json{
                    {"ref", cont}
                }}
        };
        json param{
            {"file_name", "cont.bin"},
            {"destantion", QByteArray(query_param.dump().data()).toBase64().toStdString()}
        };

        write_temp_log(__FUNCTION__, QString("cnt exec_http_query_get(\"GetBlob\", param)"));
        auto ba = m_client->exec_http_query_get("GetBlob", param);
        if(ba.isEmpty()){
            displayError("Ошибка", "Ошибка получения данных контейнера с сервера!");
            return;
        }
        auto file = new QTemporaryFile();
        file->setAutoRemove(false);
        if(file->open()){
            file->write(ba);
            file->close();
        }

        write_temp_log(__FUNCTION__, QString("cnt.from_file %1").arg(file->fileName()));
        try {
            cnt.from_file(arcirk::from_utf(file->fileName().toStdString()));
        } catch (const std::exception& e) {
            write_temp_log(__FUNCTION__, QString("cnt.from_file::error: %1").arg(e.what()));
        }
        file->remove();
        delete file;

    }
    if(!sha1.empty()){
        json query_param{
            {"table_name", arcirk::enum_synonym(tables::tbCertificates)},
            {"where_values", json{
                    {"sha1", sha1}
                }}
        };
        json param{
            {"file_name", "cert.bin"},
            {"destantion", QByteArray(query_param.dump().data()).toBase64().toStdString()}
        };

        write_temp_log(__FUNCTION__, QString("cert exec_http_query_get(\"GetBlob\", param)"));
        auto ba = m_client->exec_http_query_get("GetBlob", param);

        if(ba.isEmpty()){
            displayError("Ошибка", "Ошибка получения данных сертификата с сервера!");
            return;
        }

        auto file = new QTemporaryFile();
        file->setAutoRemove(false);
        if(file->open()){
            file->write(ba);
            file->close();
        }

        write_temp_log(__FUNCTION__, QString("cert.from_file %1").arg(file->fileName()));
        try {
            cert.fromFile(file->fileName());
        } catch (const std::exception& e) {
            write_temp_log(__FUNCTION__, QString("cert.from_file::error: %1").arg(e.what()));
        }


        file->remove();
        delete file;
    }

    if(!cnt.isValid() || !cert.isValid()){
        displayError("Ошибка", "Ошибка получения данных с сервера!");
        return;
    }

    auto result = cnt.to_registry(current_user->getInfo().sid.c_str(), cnt.originalName());
    if(!result){
        displayError("Ошибка", "Ошибка установки сертификата!");
        return;
    }

    result = cert.install(QString("\\\\.\\REGISTRY\\") + cnt.originalName(), this);

    m_show_alerts = true;
    if(!result){
        displayError("Ошибка", "Ошибка установки сертификата!");
        return;
    }else
        trayShowMessage("Сертификат успешно установлен!");
    m_show_alerts = false;
}

void DialogMain::verify_certificate(TreeItemModel *model)
{
    for (int itr = 0; itr < model->rowCount(QModelIndex()); ++itr) {
        auto index = model->index(itr, 0, QModelIndex());
        auto object = model->to_object(index);
        auto dt_str = object.value("not_valid_after", "");
        if(!dt_str.empty()){
            auto dt = QDateTime::fromString(dt_str.c_str(), "dd.MM.yyyy  hh:mm:ss");
            if(dt.isValid()){
                if(dt < QDateTime::currentDateTime()){
                    model->setData(index, QColor(Qt::red), Qt::ForegroundRole);
                }
            }else{
                dt = QDateTime::fromString(dt_str.c_str(), "dd.MM.yyyy hh:mm");
                if(dt.isValid()){
                    if(dt < QDateTime::currentDateTime()){
                        model->setData(index, QColor(Qt::red), Qt::ForegroundRole);
                    }
                }
            }
        }
    }
}

void DialogMain::onBtnCertAddClicked()
{
    qDebug() << __FUNCTION__; // << cert.dump().c_str();

    if(current_user->role() != arcirk::database::roles::dbAdministrator){
        auto dlg = DialogSelectAuth(this);
        dlg.setWindowTitle("Учетные данные администратора");
        if(dlg.exec() == QDialog::Accepted){
            auto query_param = dlg.get_result();
            auto res = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::VerifyAdministrator), query_param);
            if(res == WS_RESULT_ERROR){
                displayError("Ошибка", "Не верные имя пользователя или пароль!");
                return;
            }
        }else{
            return;
        }
    }

    auto tab = ui->tabCrypt->currentIndex();
    if(tab == 0){

        using namespace arcirk::server;

        auto model = treeAvailableCerts->get_model();
        Q_ASSERT(model != 0);
        auto dlg = DialogSelectInTree(model, this);
        dlg.set_window_text("Выбор сетритификата");
        dlg.setModal(true);
        dlg.exec();
        write_temp_log(__FUNCTION__, QString("DialogSelectInTree exec"));
        if(dlg.result() == QDialog::Accepted){
            auto obj = dlg.selectedObject();
            auto sha = obj.value("sha1", "");
            auto cnt = obj.value("private_key", "");
            auto dt_str = obj.value("not_valid_after", "");
            auto dt = QDateTime::fromString(dt_str.c_str(), "dd.MM.yyyy  hh:mm:ss");
            write_temp_log(__FUNCTION__, QString("parse %1 %2 %3").arg(sha.c_str(), cnt.c_str(), dt_str.c_str()));
            if(dt.isValid()){
                if(dt < QDateTime::currentDateTime()){
                    if(QMessageBox::question(this, "Истек строк", "Строк выбранного сертификата истек. Продолжить?") == QMessageBox::No)
                        return;
                }
            }else{
                dt = QDateTime::fromString(dt_str.c_str(), "dd.MM.yyyy hh:mm");
                if(dt.isValid()){
                    if(dt < QDateTime::currentDateTime()){
                        if(dt < QDateTime::currentDateTime()){
                            if(QMessageBox::question(this, "Истек строк", "Строк выбранного сертификата истек. Продолжить?") == QMessageBox::No)
                                return;
                        }
                    }
                }
            }
            write_temp_log(__FUNCTION__, QString("install_certificate %1 %2").arg(sha.c_str(), cnt.c_str()));
            install_certificate(sha, cnt);
            onBtnCertAddClicked();
        }

    }
}

void DialogMain::onBtnCertDeleteClicked()
{
    qDebug() << __FUNCTION__; // << cert.dump().c_str();

    if(current_user->role() != arcirk::database::roles::dbAdministrator){
        auto dlg = DialogSelectAuth(this);
        //dlg.setModal(true);
        dlg.setWindowTitle("Учетные данные администратора");
        if(dlg.exec() == QDialog::Accepted){
            auto query_param = dlg.get_result();
            auto res = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::VerifyAdministrator), query_param);
            if(res == WS_RESULT_ERROR){
                displayError("Ошибка", "Не верные имя пользователя или пароль!");
                return;
            }
        }else
            return;
    }
    auto tab = ui->tabCrypt->currentIndex();
    if(tab == 0){
        auto proxy_index = treeCertificates->currentIndex();
        if(!proxy_index.isValid()){
            QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
            return;
        }

        if(QMessageBox::question(this, "Удаление", "Удалить выбранный сертификат?") == QMessageBox::No)
            return;

//        auto sort_model = treeCertificates->get_model();
//        Q_ASSERT(sort_model != 0);
//        auto model = (TreeItemModel*)sort_model->sourceModel();
//        Q_ASSERT(model != 0);
        auto model = treeCertificates->get_model();
        auto index = treeCertificates->current_index(); //model->find(proxy_index.column(), proxy_index.data().toString(),  QModelIndex());
        Q_ASSERT(index.isValid());
        auto object = model->to_object(index);
        auto sha = object.value("sha1", "");

        auto result = delete_certificate(sha);

        if(result){
            auto tableCerts = current_user->getCertificates(true);
            auto baCerts = arcirk::string_to_byte_array(tableCerts.dump());
            crypt_data.certs = ByteArray(baCerts.size());
            std::copy(baCerts.begin(), baCerts.end(), crypt_data.certs.begin());
            onCertificates(crypt_data);
        }
    }else if(tab == 1){
        //auto index = ui->treeContainers->currentIndex();
        auto proxy_index = treeContainers->currentIndex();
        if(!proxy_index.isValid()){
            QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
            return;
        }

        if(QMessageBox::question(this, "Удаление", "Удалить выбранный контейнер?") == QMessageBox::No)
            return;

//        auto sort_model = (TreeSortModel*)ui->treeContainers->model();
//        Q_ASSERT(sort_model != 0);
//        auto model = (TreeItemModel*)sort_model->sourceModel();
        auto model = treeContainers->get_model();
        Q_ASSERT(model != 0);
        //auto index = model->find(proxy_index.column(), proxy_index.data().toString(), QModelIndex());
        auto index = treeContainers->current_index();
        Q_ASSERT(index.isValid());
        auto object = model->to_object(index);
        auto name = object.value("name", "");
        auto volume = object.value("volume", "");

        auto result = delete_container(volume + name);

        if(result){
            auto table = current_user->getContainers();
            auto ba = arcirk::string_to_byte_array(table.dump());
            crypt_data.conts = ByteArray(ba.size());
            std::copy(ba.begin(), ba.end(), crypt_data.conts.begin());
            onContainers(crypt_data);
        }
    }


}


void DialogMain::onBtnMstscRemoveClicked()
{
    auto index = treeViewMstsc->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    if(QMessageBox::question(this, "Удаление", "Удалить выбранную настройку?") == QMessageBox::No)
        return;

    auto model = treeViewMstsc->get_model();
    model->remove(index);
}


void DialogMain::onButtonBoxAccepted()
{
    accept();
}


void DialogMain::onButtonBoxRejected()
{
    //qDebug() << __FUNCTION__;
    QDialog::hide();
}


void DialogMain::onBtnMstscClicked()
{
    qDebug() << __FUNCTION__;;
    auto index = treeViewMstsc->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = treeViewMstsc->get_model();
    auto object = arcirk::secure_serialization<arcirk::client::mstsc_options>(model->to_object(index), __FUNCTION__);

    if(!object.reset_user){
        openMstsc(object.name.c_str());
    }else{
        std::string _pwd = object.password;
        QString address = object.address.c_str();
        int port = object.port;
        bool defPort = object.def_port;
        QString user = object.user_name.c_str();
        if(!defPort){
            address.append(":" + QString::number(port));
        }

        std::string pwd;
        if(!_pwd.empty())
            pwd = arcirk::crypt(_pwd, CRYPT_KEY);

        editMstsc(address, user, pwd.c_str(), object.name.c_str());
    }
}

void DialogMain::editMstsc(const QString& host, const QString& usr, const QString& pwd, const QString& name)
{
    QString command;

    QString address = host;

    command = QString("cmdkey /add:%1 /user:%2 /pass:%3 & ").arg("TERMSRV/" + address, name, pwd);

    QFile f(cache_mstsc_directory() + "/" + name + ".rdp");

    command.append("mstsc /edit \"" + QDir::toNativeSeparators(f.fileName()) + "\" & exit");

    auto cmd = CommandLine(this);
    QEventLoop loop;

    auto started = [&cmd, &command]() -> void
    {
        cmd.send(command, CmdCommand::mstscEditFile);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    auto output = [](const QByteArray& data) -> void
    {
        std::string result_ = arcirk::to_utf(data.toStdString(), "cp866");
    };
    loop.connect(&cmd, &CommandLine::output, output);

    auto err = [&loop, &cmd](const QString& data, int command) -> void
    {
        qDebug() << __FUNCTION__ << data << command;
        cmd.stop();
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::error, err);

    auto state = [&loop]() -> void
    {
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::complete, state);

    cmd.start();
    loop.exec();
}

void DialogMain::openMstsc(const QString &name)
{
    qDebug() << __FUNCTION__;

    auto cmd = CommandLine(this);
    QEventLoop loop;
    QFile f(cache_mstsc_directory() + "/" + name + ".rdp");

    auto started = [&cmd, &f]() -> void
    {
        QString command = QString("mstsc /edit \"%1\" & exit").arg(QDir::toNativeSeparators(f.fileName()));
        cmd.send(command, CmdCommand::COMMAND_INVALID);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    QByteArray cmd_text;
    auto output = [&cmd_text](const QByteArray& data) -> void
    {
        cmd_text.append(data);
    };
    loop.connect(&cmd, &CommandLine::output, output);
    auto err = [&loop, &cmd](const QString& data, int command) -> void
    {
        qDebug() << __FUNCTION__ << data << command;
        cmd.stop();
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::error, err);

    auto state = [&loop]() -> void
    {
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::complete, state);

    cmd.start();
    loop.exec();
}


QIcon DialogMain::get_link_icon(const QString &link)
{
    if(link.indexOf("markirovka") != -1){
        return QIcon(":/img/markirowka.png");
    }else if(link.indexOf("diadoc.kontur.ru") != -1){
        return QIcon(":/img/diadoc.png");
    }else if(link.indexOf("ofd.kontur.ru") != -1){
        return QIcon(":/img/ofd.png");
    }else if(link.indexOf("extern.kontur.ru") != -1){
        return QIcon(":/img/extern.png");
    }else if(link.indexOf("sberbank.ru") != -1){
        return QIcon(":/img/sberbank.png");
    }else
        return QIcon(":/img/link.png");
}

void DialogMain::run_mstsc_link(const arcirk::client::mstsc_options &opt)
{

    QString command;

    QString address = QString(opt.address.c_str()) + ":" + QString::number(opt.port);
    QString pwd;
    if(!opt.password.empty())
        pwd = WebSocketClient::crypt(opt.password.c_str(), CRYPT_KEY).c_str();

    if(opt.reset_user){
        command = QString("cmdkey /add:%1 /user:%2 /pass:%3 & ").arg("TERMSRV/" + address, opt.user_name.c_str(), pwd);
    }

    QFile f(cache_mstsc_directory() + "/" + opt.name.c_str() + ".rdp");

    command.append("mstsc \"" + QDir::toNativeSeparators(f.fileName()) + "\" & exit");

    auto cmd = CommandLine(this);
    QEventLoop loop;

    auto started = [&cmd, &command]() -> void
    {
        cmd.send(command, CmdCommand::csptestContainerFnfo);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    auto output = [](const QByteArray& data) -> void
    {
        std::string result_ = arcirk::to_utf(data.toStdString(), "cp866");
    };
    loop.connect(&cmd, &CommandLine::output, output);

    auto err = [&loop, &cmd](const QString& data, int command) -> void
    {
        qDebug() << __FUNCTION__ << data << command;
        cmd.stop();
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::error, err);

    auto state = [&loop]() -> void
    {
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::complete, state);

    cmd.start();
    loop.exec();
}

void DialogMain::onBtnSelectPathFirefoxClicked()
{

    auto folder = QFileDialog::getExistingDirectory(this, "Выбор каталога", ui->txtFirefoxPath->text());
    if(!folder.isEmpty())
        ui->txtFirefoxPath->setText(folder);

}

void DialogMain::onCheckIP()
{
    QDir dir(mpl_.firefox_path.c_str());
    QString exeFile = "firefox";
#ifdef Q_OS_WINDOWS
    exeFile.append(".exe");
#endif
    QFile exe(dir.path() + "/" + exeFile);
    if(!dir.exists() || !exe.exists()){
        QString file = QFileDialog::getOpenFileName(this, tr("Путь к firefox"),
                                                     QDir::homePath(),
                                                     exeFile);
        if(file != ""){
            QFileInfo fi(file);
            exe.setFileName(file);
            mpl_.firefox_path = QDir::toNativeSeparators(fi.absolutePath()).toStdString();
        }
    }

    if(exe.exists()){
        QStringList args;
        args.append("-URL");
        args.append("https://2ip.ru/");
        mozillaApp->waitForFinished();
        mozillaApp->start(exe.fileName(), args);
    }
}

void DialogMain::onUserInfo(const json &info)
{
    auto info_ = secure_serialization<arcirk::database::user_info>(info);
    json role = info_.role;
    current_user->set_role(role.get<arcirk::database::roles>());
}

void DialogMain::openMozillaFirefox()
{
    QDir dir(mpl_.firefox_path.c_str());
    QString exeFile = "firefox";
#ifdef Q_OS_WINDOWS
    exeFile.append(".exe");
#endif
    QFile exe(dir.path() + "/" + exeFile);
    if(!dir.exists() || !exe.exists()){
        QString file = QFileDialog::getOpenFileName(this, tr("Путь к firefox"),
                                                     QDir::homePath(),
                                                     exeFile);
        if(file != ""){
            QFileInfo fi(file);
            exe.setFileName(file);
            mpl_.firefox_path = QDir::toNativeSeparators(fi.absolutePath()).toStdString();
        }
    }

    if(exe.exists()){
        QStringList args;

        args.append("-P");
        mozillaApp->waitForFinished();
        mozillaApp->start(exe.fileName(), args);
    }
}

void DialogMain::read_available_certificates(){
    using namespace arcirk::database;
    using json = nlohmann::json;
    auto cache = current_user->cache();
    auto table = cache.value("available_certificates", json::object());
    auto model = treeAvailableCerts->get_model();

    QVector<QString> m_order{
        "first",
        "subject",
        "issuer",
        "not_valid_before",
        "not_valid_after",
        "parent_user",
        "private_key"
    };
    if(model == 0){
        model = new TreeItemModel(this);
        model->set_column_aliases(m_colAliases);
        model->set_rows_icon(tree::item_icons_enum::Item, QIcon(":/img/cert16NoKey.png"));
        model->set_hierarchical_list(false);
    }else
        model->clear();

    if(!table.empty()){
        model->set_table(table);
        model->set_columns_order(m_order);
    }else{
        auto a_certs = pre::json::to_json(arcirk::database::certificates_view());
        auto table = json::object();
        auto columns = json::array();
        auto rows = json::array();
        for (auto itr = a_certs.items().begin(); itr != a_certs.items().end(); ++itr) {
            columns += itr.key();
        }
        table["columns"] = columns;
        table["rows"] = rows;
        model->set_table(table);
        model->set_columns_order(m_order);
    }

    verify_certificate(model);

    auto cols = model->columnNames();
    foreach (auto itr, cols) {
        if(m_order.indexOf(itr) == -1){
            auto ind = model->column_index(itr);
            if(ind != -1)
                treeAvailableCerts->hideColumn(ind);
        }
    }

    treeAvailableCerts->setModel(model);
    treeAvailableCerts->setSortingEnabled(true);
    treeAvailableCerts->sortByColumn(0, Qt::AscendingOrder);
}

void DialogMain::write_available_certificates(nlohmann::json &data){

    auto model = treeAvailableCerts->get_model();
    Q_ASSERT(model != 0);
    if(model->columnCount(QModelIndex()) == 0){
       data["available_certificates"] = "";
       return;
    }
    auto table = model->to_table_model(QModelIndex());
    data["available_certificates"] = table;
}

void DialogMain::read_tasks_param()
{
    auto m_cache = current_user->cache();
    auto tasks_param = m_cache.value("tasks_param", json::array());
    QVector<services::task_options> m_tasks;
    if(tasks_param.is_array()){
       for (auto itr = tasks_param.begin(); itr != tasks_param.end(); ++itr) {
            auto p = secure_serialization<services::task_options>(*itr);
            m_tasks.push_back(p);
       }
    }
    auto model = (ITreeTasksModel*)treeTasks->get_model();
    model->clear();
    model->create_controls();
    model->add_items(m_tasks);

    treeTasks->hideColumn(model->column_index("param"));
    treeTasks->hideColumn(model->column_index("parent"));
    treeTasks->hideColumn(model->column_index("ref"));
    treeTasks->hideColumn(model->column_index("script"));
    treeTasks->hideColumn(model->column_index("days_of_week"));
    treeTasks->hideColumn(model->column_index("uuid"));
    treeTasks->hideColumn(model->column_index("name"));
    treeTasks->hideColumn(model->column_index("is_group"));

}

void DialogMain::write_tasks_param(nlohmann::json &data)
{
    auto model = (ITreeTasksModel*)treeTasks->get_model();
    data["tasks_param"] = model->to_array(QModelIndex());
}


void DialogMain::onInstallCertificate()
{
    qDebug() << __FUNCTION__; // << cert.dump().c_str();
    using namespace arcirk::server;

    auto parent_model = treeAvailableCerts->get_model();
    auto model = new TreeItemModel(this);
    model->set_column_aliases(parent_model->columns_aliases());
    model->set_columns_order(parent_model->columns_order());
    model->set_hierarchical_list(false);
    model->set_rows_icon(tree::item_icons_enum::Item, QIcon(":/img/cert16NoKey.png"));
    QList<QString> m_hide;
    for (int i = 0; i < parent_model->columnCount(); ++i) {
       if(parent_model->column_name(i) == "first")
            continue;
       m_hide.append(parent_model->column_name(i));
    }
    model->set_table(parent_model->to_table_model(QModelIndex()));
    auto dlg = DialogSelectInTree(model, m_hide, this);
    dlg.set_window_text("Выбор сетритификата");
    dlg.setModal(true);
    if(dlg.exec()){
        auto obj = dlg.selectedObject();
        auto sha = obj.value("sha1", "");
        auto cnt = obj.value("private_key", "");
        install_certificate(sha, cnt);
        onBtnResetCertIlstClicked();
    }
}

void DialogMain::onTabCryptTabBarClicked(int index)
{
    auto enable = index < 1;
    ui->btnCertAdd->setEnabled(enable);
    ui->btnCertDelete->setEnabled(enable);
    ui->btnResetCertIlst->setEnabled(enable);
}


void DialogMain::onBtnProfileCopyClicked()
{
    auto table = treeViewMpl;
    auto index = table->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = (TreeItemModel*)table->get_model();
    auto object = model->to_object(index);

    model->add(object);

    update_mpl_items_icons();
}



void DialogMain::onBtnMstscCopyClicked()
{
    //auto table = ui->treeViewMstsc;
    auto index = treeViewMstsc->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = treeViewMstsc->get_model();
    auto object = model->to_object(index);
    object["uuid"] = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();

    model->add(object);

}


void DialogMain::onChkWriteLogToggled(bool checked)
{
    m_write_log = checked;
}

json DialogMain::get_local_users_details()
{

    auto cmd = CommandLine(this);
    QString cryptoPro = cryptoProDirectory().path();

    if(!cryptoPro.isEmpty())
        cmd.setWorkingDirectory(cryptoPro);

    QEventLoop loop;

    auto started = [&cmd]() -> void
    {
        QString s = QString("wmic useraccount get name,sid & exit");
        cmd.send(s, CmdCommand::wmicUsers);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    QByteArray cmd_text;
    auto output = [&cmd_text](const QByteArray& data, CmdCommand command) -> void
    {
        cmd_text.append(data);
    };
    loop.connect(&cmd, &CommandLine::output, output);

    auto err = [&loop, &cmd](const QString& data, int command) -> void
    {
        qDebug() << __FUNCTION__ << data << command;
        cmd.stop();
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::error, err);

    auto state = [&loop]() -> void
    {
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::complete, state);


    cmd.start();
    loop.exec();

    std::string result_ = arcirk::to_utf(cmd_text.toStdString(), "cp866");
    //std::string result_ = arcirk::to_utf(cmd_text.toStdString(), "cp1251");
    //qDebug() << qPrintable(result_.c_str());
    auto result = CommandLineParser::parse(result_.c_str(), CmdCommand::wmicUsers);
    //qDebug() << result.dump(4).c_str();
    return result;

}

void DialogMain::create_tasks_model()
{
    auto model = new ITreeTasksModel(this);
    model->set_hierarchical_list(false);
    model->set_column_aliases(QMap<QString,QString>{
        qMakePair("uuid", "Ссылка"),
        qMakePair("name", "Наименование"),
        qMakePair("synonum", "Представление"),
        qMakePair("start_task", "Начало"),
        qMakePair("end_task", "Конец"),
        qMakePair("interval", "Интервал"),
        qMakePair("allowed", "Разрешено"),
        qMakePair("days_of_week", "Выполнять по дням"),
        qMakePair("script", "Скрипт"),
        qMakePair("comment", "Комментарий"),
        qMakePair("predefined", "Предопределенное"),
        qMakePair("script_synonum", "Представление скрипта"),
        qMakePair("type_script", "Тип скрипта"),
    });
    model->set_enable_rows_icons(false);
    model->set_columns_order(QList<QString>{"allowed", "synonum", "start_task", "end_task", "interval"});
    treeTasks->setModel(model);

    connect(ui->btnTaskAdd, &QToolButton::clicked, this, &DialogMain::onTasksButtonClick);
    connect(ui->btnTaskEdit, &QToolButton::clicked, this, &DialogMain::onTasksButtonClick);
    connect(ui->btnTaskDelete, &QToolButton::clicked, this, &DialogMain::onTasksButtonClick);
    connect(treeTasks, &QTreeView::doubleClicked, this, &DialogMain::onTreeTaskDoubleClicked);
    connect(ui->btnTaskStart, &QToolButton::clicked, this, &DialogMain::onBtnTaskStartClicked);
    connect(ui->btnTaskDelete, &QToolButton::clicked, this, &DialogMain::onBtnTaskDeleteClicked);

    auto header = treeTasks->header();
    header->resizeSection(0, 20);
    header->setSectionResizeMode(0, QHeaderView::Fixed);

    treeTasks->hideColumn(model->column_index("reset_version"));
}

void DialogMain::onBtnTaskStartClicked()
{
    auto index = treeTasks->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = (ITreeTasksModel*)treeTasks->get_model();
    auto object = model->object(index);
    if(object.type_script == 1){
        QString plugin_name = object.script_synonum.c_str();
        if(plugin_name.isEmpty()){
            QMessageBox::critical(this, "Ошибка", "Не указан файл плагина!");
            return;
        }
        QString file = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/plugins/" + plugin_name;
        if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/plugins").exists())
            QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/plugins");
        if(!QFile::exists(file)){
            if(object.script.size() == 0){
                QMessageBox::critical(this, "Ошибка", "Плагин не загружен!");
                return;
            } else{
                arcirk::write_file(file.toStdString(), object.script);
            }
        }

        QFile f(file);
        if(!f.exists()){
            QMessageBox::critical(this, "Ошибка", "Файл плагина не найден!");
            return;
        }
        using namespace arcirk::plugins;
        try {
            auto loader = new QPluginLoader(f.fileName(), this);
            QObject *obj = loader->instance();
            IAIPlugin* plugin
                = qobject_cast<IAIPlugin*>(obj);
            if(plugin){
                if(!plugin->accept())
                    QMessageBox::critical(this, "Ошибка", plugin->lastError());
                else
                    QMessageBox::information(this, "Ошибка", "Обработка успешно завершена!");
                loader->unload();
            }
            delete loader;
        } catch (const std::exception& e) {
            qCritical() << e.what();
        }
    }
}


void DialogMain::onBtnTaskDeleteClicked()
{
    auto index = treeTasks->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }
    auto model = (ITreeTasksModel*)treeTasks->get_model();
    if(QMessageBox::question(this, "Удаление элемента", "Удалить выбранный элемент?") == QMessageBox::No)
        return;
    model->remove(index);
}

void DialogMain::onBtnParamClicked()
{
    auto index = treeTasks->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = (ITreeTasksModel*)treeTasks->get_model();
    auto object = model->object(index);
    if(object.type_script == 1){
        QPath p(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        p /= "plugins";
        p /= QString(object.uuid.c_str());
        p /= QString(object.script_synonum.c_str());
//        QString plugin_name = object.script_synonum.c_str();
//        if(plugin_name.isEmpty()){
//            QMessageBox::critical(this, "Ошибка", "Не указан файл плагина!");
//            return;
//        }
//        QString file = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/plugins/" + plugin_name;
//        if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/plugins").exists())
//            QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/plugins");
//        if(!QFile::exists(file)){
//            if(object.script.size() == 0){
//                QMessageBox::critical(this, "Ошибка", "Плагин не загружен!");
//                return;
//            } else{
//                arcirk::write_file(file.toStdString(), object.script);
//            }
//        }

        QFile f(p.path());
        if(!f.exists()){
            QMessageBox::critical(this, "Ошибка", "Файл плагина не найден!");
            return;
        }
        using namespace arcirk::plugins;
        try {
            auto loader = new QPluginLoader(f.fileName(), this);
            QObject *obj = loader->instance();
            IAIPlugin* plugin
                = qobject_cast<IAIPlugin*>(obj);
            if(plugin){
                if(object.param.size() > 0){
                    plugin->setParam(arcirk::byte_array_to_string(object.param).c_str());
                }
                if(plugin->editParam(this)){
                    //qDebug() << plugin->param();
                    auto bt = arcirk::string_to_byte_array(plugin->param().toStdString());
                    object.param = ByteArray(bt.size());
                    std::copy(bt.begin(), bt.end(), object.param.begin());
                    model->set_struct(object, index);
                }
                loader->unload();
            }
            delete loader;
        } catch (const std::exception& e) {
            qCritical() << e.what();
        }
    }

}

void DialogMain::onTreeTaskDoubleClicked(const QModelIndex &index)
{
    auto tree = qobject_cast<TreeViewWidget*>(sender());
    Q_ASSERT(tree!=0);

    if(tree->objectName() == "treeTasks"){
        auto index_ = treeTasks->get_index(index);
        if(!index_.isValid()) return;
        auto model = (ITree<services::task_options>*)treeTasks->get_model();
        auto task = model->object(index_);
        auto dlg = DialogTask(task, this);
        connect(&dlg, &DialogTask::doInstallPlugin, this, &DialogMain::onBtnInstallBpuginPrivate);
        connect(this, &DialogMain::doEndInstallPlugin, &dlg, &DialogTask::onEndInstallPlugin);
        if(dlg.exec() == QDialog::Accepted){
            model->set_struct(task, index_);
            if(!dlg.currentScript().isEmpty()){
                onSavePluginFile(current_user->cert_user_data().ref.c_str(), dlg.currentScript());
            }
        }
    }

}

void DialogMain::onSavePluginFile(const QString &uuidUser, const QString &fileName)
{
    //сохраняем файл на сервер
    QFile f(fileName);
    if(!f.exists()){
        displayError("Ошибка", "Файл плагина не найден!");
        return;
    }
    ByteArray data{};
    arcirk::read_file(fileName.toStdString(), data);
    if(data.size() > 0){
        QString destantion  = QString("html\\client_data\\plugins\\%1").arg(uuidUser);
        QFileInfo inf(fileName);
        json param{
            {"destantion", destantion.toUtf8().toBase64().toStdString()},
            {"file_name", inf.fileName().toStdString()}
        };
        auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::CreateDirectories), param);
        resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::DownloadFile), param, data);
        if(resp == WS_RESULT_ERROR)
            displayError("Ошибка", "Ошибка копирования файла на сервер!");
        else{
            if(resp.is_object()){
                trayShowMessage("Файл успешно загружен на сервер!");
            }
        }
    }

}

void DialogMain::onInstallPlugin(const json &param, const std::string& ref)
{

    infoIco->setVisible(true);
    infoIco->movie()->start();
    infoBar->setText("Загрузка файла с сервера ...");
    auto result = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::UploadFile), param, {}, true);
    if(result != WS_RESULT_ERROR){
         infoBar->setText("Установка ...");
         try {
            ByteArray bt = result["data"].get<ByteArray>();
            if(bt.size() == 0)
                displayError("Ошибка", "Ошибка данных плагина!");
            else{
                QPath p(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
                p /= "plugins";
                p /= QString(ref.c_str());

                QDir dir(p.path());
                if(!dir.exists())
                    dir.mkpath(p.path());

                QFileInfo fs(param["file_name"].get<std::string>().c_str());
                p /= fs.fileName();

                if(p.exists())
                    QFile::remove(p.path());

                arcirk::write_file(p.path().toStdString(), bt);
                trayShowMessage("Плагин успешно установлен!");
                emit doEndInstallPlugin(p.path());
            }
         } catch (const std::exception& e) {
            qCritical() << e.what();
         }
    }else
         displayError("Ошибка", "Ошибка получения файла плагина!");

    infoIco->setVisible(false);
    infoIco->movie()->stop();
    infoBar->setText(QString("Подключен: %1 (%2)").arg(m_client->conf().server_host.c_str(), m_client->server_conf().ServerName.c_str()));
}

void DialogMain::onBtnInstallBpuginPrivate(const json &param, const std::string &ref)
{
    onInstallPlugin(param, ref);
}

void DialogMain::onEndInstallPlugin(const QString &file_name)
{
    emit doEndInstallPlugin(file_name);
}

void DialogMain::onBtnInstallBpugin()
{
    auto model = (ITree<services::task_options>*)treeTasks->get_model();
    auto index = treeTasks->current_index();
    if(!index.isValid()){
         QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
         return;
    }

    auto obj = model->object(index);
    if(obj.type_script !=1){
         QMessageBox::critical(this, "Ошибка", "Не верный тип скрипта!");
         return;
    }

    if(QMessageBox::question(this, "Установка", "Установить плагин текущему пользователю?") == QMessageBox::No)
         return;

    QPath path(QString("html\\client_data\\plugins"));
    path /= QString(current_user->cert_user_data().ref.c_str());
    path /= QString(obj.script_synonum.c_str());

    auto param = json::object({
        {"file_name", path.path().toStdString()}
    });

    onInstallPlugin(param, obj.uuid);
}
