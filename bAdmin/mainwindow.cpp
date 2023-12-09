#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardPaths>
#include <QMessageBox>
#include "query_builder.hpp"
#include "dialoguser.h"
#include "connectiondialog.h"
#include <QVector>
#include <QFileDialog>
#include "dialogabout.h"
#include "dialogseversettings.h"
#include "dialogdevice.h"
#include "dialogselectinlist.h"
#include "dialogtask.h"
#include <QStandardPaths>
#include <QException>
#include <QStorageInfo>
#include <QSysInfo>
#include "dialogselectdevice.h"
#include <QDirIterator>
#include <QTemporaryFile>
#include "dialoginfo.h"
#include "dialogedit.h"
#include "dialogcertusercache.h"
#include <QClipboard>
#include "dialogselectintree.h"
#include "commandline.h"
#include <fmt/core.h>
#include "dialogselect.h"
//#include "tableviewdelegate.h"
#include <QDir>
#include "commandlineparser.h"
#include <boost/random.hpp>
#include "dialogsqltableviewer.h"
#include "dialogtreemodelsettings.h"
#include "dialogquerybuilder.h"
#include <QTreeWidget>
#include <QMovie>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mozillaApp = new QProcess(this);

    treeView = new TreeViewWidget(this);
    ui->verticalLayoutTree->addWidget(treeView);

    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &MainWindow::onTreereeWidgetItemClicked);

    root_item = nullptr;

    connect(ui->mnuConnect, &QAction::triggered, this, &MainWindow::onMnuConnectTriggered);

    this->createTrayActions();
    this->createTrayIcon();
    this->trayIcon->show();

    current_user = new CertUser(this);
    current_user->setLocalhost(true);

    m_client = new WebSocketClient(this);
    connect(m_client, &WebSocketClient::connectionChanged, this, &MainWindow::connectionChanged);
    connect(m_client, &WebSocketClient::connectionSuccess, this, &MainWindow::connectionSuccess);
    connect(m_client, &WebSocketClient::displayError, this, &MainWindow::displayError);
    connect(m_client, &WebSocketClient::serverResponse, this, &MainWindow::serverResponse);
    connect(m_client, &WebSocketClient::commandToClientResponse, this, &MainWindow::onCommandToClient);
    connect(m_client, &WebSocketClient::error, this, &MainWindow::wsError);
    connect(m_client, &WebSocketClient::userMessage, this, &MainWindow::doUserMessage);

    m_client->set_system_user(current_user->user_name());

    formControl();

    if(!m_client->conf().is_auto_connect)
        openConnectionDialog();
    else
        reconnect();

    createColumnAliases();

    fillDefaultTree();

    setWindowTitle(QString("Менеджер сервера (%1)").arg(current_user->getInfo().user.c_str()));

    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeView, &QTreeView::customContextMenuRequested, [&](const QPoint& pos) {

        QModelIndex index = treeView->indexAt(pos);
        QMenu menu;

        menu.addAction(tr("Копировать"), treeView, [&, index]() {
            QApplication::clipboard()->setText( index.data().toString() );
        });
        menu.addSeparator();
        menu.addAction("Очистить", treeView,  [&, index]() {
            auto model = get_model();
            auto index_ = get_index(index);
            if(!index_.isValid())
                return;
            if(model->server_object() == arcirk::server::server_objects::DatabaseTables){
                if(QMessageBox::question(this, "Оичстка", QString("Очистить таблицу %1? \nОперация не обратима!").arg( model->index(index_.row(), 0, QModelIndex()).data().toString())) == QMessageBox::Yes){
                    json query_param = {
                        {"table_name", model->index(index_.row(), 0, QModelIndex()).data().toString().toStdString()},
                        {"query_type", "delete"},
                        {"query_text", QString("DELETE FROM %1").arg(index.data().toString()).toStdString()}
                    };
                    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
                    auto dev = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                                         {"query_param", base64_param}
                                                     });
                }
            }
        });
        menu.addAction("Заполнить демо записями", treeView,  [&, index]() {
            auto model = get_model();
            auto index_ = get_index(index);
            if(!index_.isValid())
                return;
            if(QMessageBox::question(this, "Заполнить", QString("Заполнить выбранную таблицу демо данными?").arg( index.data().toString())) == QMessageBox::No)
                return;
            if(model->server_object() == arcirk::server::server_objects::DatabaseTables){
                json table = model->index(index_.row(), 0, QModelIndex()).data().toString().toStdString();
                load_table_demo(table.get<database::tables>());
            }
        });
        menu.addSeparator();
        menu.addAction("Настройка списка", treeView,  [&]() {
            emit openTreeModelSettingsDialog(get_model());
        });
        menu.addSeparator();
        menu.addAction("Просмотр", treeView,  [&]() {
           auto model = get_model();
           auto index_ = get_index(index);
           if(!index_.isValid())
               return;
           json table = model->index(index_.row(), 0, QModelIndex()).data().toString().toStdString();
           emit openTableViewDialog(table.get<database::tables>());
        });
        menu.exec(treeView->viewport()->mapToGlobal(pos));
    });

    m_messanager = nullptr;

    connect(this, &MainWindow::selectGetLocalUsers, this, &MainWindow::onSelectGetLocalUsers);
    connect(this, &MainWindow::openTreeModelSettingsDialog, this, &MainWindow::onOpenTreeModelSettingsDialog);
    connect(this, &MainWindow::openTableViewDialog, this, &MainWindow::onOpenTableViewDialog);

    m_query_console = new DialogQueryConsole(m_client, this);
    m_query_console->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
//    //test Qt::WindowSystemMenuHint |
//    auto today = arcirk::current_date();
//    auto st_day = arcirk::start_day(today);
//    auto en_day = arcirk::end_day(today);
//    qDebug() << QDateTime::fromSecsSinceEpoch(st_day) << QDateTime::fromSecsSinceEpoch(en_day);


    connect(ui->mnuExit, &QAction::triggered, this, &MainWindow::onMnuExitTriggered);
    connect(ui->mnuDisconnect, &QAction::triggered, this, &MainWindow::onMnuDisconnectTriggered);
    connect(ui->mnuAbout, &QAction::triggered, this, &MainWindow::onMnuAboutTriggered);
    connect(ui->mnuOptions, &QAction::triggered, this, &MainWindow::onMnuOptionsTriggered);
    connect(ui->mnuUpdatePriceChecker, &QAction::triggered, this, &MainWindow::onMnuUpdatePriceCheckerTriggered);
    connect(ui->mnuMessaneger, &QAction::triggered, this, &MainWindow::onMnuMessanegerTriggered);
    connect(ui->mnuMessaneger, &QAction::triggered, this, &MainWindow::onMnuMessanegerTriggered);
    connect(ui->mnuDwnloadAnydesk, &QAction::triggered, this, &MainWindow::onDownloadAnydesk);
    connect(ui->actionmnuQueryConsole, &QAction::triggered, this, &MainWindow::onMnuQueryConsoleTriggered);
    connect(ui->btnEdit, &QToolButton::clicked, this, &MainWindow::onBtnEditClicked);
    connect(ui->btnDataImport, &QToolButton::clicked, this, &MainWindow::onBtnDataImportClicked);
    connect(ui->btnAdd, &QToolButton::clicked, this, &MainWindow::onBtnAddClicked);
    connect(ui->btnDelete, &QToolButton::clicked, this, &MainWindow::onBtnDeleteClicked);
    connect(ui->btnSetLinkDevice, &QToolButton::clicked, this, &MainWindow::onBtnSetLinkDeviceClicked);
    connect(ui->btnTaskRestart, &QToolButton::clicked, this, &MainWindow::onBtnTaskRestartClicked);
    connect(ui->btnStartTask, &QToolButton::clicked, this, &MainWindow::onBtnStartTaskClicked);
    connect(ui->btnSendClientRelease, &QToolButton::clicked, this, &MainWindow::onBtnSendClientReleaseClicked);
    //connect(ui->btn, &QToolButton::clicked, this, &MainWindow::onBtnSendoToClientClicked);
    connect(ui->btnInfo, &QToolButton::clicked, this, &MainWindow::onBtnInfoClicked);
    connect(ui->btnAddGroup, &QToolButton::clicked, this, &MainWindow::onBtnAddGroupClicked);
    connect(ui->btnRegistryDevice, &QToolButton::clicked, this, &MainWindow::onBtnRegistryDeviceClicked);
    connect(ui->btnRegistryUser, &QToolButton::clicked, this, &MainWindow::onBtnRegistryUserClicked);
    connect(ui->btnEditCache, &QToolButton::clicked, this, &MainWindow::onBtnEditCacheClicked);
    connect(ui->btnSystemUsers, &QToolButton::clicked, this, &MainWindow::onBtnSystemUsersClicked);
    connect(ui->btnRefresh, &QToolButton::clicked, this, &MainWindow::onBtnRefreshClicked);
    connect(ui->btnMoveTo, &QToolButton::clicked, this, &MainWindow::onBtnMoveToClicked);


    connect(treeView, &TreeViewWidget::doubleClicked, this, &MainWindow::onTreeViewDoubleClicked);

}

MainWindow::~MainWindow()
{
    if(m_client->isConnected())
        m_client->close();
    delete ui;
}

bool MainWindow::openConnectionDialog()
{
    auto dlg = ConnectionDialog(m_client->conf(), this);
    dlg.setConnectionState(m_client->isConnected());
    connect(this, &MainWindow::setConnectionChanged, &dlg, &ConnectionDialog::connectionChanged);
    connect(&dlg, &ConnectionDialog::closeConnection, this, &MainWindow::closeConnection);
    connect(&dlg, &ConnectionDialog::openConnection, this, &MainWindow::openConnection);

    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        m_client->write_conf();
        reconnect();
    }

    return true;
}

void MainWindow::reconnect()
{
    if(m_client->isConnected())
            m_client->close();

    m_client->open(current_user->getInfo().sid);
    infoBar->setText(QString("Попытка подключения %1 ...").arg(m_client->url().toString()));
    set_enable_form(false);
    ui->toolBar->setEnabled(false);
}

void MainWindow::displayError(const QString &what, const QString &err)
{
    qCritical() << __FUNCTION__ << what << ": " << err ;

    QIcon msgIcon(":/img/h.png");
    trayIcon->showMessage(what, err, msgIcon,
                          3 * 1000);
}

void MainWindow::connectionSuccess()
{
    qDebug() << __FUNCTION__;

    createModels();

    current_user->read_database_cache(m_client->url(), m_client->conf().hash.c_str());

    //get_online_users();

    trayShowMessage(QString("Успешное подключение к севрверу: %1:%2").arg(m_client->server_conf().ServerHost.c_str(), QString::number(m_client->server_conf().ServerPort)));

    createDynamicMenu();

    if(m_messanager == 0){
//        m_messanager = new DialogMessagesList(m_client->conf(), this);
////        connect(m_messanager, &DialogMessagesList::getMessages, this, &MainWindow::doGetMessages);
//        connect(m_messanager, &DialogMessagesList::sendMessage, this, &MainWindow::onSendMessage);
////        connect(this, &MainWindow::readMessages, m_messanager, &DialogMessagesList::onGetMessages);
//        connect(this, &MainWindow::userMessage, m_messanager, &DialogMessagesList::onUserMessage);
//        connect(this, &MainWindow::clientConnectionSuccess, m_messanager, &DialogMessagesList::onConnectionSuccess);
//        connect(this, &MainWindow::clientLeave, m_messanager, &DialogMessagesList::onClientLeave);
//        connect(this, &MainWindow::clientJoin, m_messanager, &DialogMessagesList::onClientJoin);
    }

    emit clientConnectionSuccess(m_client->currentSession(), m_client->currentUserUuid());
    if(root_item !=0)
        emit ui->treeWidget->itemClicked(root_item, 0);
}

void MainWindow::connectionChanged(bool state)
{
    ui->toolBar->setEnabled(true);
    ui->mnuConnect->setEnabled(!state);
    ui->mnuDisconnect->setEnabled(state);
    ui->mnuMessaneger->setEnabled(state);
    ui->mnuOptions->setEnabled(state);

    auto tree = ui->treeWidget;

    if(!state){
        infoBar->setText("Не подключен");
        tree->clear();
        treeView->clear();
        //m_sort_model->setSourceModel(nullptr);
        if(m_messanager != 0){
            if(m_messanager->isVisible())
                m_messanager->close();
            delete m_messanager;
            m_messanager = nullptr;
        }
        root_item = nullptr;
    }else{
        infoBar->setText(QString("Подключен: %1 (%2)").arg(m_client->conf().server_host.c_str(), m_client->server_conf().ServerName.c_str()));
        if(!tree->topLevelItem(0)){
            fillDefaultTree();          
        }
         tree->topLevelItem(0)->setText(0, QString("%1 (%2)").arg(m_client->conf().server_host.c_str(), m_client->server_conf().ServerName.c_str()));

    }

    set_enable_form(state);
}


void MainWindow::formControl()
{
    infoIco = new QLabel(this);
    auto mv = new QMovie(":/img/animation_loaded.gif");
    mv->setScaledSize(QSize(16,16));
    infoIco->setMovie(mv);
    infoIco->setMaximumHeight(ui->statusbar->height());
    infoIco->setVisible(false);
    infoBar = new QLabel(this);
    ui->statusbar->addWidget(infoIco);
    ui->statusbar->addWidget(infoBar);
    infoBar->setText("Готово");

    app_icons.insert(arcirk::server::application_names::PriceChecker, qMakePair(QIcon(":/img/pricechecker.png"), QIcon(":/img/pricechecker.png")));
    app_icons.insert(arcirk::server::application_names::ServerManager, qMakePair(QIcon(":/img/socket.ico"), QIcon(":/img/socket.ico")));
    app_icons.insert(arcirk::server::application_names::ExtendedLib, qMakePair(QIcon(":/img/1cv8.png"), QIcon(":/img/1cv8.png")));
    app_icons.insert(arcirk::server::application_names::ProfileManager, qMakePair(QIcon(":/img/mpl.png"), QIcon(":/img/mpl.png")));

}

void MainWindow::write_conf()
{

}

void MainWindow::openConnection()
{
    if(m_client->isConnected())
            m_client->close();

    m_client->open();
}

void MainWindow::closeConnection()
{
    if(m_client->isConnected())
        m_client->close();
}

void MainWindow::serverResponse(const arcirk::server::server_response &message)
{
    using namespace arcirk::server;

    auto command = json(message.command).get<arcirk::server::server_commands>();


    if(command == server_commands::ServerOnlineClientsList){
        resetModel(arcirk::server::OnlineUsers, get_table(message.result));
    }else if(command == server_commands::ServerConfiguration){
        resetModel(arcirk::server::Root, get_server_conf_table(message.result));
    }else if(command == server_commands::GetDatabaseTables){
        resetModel(arcirk::server::DatabaseTables, get_table(message.result));
    }else if(command == server_commands::UpdateServerConfiguration){
        //
    }else if(command == server_commands::ExecuteSqlQuery){
        if(message.result == WS_RESULT_ERROR){
            qCritical() << __FUNCTION__ << "error result ExecuteSqlQuery" << message.message.c_str();
        }else if(message.result == WS_RESULT_SUCCESS){
            qDebug() << __FUNCTION__ << "success result ExecuteSqlQuery";
            auto param = json::parse(QByteArray::fromBase64(message.param.c_str()));
            if(!param.is_object())
                return;
            auto query_param_base64 = param.value("query_param", "");
            if(query_param_base64.empty())
                return;

            auto query_param = json::parse(QByteArray::fromBase64(query_param_base64.c_str()));
            auto query_type = query_param.value("query_type", "");
            auto table_name = query_param.value("table_name", "");
            if(table_name == arcirk::enum_synonym(arcirk::database::tables::tbCertificates)){
                if(query_type == "update"){
                    trayShowMessage("Обновление сертификата успешно выполнено!");
                    database_get_certificates_asynch();
                }
            }
            else if(table_name == arcirk::enum_synonym(arcirk::database::tables::tbCertUsers)){
                if(query_type == "update"){
                    auto values = query_param.value("values", json::object());
                    if(!values.empty()){
                        auto host = values.value("host", "");
                        auto system_user = values.value("system_user", "");
                        if(!host.empty() && !system_user.empty()){
                            auto index = is_user_online(host.c_str(), system_user.c_str(), enum_synonym(server::application_names::ProfileManager).c_str());
                            if(index.isValid()){
                                auto session_info =arcirk::secure_serialization<client::session_info>( m_models[server::OnlineUsers]->to_object(index), __FUNCTION__);
                                m_client->command_to_client(session_info.session_uuid, "ResetCache", json{});
                            }
                        }
                    }
                }
            }else
                qDebug() << __FUNCTION__ << table_name.c_str() << query_type.c_str();
        }else{
            auto param = nlohmann::json::parse(QByteArray::fromBase64(message.param.data()));
            auto query_param = param.value("query_param", "");
            if(!query_param.empty()){
                auto param_ = nlohmann::json::parse(QByteArray::fromBase64(query_param.data()));
                auto table_name = param_.value("table_name", "");
                if(table_name == "Devices" || table_name == "DevicesView"){
                    //tableResetModel(arcirk::server::Devices, message.result.data());
                    resetModel(arcirk::server::Devices, get_table(message.result));
                }else
                    resetModel(serverObject(table_name), get_table(message.result));
            }

        }
    }else if(command == server_commands::ProfileDeleteFile){
        if(message.message == "OK"){
            QMessageBox::information(this, "Удаление файла", "Файл успешно удален!");
            auto model = get_model();// (TreeItemModel*)ui->treeView->model();
            model->clear();
            //model->fetchRoot("ProfileDirectory");
        }else if(message.message == "error"){
            QMessageBox::critical(this, "Удаление файла", "Ошибка удаения файла!");
        }
    }else if(command == server_commands::GetTasks){
        if(message.message == "OK"){
            //tableResetModel(arcirk::server::Services, message.result.data());
            resetModel(arcirk::server::Services, get_table(message.result));
        }
    }else if(command == server_commands::GetMessages){
        emit readMessages(message);
    }else if(command == server_commands::CMD_INVALID){
        if(message.command == "ClientLeave" || message.command == "ClientJoin"){
            if(message.command == "ClientLeave"){
                auto index = m_models[server_objects::OnlineUsers]->find(m_models[server_objects::OnlineUsers]->column_index("session_uuid"), message.sender.c_str(), QModelIndex());
                if(index.isValid()){
                    emit clientLeave(arcirk::secure_serialization<client::session_info>(m_models[server_objects::OnlineUsers]->to_object(index)));
                }
            }
            //get_online_users();
            get_online_users_synh();
            if(message.command == "ClientJoin"){
                auto index = m_models[server_objects::OnlineUsers]->find(m_models[server_objects::OnlineUsers]->column_index("session_uuid"), message.sender.c_str(), QModelIndex());
                if(index.isValid()){
                    emit clientJoin(arcirk::secure_serialization<client::session_info>(m_models[server_objects::OnlineUsers]->to_object(index)));
                }
            }
        }else if(message.command == "update_certUsers"){
            auto param = nlohmann::json::parse(QByteArray::fromBase64(message.param.data()));
            qDebug() << param.dump().c_str();
            auto sid = param.value("sid", "");
            auto host = param.value("host", "");
            auto system_user = param.value("system_user", "");
            auto model = get_model();//(TreeItemModel*)ui->treeView->model();
            if(model){
                if(model->server_object() == server::server_objects::CertUsers){
                    auto f_index = model->find(model->column_index("sid"), sid.c_str(), QModelIndex());
                    if(f_index.isValid()){
                        json object;
                        is_cert_user_exists(host.c_str(), system_user.c_str(), object);
                        if(object.is_object()){
                            model->set_object(f_index, object);
                        }
                    }
                }
            }
        }
    }else{
        auto param = nlohmann::json::parse(QByteArray::fromBase64(message.param.data()));
        auto query_param = param.value("query_param", "");
        if(!query_param.empty()){
            auto param_ = nlohmann::json::parse(QByteArray::fromBase64(query_param.data()));
            auto table_name = param_.value("table_name", "");
            resetModel(serverObject(table_name), get_table(message.result));
        }
    }


}

void MainWindow::onCertUserCache(const QUrl &ws, const QString &host, const QString &system_user, QWidget *parent)
{
    Q_UNUSED(parent);

    //Процедура использутеся в mpl поэтому статичная http_query

    using json = nlohmann::json;
    try {
        json query_param = {
            {"table_name", arcirk::enum_synonym(tables::tbCertUsers)},
            {"query_type", "select"},
            {"values", json::array({"cache"})},
            {"where_values", json::object({
                 {"host", host.toStdString()},
                 {"system_user", system_user.toStdString()}
             })}
        };
        QString token = m_client->conf().hash.c_str();
        std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
        auto resp = WebSocketClient::http_query(ws, token, arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                             {"query_param", base64_param}
                                         });

        if(resp.is_object()){
            auto rows = resp.value("rows", json::array());
            if(rows.size() == 0){
                emit displayError("Ошибка", "Ошибка получения данных пользователя!");
            }else{
                auto cache = json::parse(rows[0]["cache"].get<std::string>());
                emit certUserData(host, system_user, cache);
            }
        }else
            emit displayError("Ошибка", "Ошибка получения данных пользователя!");
    } catch (...) {
        emit displayError("Ошибка", "Ошибка получения данных пользователя!");
    }

}

void MainWindow::onMozillaProfiles(const QString &host, const QString &system_user)
{
    auto index = is_user_online(host, system_user);
    bool is_online = index.isValid();

    if(!is_online)
        QMessageBox::critical(this, "Ошибка", "Клиент не в сети!");
    else{
        auto model_online_users = m_models[arcirk::server::server_objects::OnlineUsers];
        auto object = model_online_users->to_object(index);
        auto struct_obj = pre::json::from_json<arcirk::client::session_info>(object);
        if(struct_obj.session_uuid == m_client->currentSession().toString(QUuid::WithoutBraces).toStdString()){
            auto lst = CertUser::read_mozilla_profiles();
            lst.insert(0, " ");
            emit mozillaProfiles(lst);
        }else
            m_client->command_to_client(struct_obj.session_uuid.c_str(), "GetMozillaProfilesList");
    }
}

void MainWindow::doGetCertUserCertificates(const QString &host, const QString &system_user)
{
    auto model_online_users = m_models[arcirk::server::server_objects::OnlineUsers];
    bool is_online = false;
    arcirk::client::session_info struct_obj;
    for (auto i = 0; i < model_online_users->rowCount(QModelIndex()) ; ++i) {
        auto object = model_online_users->to_object(model_online_users->index(i,0, QModelIndex()));
        struct_obj = pre::json::from_json<arcirk::client::session_info>(object);
        if(struct_obj.host_name == host.toStdString() && struct_obj.system_user == system_user.toStdString()){
            is_online = true;
            break;
        }
    }

    if(!is_online)
        QMessageBox::critical(this, "Ошибка", "Клиент не в сети!");
    else{
        if(struct_obj.session_uuid == m_client->currentSession().toString(QUuid::WithoutBraces).toStdString()){
            auto table = current_user->getCertificates(true);
            auto ba = arcirk::string_to_byte_array(table.dump());
            auto result = arcirk::client::cryptopro_data();
            result.certs = ByteArray(ba.size());
            std::copy(ba.begin(), ba.end(), result.certs.begin());
            emit certUserCertificates(result);
        }else
            m_client->command_to_client(struct_obj.session_uuid.c_str(), "GetCertificatesList");
    }
}

void MainWindow::doGetCertUserContainers(const QString &host, const QString &system_user)
{
    auto model_online_users = m_models[arcirk::server::server_objects::OnlineUsers];
    bool is_online = false;
    arcirk::client::session_info struct_obj;
    for (auto i = 0; i < model_online_users->rowCount(QModelIndex()) ; ++i) {
        auto object = model_online_users->to_object(model_online_users->index(i,0, QModelIndex()));
        struct_obj = pre::json::from_json<arcirk::client::session_info>(object);
        if(struct_obj.host_name == host.toStdString() && struct_obj.system_user == system_user.toStdString()){
            is_online = true;
            break;
        }
    }

    if(!is_online)
        QMessageBox::critical(this, "Ошибка", "Клиент не в сети!");
    else{
        if(struct_obj.session_uuid == m_client->currentSession().toString(QUuid::WithoutBraces).toStdString()){
            auto table = current_user->getContainers();
            auto ba = arcirk::string_to_byte_array(table.dump());
            auto result = arcirk::client::cryptopro_data();
            result.conts = ByteArray(ba.size());
            std::copy(ba.begin(), ba.end(), result.conts.begin());
            emit certUserContainers(result);
        }else
            m_client->command_to_client(struct_obj.session_uuid.c_str(), "GetContainersList");
    }
}

void MainWindow::doSelectHosts()
{
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
    emit selectHosts(dev);
}

void MainWindow::doSelectDatabaseUser()
{
    using namespace arcirk::server;

    //m_models[DatabaseUsers]->fetchRoot("Users");
    auto dlg = DialogSelectInTree(m_models[DatabaseUsers], {"ref", "parent", "is_group", "deletion_mark"}, this);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto sel_object = dlg.selectedObject();
        emit selectDatabaseUser(sel_object);
    }
}

void MainWindow::onTreeFetch()
{
    //ui->treeView->resizeColumnToContents(0);
}

void MainWindow::onCanFetch(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    //ui->treeView->resizeColumnToContents(0);
}

void MainWindow::wsError(const QString &what, const QString &command, const QString &err)
{
    qCritical() << what << command << err;
}

//void MainWindow::doAvailableCertificates(const QString& user_uuid)
//{
//    json query_param = {
//        {"table_name", arcirk::enum_synonym(tables::tbAvailableCertificates)},
//        {"query_type", "select"},
//        {"values", json{}},
//        {"where_values", json{{"ref", user_uuid.toStdString()}}}
//    };
//    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
//    auto result = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
//                                         {"query_param", base64_param}
//                                     });

//    if(result == WS_RESULT_ERROR){

//    }else{

//    }
//}

void MainWindow::onSelectCertificate()
{
    using namespace arcirk::server;
    database_get_certificates_synch();
    auto model = m_models[server_objects::Certificates];
    auto dlg = DialogSelectInTree(model, this);
    dlg.set_window_text("Выбор сетритификата");
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto obj = dlg.selectedObject();
        //auto c_o = secure_serialization<certificates>(obj);
        emit selectCertificate(obj);
    }

}

void MainWindow::update_rdp_files(const nlohmann::json &items)
{
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

void MainWindow::database_get_containers_synch()
{
    using namespace arcirk::server;
    auto values_ = pre::json::to_json(containers());
    values_.erase("data");
    auto values = arcirk::json_keys(values_);
    auto query_param = nlohmann::json{
        {"table_name", arcirk::enum_synonym(Containers)},
        {"where_values", nlohmann::json{}},
        {"query_type", "select"},
        {"values", json{
                "first",
                "ref",
                "parent_user",
                "subject",
                "issuer",
                "not_valid_before",
                "not_valid_after",
                "cache"
            }
        }
    };
    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
    auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                         {"query_param", base64_param}
                                     });
    if(resp  != WS_RESULT_ERROR){
        if(resp.is_object()){
            modelSetTable(arcirk::server::server_objects::Containers, resp);
        }
    }
}

void MainWindow::onMnuConnectTriggered()
{
    openConnectionDialog();
}


void MainWindow::onMnuExitTriggered()
{
    QApplication::exit();
}


void MainWindow::onMnuDisconnectTriggered()
{
    if(m_client->isConnected())
            m_client->close();
}

QTreeWidgetItem *MainWindow::addTreeNode(const QString &text, const QVariant &key, const QString &imagePath)
{
    //qDebug() << __FUNCTION__;
    auto * node = new QTreeWidgetItem();
    node-> setText (0, text);
    node->setData(0, Qt::UserRole, key);
    node->setIcon(0, QIcon(imagePath));
    return node;
}

QTreeWidgetItem *MainWindow::findTreeItem(const QString &key)
{
    auto tree = ui->treeWidget;
    auto root = tree->topLevelItem(0);
    return findTreeItem(key, root);
}

QTreeWidgetItem * MainWindow::findTreeItem(const QString& key, QTreeWidgetItem* parent)
{
    int i;

    if (!parent)
        return nullptr;

    if(parent->childCount()>0){
        for(i=0;i<parent->childCount();i++){
            if(parent->child(i)!=0)
            {
                if (parent->child(i)->data(0, Qt::UserRole).toString() == key)
                    return parent->child(i);
                else{
                    auto ch = findTreeItem(key, parent->child(i));
                    if(ch)
                        return ch;
                }
            }

        }
    }

    return nullptr;
}

void MainWindow::tableSetModel(const QString &key)
{
    try {
        nlohmann::json m_key = key.toStdString();
        auto e_key = m_key.get<arcirk::server::server_objects>();
        auto model = m_models[e_key];
        Q_ASSERT(model!=0);
        if(m_hierarhy_objects.indexOf(e_key) == -1)
            m_models[e_key]->set_hierarchical_list(false);
        treeView->setModel(model);
        if(e_key == arcirk::server::server_objects::CertUsers ||
            e_key == arcirk::server::server_objects::DatabaseUsers){
            auto i = model->column_index("is_group");
            if(i != -1)
                sortByColumn(treeView, i, Qt::SortOrder::DescendingOrder);
        }
        else
            sortByColumn();

//        if(model)
//            if(ui->treeView->model()->columnCount() > 0)
//                ui->treeView->resizeColumnToContents(0);
//        qDebug() << ui->treeView->indentation();
    } catch (const QException& e) {
        qCritical() << e.what();
    }


}

void MainWindow::tableResetModel(server::server_objects key, const QByteArray& resp)
{
    if(!m_client->isConnected())
        return;

    using namespace arcirk::server;

    if(key == OnlineUsers){

        if(!resp.isEmpty() && resp != WS_RESULT_ERROR){
            auto str = QByteArray::fromBase64(resp);
            modelSetTable(key, json::parse(str));
        }
    }else if (key == Root){
        m_models[Root]->clear();
        auto srv_conf = nlohmann::json::parse(QByteArray::fromBase64(resp).toStdString());
        if(srv_conf.is_object()){
            auto items = srv_conf.items();
            auto columns = nlohmann::json::array();
            columns += "Параметр";
            columns += "Значение";
            auto rows = nlohmann::json::array();
            for (auto itr = items.begin(); itr != items.end(); ++itr) {
                auto row = nlohmann::json::object();
                row["Параметр"] = itr.key();
                row["Значение"] = itr.value();
                rows += row;
            }
            auto object = nlohmann::json::object();
            object["columns"] = columns;
            object["rows"] = rows;
            modelSetTable(key, object);
//            m_models[Root]->reset_columns(columns);
//            m_models[Root]->set_columns_order(to_string_list(columns));
//            m_models[Root]->set_table(object);
//            m_models[Root]->reset();
        }
    }else if (key == DatabaseTables){
        if(!resp.isEmpty() && resp != WS_RESULT_ERROR){
            auto str = QByteArray::fromBase64(resp);
            modelSetTable(key, json::parse(str));
        }
//        auto tbls = nlohmann::json::parse(QByteArray::fromBase64(resp).toStdString());
//        if(tbls.is_object()){
//            m_models[DatabaseTables]->reset_columns(tbls.value("columns", json::array()));
//            m_models[DatabaseTables]->set_columns_order(to_string_list(tbls.value("columns", json::array())));
//            m_models[DatabaseTables]->set_table(tbls);
//            //m_models[DatabaseTables]->reset();
//        }
    }else{
        m_models[key]->clear();
        auto tbls = QByteArray::fromBase64(resp);
        auto table = nlohmann::json::parse(tbls);
        m_models[key]->reset_columns(table["columns"]);
        if(m_order_columns.find(key) != m_order_columns.end()){
            m_models[key]->set_columns_order(m_order_columns[key]);
        }
        if(m_hierarhy_objects.indexOf(key) == -1)
            m_models[key]->set_hierarchical_list(false);
        m_models[key]->set_table(table);
        update_icons(key, m_models[key]);
        //m_models[key]->reset();
    }

    if(key == server_objects::Certificates || key == server_objects::LocalhostUserCertificates)
        verify_certificate(m_models[key]);

    //m_models[key]->reset();

//    auto model = get_model();//ui->treeView->model();
//    if(model)
//        if(ui->treeView->model()->columnCount() > 0)
//            ui->treeView->resizeColumnToContents(0);
}

void MainWindow::modelSetTable(server::server_objects key, const nlohmann::json &data)
{
    if(!m_client->isConnected())
        return;

    using namespace arcirk::server;
//    m_models[key]->set_hierarchical_list(false);
    m_models[key]->clear();
    if(m_order_columns.find(key) != m_order_columns.end()){
        m_models[key]->set_columns_order(m_order_columns[key]);
    }else
        m_models[key]->set_columns_order(to_string_list(data["columns"]));

    if(m_hierarhy_objects.indexOf(key) == -1)
        m_models[key]->set_hierarchical_list(false);

    m_models[key]->set_table(data);
    update_icons(key, m_models[key]);
    //m_models[key]->reset();

    if(key == server_objects::Certificates || key == server_objects::LocalhostUserCertificates)
        verify_certificate(m_models[key]);

    if(key == server_objects::LocalhostUserContainers){
        auto ind = m_models[server::server_objects::LocalhostUserContainers]->column_index("type");
        if(ind != -1){
            sortByColumn(treeView, ind);
        }
    }else{
        sortByColumn();
    }

    m_models[key]->reset();
    //ui->treeView->resizeColumnToContents(0);
}

void MainWindow::resetModel(server::server_objects key, const json &table)
{
    if(!m_client->isConnected())
        return;

    using namespace arcirk::server;

    auto model = m_models[key];
    if(model == 0)
        return;

    model->clear();

    if(m_hierarhy_objects.indexOf(key) == -1)
        model->set_hierarchical_list(false);

    auto columns = table["columns"];

    model->reset_columns(columns);

    if(m_order_columns.find(key) != m_order_columns.end()){
        model->set_columns_order(m_order_columns[key]);
    }else
        model->set_columns_order(to_string_list(columns));

    if(key == server_objects::Certificates || key == server_objects::LocalhostUserCertificates)
        verify_certificate(model);

    model->set_table(table);

    resetUI(key);
}

void MainWindow::resetUI(server::server_objects key)
{
    try {
        auto model = m_models[key];
        Q_ASSERT(model!=0);

        update_icons(key, model);

        treeView->setModel(model);

        update_columns();
        update_toolbar(key);

        if(m_hierarhy_objects.indexOf(key) == -1)
           sortByColumn();
        else{
           auto i = model->column_index("is_group");
           if(i != -1)
                sortByColumn(treeView, i, Qt::SortOrder::DescendingOrder);
        }

        //setIndentation(key);

        treeView->setSortingEnabled(true);

        model->reset();


    } catch (const QException& e) {
        qCritical() << e.what();
    }
}

void MainWindow::createTrayActions()
{
    qDebug() << __FUNCTION__;
    quitAction = new QAction(tr("&Выйти"), this);
    connect(quitAction, &QAction::triggered, this, &MainWindow::onAppExit);
    showAction = new QAction(tr("&Открыть менеджер сервера"), this);
    showAction->setIcon(QIcon(":/img/certificate.ico"));
    connect(showAction, &QAction::triggered, this, &MainWindow::onWindowShow);

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(showAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

//    trayIcon = new QSystemTrayIcon(this);
//    trayIcon->setContextMenu(trayIconMenu);

//    QIcon icon = QIcon(":/img/certificate.ico");
//    trayIcon->setIcon(icon);
//    setWindowIcon(icon);

//    trayIcon->setToolTip("Менеджер сервера");

//    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::trayMessageClicked);
//    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);

//    trayIcon->show();

    //createDynamicMenu();
}

void MainWindow::createTrayIcon()
{
    qDebug() << __FUNCTION__;
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    QIcon icon = QIcon(":/img/certificate.ico");
    trayIcon->setIcon(icon);
    setWindowIcon(icon);

    trayIcon->setToolTip("Менеджер сервера");

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::trayMessageClicked);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);

}

void MainWindow::createDynamicMenu()
{
    qDebug() << __FUNCTION__;
    trayIconMenu->clear();
    trayIconMenu->addAction(showAction);

    auto cache = current_user->cache();
    auto mstsc_param = cache.value("mstsc_param", json::object());
    auto mpl_ = arcirk::internal_structure<arcirk::client::mpl_options>("mpl_options", cache);
    use_firefox = mpl_.use_firefox;
    firefox_path = mpl_.firefox_path;

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
                 connect(action, &QAction::triggered, this, &MainWindow::onTrayTriggered);
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
                auto mstsc = pre::json::from_json<arcirk::client::mpl_item>(object);
                QString name = mstsc.name.c_str();
                auto action = new QAction(name, this);
                action->setProperty("data", object.dump().c_str());
                action->setProperty("type", "link");
                action->setIcon(get_link_icon(mstsc.url.c_str()));
                trayIconMenu->addAction(action);
                connect(action, &QAction::triggered, this, &MainWindow::onTrayTriggered);
            }
        }
    }

    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
}

void MainWindow::fillDefaultTree()
{
    qDebug() << __FUNCTION__;
    auto tree = ui->treeWidget;
    tree->setHeaderHidden(true);

    using namespace arcirk;

    auto root = addTreeNode("root", arcirk::enum_synonym(server::server_objects::Root).c_str(), ":/img/root.png");
    tree->addTopLevelItem(root);
    auto item = addTreeNode("Активные пользователи", arcirk::enum_synonym(server::server_objects::OnlineUsers).c_str(), ":/img/activeUesers16.png");
    root->addChild(item);
    item = addTreeNode("Зарегистрированные пользователи", arcirk::enum_synonym(server::server_objects::CertUsers).c_str(), ":/img/certUsers.png");
    root->addChild(item);
    item = addTreeNode("Службы", arcirk::enum_synonym(server::server_objects::Services).c_str(), ":/img/services.png");
    root->addChild(item);
    item = addTreeNode("База данных", arcirk::enum_synonym(server::server_objects::Database).c_str(), ":/img/sqlServer.png");
    root->addChild(item);
    auto tbl = addTreeNode("Таблицы", arcirk::enum_synonym(server::server_objects::DatabaseTables).c_str(), ":/img/socket_16_only.ico");
    item->addChild(tbl);
    tbl = addTreeNode("Устройства", arcirk::enum_synonym(server::server_objects::Devices).c_str(), ":/img/SqlDivaces.png");
    item->addChild(tbl);
    tbl = addTreeNode("Пользователи", arcirk::enum_synonym(server::server_objects::DatabaseUsers).c_str(), ":/img/users1.png");
    item->addChild(tbl);
    tbl = addTreeNode("Каталог профиля", arcirk::enum_synonym(server::server_objects::ProfileDirectory).c_str(), ":/img/users1.png");
    root->addChild(tbl);
    tbl = addTreeNode("Менеджер сертификатов", arcirk::enum_synonym(server::server_objects::CertManager).c_str(), ":/img/cert.png");
    root->addChild(tbl);
    item = addTreeNode("Контейнеры", arcirk::enum_synonym(server::server_objects::Containers).c_str(), ":/img/cont.png");
    tbl->addChild(item);
    item = addTreeNode("Сертификаты", arcirk::enum_synonym(server::server_objects::Certificates).c_str(), ":/img/certs16.png");
    tbl->addChild(item);
    item = addTreeNode("Текущий пользователь", arcirk::enum_synonym(server::server_objects::LocalhostUser).c_str(), ":/img/userOptions.ico");
    tbl->addChild(item);
    auto item_l = addTreeNode("Сертификаты", arcirk::enum_synonym(server::server_objects::LocalhostUserCertificates).c_str(), ":/img/certs16.png");
    item->addChild(item_l);
    item_l = addTreeNode("Контейнеры", arcirk::enum_synonym(server::server_objects::LocalhostUserContainers).c_str(), ":/img/certUsers.png");
    item->addChild(item_l);
//    auto item_u = addTreeNode("Реестр", arcirk::enum_synonym(server::server_objects::LocalhostUserContainersRegistry).c_str(), ":/img/registry16.png");
//    item_l->addChild(item_u);
//    item_u = addTreeNode("Устройства", arcirk::enum_synonym(server::server_objects::LocalhostUserContainersVolume).c_str(), ":/img/Card_Reader_16.ico");
//    item_l->addChild(item_u);

    root_item = root;
}

void MainWindow::database_get_deviceview_asynch(){
    using namespace arcirk::server;
    if(m_client->isConnected()){
        auto query_param = nlohmann::json{
            {"table_name", "DevicesView"},
            {"where_values", nlohmann::json{}},
            {"query_type", "select"}
        };
        m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                                   {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                               });
    }
}

std::string MainWindow::database_set_certificate(CryptCertificate& cert, const QString& file_name)
{
    using namespace arcirk::server;

    if(cert.isValid()){
        ByteArray ba = cert.getData().data;
        if(ba.size() > 0){
            //сначала запись добавим
            auto struct_certs = arcirk::database::certificates();
            struct_certs._id = 0;
            struct_certs.suffix = cert.getData().suffix;
            struct_certs.deletion_mark = 0;
            struct_certs.is_group = 0;
            struct_certs.first = cert.sinonym();
            struct_certs.not_valid_after = cert.getData().not_valid_after;
            struct_certs.not_valid_before = cert.getData().not_valid_before;
            struct_certs.serial = cert.getData().serial;
            struct_certs.version = 0;
            struct_certs.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
            struct_certs.parent = arcirk::uuids::nil_string_uuid();
            struct_certs.parent_user = cert.parent();
            struct_certs.issuer = cert.issuer_briefly();
            struct_certs.subject = cert.subject_briefly();
            struct_certs.cache = cert.getData().cache;
            struct_certs.sha1 = cert.sha1();

            nlohmann::json query_param = {
                {"table_name", arcirk::enum_synonym(tables::tbCertificates)},
                {"query_type", "insert"},
                {"values", pre::json::to_json(struct_certs)}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::ExecuteSqlQuery), json{
                                                 {"query_param", base64_param}
                                             });

            if(resp  != WS_RESULT_ERROR){
                //обновим данные
                query_param = {
                    {"table_name", arcirk::enum_synonym(tables::tbCertificates)},
                    {"where_values", json{
                         {"ref", struct_certs.ref}
                     }}
                };
                base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
                json param{
                    {"destantion", base64_param},
                    {"file_name", file_name.toStdString()}
                };
                resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::DownloadFile), param, ba);
                if(resp  != "error"){
                    return WS_RESULT_SUCCESS;
                }else {
                    return WS_RESULT_ERROR;
                }
//                if(resp  != "error"){
//                    auto query_param = nlohmann::json{
//                        {"table_name", "Certificates"},
//                        {"where_values", nlohmann::json{}},
//                        {"query_type", "select"},
//                        {"empty_column", false},
//                        {"line_number", false},
//                        {"values", json{
//                                "first",
//                                "ref",
//                                "private_key",
//                                "subject",
//                                "issuer",
//                                "not_valid_before",
//                                "not_valid_after",
//                                "parent_user",
//                                "serial",
//                                "sha1",
//                                "cache"
//                            }
//                        }
//                    };
//                    m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
//                                               {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
//                                           });
//                }
            }
        }
    }else{
        displayError("Ошибка", "Ошибка установки сертификата!");
    }

    return WS_RESULT_ERROR;
}

void MainWindow::database_insert_certificate()
{
    using namespace arcirk::server;

    auto result = QFileDialog::getOpenFileName(this, "Выбрать файл на диске", "", tr("Файлы сетрификатов (*.cer *.crt)"));
    if(!result.isEmpty()){
        //ByteArray data{};
        try {
            auto cert = CryptCertificate(this);
            cert.fromFile(result);
            auto res = database_set_certificate(cert, result);
            if(res == WS_RESULT_SUCCESS){
                database_get_certificates_asynch();
            }

//            if(cert.isValid()){
//                ByteArray ba = cert.getData().data;
//                if(ba.size() > 0){
//                    //сначала запись добавим
//                    auto struct_certs = arcirk::database::certificates();
//                    struct_certs._id = 0;
//                    struct_certs.suffix = cert.getData().suffix;
//                    struct_certs.deletion_mark = 0;
//                    struct_certs.is_group = 0;
//                    struct_certs.first = cert.sinonym();
//                    struct_certs.not_valid_after = cert.getData().not_valid_after;
//                    struct_certs.not_valid_before = cert.getData().not_valid_before;
//                    struct_certs.serial = cert.getData().serial;
//                    struct_certs.version = 0;
//                    struct_certs.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
//                    struct_certs.parent = arcirk::uuids::nil_string_uuid();
//                    struct_certs.parent_user = cert.parent();
//                    struct_certs.issuer = cert.issuer_briefly();
//                    struct_certs.subject = cert.subject_briefly();
//                    struct_certs.cache = cert.getData().cache;
//                    struct_certs.sha1 = cert.sha1();

//                    nlohmann::json query_param = {
//                        {"table_name", arcirk::enum_synonym(tables::tbCertificates)},
//                        {"query_type", "insert"},
//                        {"values", pre::json::to_json(struct_certs)}
//                    };
//                    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
//                    auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::ExecuteSqlQuery), json{
//                                                         {"query_param", base64_param}
//                                                     });

//                    if(resp  != "error"){
//                        //обновим данные
//                        query_param = {
//                            {"table_name", arcirk::enum_synonym(tables::tbCertificates)},
//                            {"where_values", json{
//                                 {"ref", struct_certs.ref}
//                             }}
//                        };
//                        base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
//                        json param{
//                            {"destantion", base64_param},
//                            {"file_name", result.toStdString()}
//                        };
//                        resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::DownloadFile), param, ba);
//                        if(resp  != "error"){
//                            auto query_param = nlohmann::json{
//                                {"table_name", "Certificates"},
//                                {"where_values", nlohmann::json{}},
//                                {"query_type", "select"},
//                                {"empty_column", false},
//                                {"line_number", false},
//                                {"values", json{
//                                        "first",
//                                        "ref",
//                                        "private_key",
//                                        "subject",
//                                        "issuer",
//                                        "not_valid_before",
//                                        "not_valid_after",
//                                        "parent_user",
//                                        "serial",
//                                        "sha1",
//                                        "cache"
//                                    }
//                                }
//                            };
//                            m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
//                                                       {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
//                                                   });
//                        }
//                    }
//                }
//            }else{
//                displayError("Ошибка", "Ошибка загрузки файла!");
//            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Ошибка", e.what());
            return;
        }

    }
}

void MainWindow::localhost_cert_to_database(const QString &sha1)
{

    auto cert = CryptCertificate(this);
    cert.fromLocal(sha1);
    if(!cert.isValid()){
        displayError("Ошибка", "Ошибка чтения данных сертификата!");
        return;
    }
    try {
        auto res = database_set_certificate(cert, cert.sinonym().c_str());
        if(res == WS_RESULT_SUCCESS){
            trayShowMessage("Сертификат успешно импортирован!");
            database_get_certificates_asynch();
        }
    } catch (std::exception& e) {
        displayError("Error", e.what());
    }

}


void MainWindow::set_enable_form(bool value)
{
    treeView->setEnabled(value);
    ui->treeWidget->setEnabled(value);
    ui->widgetToolBar->setEnabled(value);
}

void MainWindow::database_get_certificates_asynch(){
    using namespace arcirk::server;
    if(m_client->isConnected()){
        auto query_param = nlohmann::json{
            {"table_name", arcirk::enum_synonym(Certificates)},
            {"where_values", nlohmann::json{}},
            {"query_type", "select"},
            {"values", json{
                    "first",
                    "ref",
                    "private_key",
                    "subject",
                    "issuer",
                    "not_valid_before",
                    "not_valid_after",
                    "parent_user",
                    "serial",
                    "sha1",
                    "suffix",
                    "cache"
                }
            }
        };
        m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                                   {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                               });
    }
}

void MainWindow::database_get_certificates_synch()
{
    using namespace arcirk::server;
    auto query_param = nlohmann::json{
        {"table_name", arcirk::enum_synonym(Certificates)},
        {"where_values", nlohmann::json{}},
        {"query_type", "select"},
        {"values", json{
                "first",
                "ref",
                "private_key",
                "subject",
                "issuer",
                "not_valid_before",
                "not_valid_after",
                "parent_user",
                "serial",
                "sha1",
                "suffix",
                "cache"
            }
        }
    };

    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
    auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                         {"query_param", base64_param}
                                     });
    if(resp  != WS_RESULT_ERROR){
        if(resp.is_object()){
            modelSetTable(arcirk::server::server_objects::Certificates, resp);
        }
    }
}

void MainWindow::database_get_containers_asynch(){
    using namespace arcirk::server;
    if(m_client->isConnected()){
        auto values_ = pre::json::to_json(containers());
        values_.erase("data");
        auto values = arcirk::json_keys(values_);
        auto query_param = nlohmann::json{
            {"table_name", arcirk::enum_synonym(Containers)},
            {"where_values", nlohmann::json{}},
            {"query_type", "select"},
            {"values", json{
                    "first",
                    "ref",
                    "parent_user",
                    "subject",
                    "issuer",
                    "not_valid_before",
                    "not_valid_after",
                    "cache"
                }
            }
        };
        m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                                   {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                               });
    }
}
void MainWindow::createModels()
{
    using namespace arcirk::server;
    QVector<server_objects> vec{
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
        CertUsers,
        Certificates,
        LocalhostUser,
        LocalhostUserCertificates,
        LocalhostUserContainers
    };
//    LocalhostUserContainersRegistry,
//    LocalhostUserContainersVolume
    m_models.clear();

    foreach (auto const& itr, vec) {
        TreeItemModel * model = nullptr;
        if(itr == OnlineUsers){
            model = new ITree<arcirk::client::session_info>(this);
        }else if(itr == Devices){
            model = new ITree<arcirk::database::devices_view>(this);
        }else if(itr == CertUsers){
            model = new ITree<arcirk::database::cert_users>(this);
            model->set_fetch_expand(true);

            model->set_connection(root_tree_conf::typeConnection::httpConnection,
                                  http_conf(m_client->http_url().toString().toStdString(), m_client->conf().hash, "CertUsers"));
        }else if(itr == DatabaseUsers){
            model = new ITree<arcirk::database::user_info>(this);
            model->set_fetch_expand(true);

            model->set_connection(root_tree_conf::typeConnection::httpConnection,
                                  http_conf(m_client->http_url().toString().toStdString(), m_client->conf().hash, "Users"));
        }else if(itr == Root){
            model = new TreeItemModel(json{{"Параметр", ""}, {"Значение", ""}}, this);
        }else if(itr == ProfileDirectory){
            model = new ITree<arcirk::tree_model::profile_directory>(this);
            model->set_fetch_expand(true);

            model->set_connection(root_tree_conf::typeConnection::serverDirectorias,
                                  http_conf(m_client->http_url().toString().toStdString(), m_client->conf().hash, "ProfileDirectory"));
        }else
            model = new TreeItemModel(this);
            //model = new TreeItemModel(m_client->conf(), this);
        model->set_column_aliases(m_colAliases);
        model->set_server_object(itr);

        if(itr == DatabaseUsers){
            model->set_columns_order({"first","second","ref","parent", "is_group", "deletion_mark"});
            //model->use_hierarchy("first");
            model->set_hierarchical_list(true);
        }else if(itr == DatabaseTables){
            model->set_rows_icon(tree::item_icons_enum::Item, QIcon(":/img/externalDataTable.png"));
            //model->set_hierarchical_list(false);
        }else if(itr == CertUsers){
            model->set_columns_order({"first","host","system_user"});
            //model->use_hierarchy("first");
            model->set_hierarchical_list(true);
        }else if(itr == Services){
            model->set_rows_icon(tree::item_icons_enum::Item, QIcon(":/img/task-process-2.svg"));
        }else if(itr == ProfileDirectory){
            model->set_columns_order({"name","size"});
            model->set_hierarchical_list(true);
        }else
            model->set_hierarchical_list(false);
        //connect(model, &TreeItemModel::fetch, this, &MainWindow::onTreeFetch);
        //connect(model, &TreeItemModel::canFetch, this, &MainWindow::onCanFetch);
        m_models.insert(itr, model);
    }
}

void MainWindow::createColumnAliases()
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
    m_colAliases.insert("host", "Хост");
    m_colAliases.insert("system_user", "Пользователь ОС");
    m_colAliases.insert("info_base", "Информационная база");
}

void MainWindow::onTreereeWidgetItemClicked(QTreeWidgetItem *item, int column)
{
    if(!item)
        return;

    using namespace arcirk::server;

    QString key_str = item->data(0, Qt::UserRole).toString();
    qDebug() << __FUNCTION__ << "tree key:" << key_str;

    json k = key_str.toStdString();
    auto key = k.get<server_objects>();

    if(key == server_objects::OnlineUsers){
        get_online_users();
    }else if(key == server_objects::Root){
        m_client->send_command(arcirk::server::server_commands::ServerConfiguration, nlohmann::json{});
    }else if(key == server_objects::DatabaseTables){
        m_client->send_command(arcirk::server::server_commands::GetDatabaseTables, nlohmann::json{});
    }else if(key == server_objects::Devices){
        database_get_deviceview_asynch();
    }else if(key == server_objects::DatabaseUsers){
        resetUI(key);
    }else if(key == server_objects::ProfileDirectory){
        resetUI(key);
    }else if(key == server_objects::Services){
        m_client->send_command(arcirk::server::server_commands::GetTasks, nlohmann::json{});
    }else if(key == server_objects::Certificates){
        database_get_certificates_asynch();
    }else if(key == server_objects::CertUsers){
        resetUI(key);
    }else if(key == server_objects::Containers){
        database_get_containers_asynch();
    }else if(key == server_objects::LocalhostUserCertificates){
        auto certs = current_user->getCertificates(true);
        resetModel(key, certs);
    }else if(key == server_objects::LocalhostUserContainers){
        auto conts = current_user->getContainers();
        resetModel(key, conts);
    }

    if(key != server_objects::OBJ_INVALID &&
        key != server_objects::Database &&
        key != server_objects::CertManager &&
        key != server_objects::LocalhostUser){
        update_columns();
    }else{
        treeView->setModel(nullptr);
    }

    update_toolbar(key);
}

void MainWindow::get_online_users() {

    qDebug() << __FUNCTION__;

    if(!m_client->isConnected())
        return;

    using json_nl = nlohmann::json;

    json_nl param = {
            {"table", true},
            {"uuid_form", NIL_STRING_UUID},
            {"empty_column", false}
    };

    m_client->send_command(arcirk::server::server_commands::ServerOnlineClientsList, param);

}

void MainWindow::get_online_users_synh()
{

    nlohmann::json query_param = {
        {"table", true},
        {"uuid_form", NIL_STRING_UUID},
        {"empty_column", false}
    };

    auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ServerOnlineClientsList), query_param);
    if(resp  != WS_RESULT_ERROR){
        if(resp.is_object()){
            modelSetTable(arcirk::server::OnlineUsers, resp);
        }
    }

}

void MainWindow::update_icons(arcirk::server::server_objects key, TreeItemModel *model)
{
    using namespace arcirk::server;
    using json = nlohmann::json;

    if(key == Devices){
//        int ind = model->column_index("device_type");
//        int indRef = model->column_index("ref");
//        int indDev = m_models[OnlineUsers]->column_index("device_id");
//        auto parent = QModelIndex();
//        if(ind == -1)
//            return;
        auto model_ = (ITree<arcirk::database::devices_view>*)model;

        for (int i = 0; i < model->rowCount(); ++i) {
            auto index = model->index(i, 0, QModelIndex());
//            json device_type = model->index(i, ind, parent).data(Qt::DisplayRole).toString().trimmed().toStdString();
//            QString ref = model->index(i, indRef, parent).data(Qt::DisplayRole).toString().trimmed();
            auto object = model_->object(index);
            auto indexRef = m_models[OnlineUsers]->find(to_qt_uuid(object.ref));
            auto type = json(object.device_type).get<device_types>();

            if(type == devDesktop){
                if(!indexRef.isValid())
                    model_->set_row_image(index, QIcon(":/img/desktop.png"));
                else
                    model_->set_row_image(index, QIcon(":/img/desktopOnline.png"));
            }else if(type == devServer){
                if(!indexRef.isValid())
                    model_->set_row_image(index, QIcon(":/img/server.png"));
                else
                    model_->set_row_image(index, QIcon(":/img/serverOnline.png"));
            }else if(type == devPhone){
                if(!indexRef.isValid())
                    model_->set_row_image(index, QIcon(":/img/mobile.png"));
                else
                    model_->set_row_image(index, QIcon(":/img/mobileOnline.png"));
            }else if(type == devTablet){
                if(!indexRef.isValid())
                    model_->set_row_image(index, QIcon(":/img/tablet.png"));
                else
                    model_->set_row_image(index, QIcon(":/img/tabletOnline.png"));
            }
        }

    }else if(key == OnlineUsers){
        using namespace arcirk::server;

        //int ind = model->column_index("app_name");
        //auto parent = QModelIndex();
        auto model_ = (ITree<arcirk::client::session_info>*)model;
//        if(ind == -1)
//            return;
        for (int i = 0; i < model->rowCount(); ++i) {
            auto index = model->index(i, 0, QModelIndex());
            auto object = model_->object(index);
            //json app = model->index(i, ind, parent).data(Qt::DisplayRole).toString().trimmed().toStdString();
            auto type_app = json(object.app_name).get<application_names>();
            if(type_app == PriceChecker){
                model_->set_row_image(index, app_icons[PriceChecker].first);
            }else if(type_app == ServerManager){
                model_->set_row_image(index, app_icons[ServerManager].first);
            }else if(type_app == ExtendedLib){
                model_->set_row_image(index, app_icons[ExtendedLib].first);
            }else if(type_app == ProfileManager){
                model_->set_row_image(index, app_icons[ProfileManager].first);
            }
        }
    }else if(key == LocalhostUserCertificates || key == Certificates){
        model->set_rows_icon(tree::item_icons_enum::Item, QIcon(":/img/cert16NoKey.png"));
    }else if(key == Containers){
        model->set_rows_icon(tree::item_icons_enum::Item, QIcon(":/img/cert16Key.png"));
    }else if(key == arcirk::server::LocalhostUserContainers){
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
    }

    qDebug() << key;

    model->reset();
}

void MainWindow::insert_container(CryptContainer &cnt)
{

    Q_ASSERT(cnt.isValid());
    auto org_name = cnt.originalName();
    auto volume = cnt.get_volume();
    auto info = cnt.info(volume + org_name);
    auto cnt_info_ = arcirk::database::containers();

    cnt_info_.cache = info.dump();
    cnt_info_._id = 0;
    cnt_info_.deletion_mark = 0;
    cnt_info_.is_group = 0;
    cnt_info_.first = org_name.toStdString();
    cnt_info_.not_valid_after = info["Not valid after"];
    cnt_info_.not_valid_before = info["Not valid before"];
    cnt_info_.version = 0;
    cnt_info_.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    cnt_info_.parent = arcirk::uuids::nil_string_uuid();
    cnt_info_.parent_user = info["ParentUser"];
    cnt_info_.issuer = info["Issuer"];
    cnt_info_.subject = info["Subject"];

    nlohmann::json query_param = {
        {"table_name", arcirk::enum_synonym(tables::tbContainers)},
        {"query_type", "insert"},
        {"values", pre::json::to_json(cnt_info_)}
    };
    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
    auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                         {"query_param", base64_param}
                                     });
    if(resp  != WS_RESULT_ERROR){
        //обновим данные
        query_param = {
            {"table_name", arcirk::enum_synonym(tables::tbContainers)},
            {"where_values", json{
                 {"ref", cnt_info_.ref}
             }}
        };
        base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
        json param{
            {"destantion", base64_param},
            {"file_name", org_name.toStdString() + ".bin"}
        };
        auto ba = cnt.to_byate_array();
        resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::DownloadFile), param, ba);
        if(resp  != WS_RESULT_ERROR){
            auto query_param = nlohmann::json{
                {"table_name", arcirk::enum_synonym(tables::tbContainers)},
                {"where_values", nlohmann::json{}},
                {"query_type", "select"},
                {"empty_column", false},
                {"line_number", false},
                {"values", json{
                        "first",
                        "ref",
                        "parent_user",
                        "subject",
                        "issuer",
                        "not_valid_before",
                        "not_valid_after",
                        "cache"
                    }
                }
            };
            m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                                       {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                                   });
        }
    }
}

void MainWindow::update_columns()
{
    auto model = get_model();
    if(model == 0)
        return;
    auto table = treeView;

    for (int i = 0; i < model->columnCount(QModelIndex()); ++i) {
        table->setColumnHidden(i, false);
    }

    QVector<QString> m_hide_std{"is_group", "deletion_mark", "version", "_id", "cache", "deletion_mark", "parent"};
    foreach (auto const& itr, m_hide_std) {
        auto index = model->column_index(itr);
        if(index != -1)
            table->hideColumn(index);
    }
    if(model->server_object() == arcirk::server::server_objects::CertUsers){
        QVector<QString> m_hide{"ref", "sid", "uuid", "second"};
        foreach (auto const& itr, m_hide) {
            auto index = model->column_index(itr);
            if(index != -1)
                table->hideColumn(index);
        }
    }else if(model->server_object() == arcirk::server::server_objects::ProfileDirectory){
        QVector<QString> m_hide{"path", "ref", "parent", "is_group"};
        foreach (auto const& itr, m_hide) {
            auto index = model->column_index(itr);
            if(index != -1)
                table->hideColumn(index);
        }
        //ui->treeView->resizeColumnToContents(0);
    }else if(model->server_object() == arcirk::server::server_objects::DatabaseUsers){
        QVector<QString> m_hide{"ref"};
        foreach (auto const& itr, m_hide) {
            auto index = model->column_index(itr);
            if(index != -1)
                table->hideColumn(index);
        }
    }else{
        QList<QString> m_lst{"ref", "data"};

        foreach (auto it, m_lst) {
            auto ind = model->column_index(it);
            if(ind != -1)
                table->hideColumn(ind);
        }
    }

    //table->resizeColumnToContents(0);
}

void MainWindow::edit_cert_user(const QModelIndex &index)
{

    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = get_model();
    using namespace arcirk::database;
    using json = nlohmann::json;
    using namespace arcirk::server;


    auto object = model->to_object(index);
    auto struct_users = pre::json::from_json<arcirk::database::cert_users>(object);
    if(struct_users.sid.empty()){
        if(struct_users.host == current_user->host().toStdString() && struct_users.system_user == current_user->user_name().toStdString()){
            struct_users.sid = current_user->getInfo().sid;
        }
    }
    auto parent = index.parent();
    QString parentName;
    if(parent.isValid()){
        parentName = parent.data().toString();
    }

    auto types = json::array({enum_synonym(device_types::devDesktop), enum_synonym(device_types::devServer)});
    json query_param = {
        {"table_name", arcirk::enum_synonym(tables::tbDevices)},
        {"query_type", "select"},
        {"values", json{"first", "ref"}},
        {"where_values", json{{"deviceType", types}}}
    };
    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
    auto dev = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                         {"query_param", base64_param}
                                     });
    auto devices = dev.value("rows", json::array());

    //m_models[DatabaseUsers]->fe("Users");
    auto dlg = DialogEditCertUser(struct_users, parentName, m_models[arcirk::server::server_objects::DatabaseUsers], devices, this);
    if(!struct_users.uuid.empty()){
        json query_param = {
            {"table_name", arcirk::enum_synonym(tables::tbUsers)},
            {"query_type", "select"},
            {"values", json{"first"}},
            {"where_values", json{{"ref", struct_users.uuid}}}
        };
        std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
        auto us = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                             {"query_param", base64_param}
                                         });
        auto arr = us.value("rows", json::array());
        if(arr.size() > 0){
            std::string us_name = arr[0].value("first", "");
            dlg.set_1c_parent(us_name.c_str());
        }
    }

    connect(&dlg, &DialogEditCertUser::selectRemoteUser, this, &MainWindow::onSelectRemoteUser);
    connect(this, &MainWindow::selectRemoteSystemUser, &dlg, &DialogEditCertUser::onSelectRemoteUser);

    if(dlg.exec() == QDialog::Accepted){
        nlohmann::json query_param = {
            {"table_name", arcirk::enum_synonym(tables::tbCertUsers)},
            {"query_type", "update"},
            {"values", pre::json::to_json(struct_users)},
            {"where_values", nlohmann::json({
                 {"ref", struct_users.ref}
             })}
        };

        std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
        auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::ExecuteSqlQuery), json{
                                             {"query_param", base64_param}
                                         });
        if(resp.is_string()){
            if(resp.get<std::string>() == WS_RESULT_SUCCESS){
                model->set_object(index ,pre::json::to_json(struct_users));
                update_icons(server_objects::CertUsers, model);
            }
        }
    }
}


QModelIndex MainWindow::findInTable(QAbstractItemModel * model, const QString &value, int column, bool findData)
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

void MainWindow::onBtnEditClicked()
{
    auto proxy_index = treeView->currentIndex();

    if(!proxy_index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto index = get_index(proxy_index);
    auto model = get_model();
    if(model->server_object() == arcirk::server::CertUsers){
        edit_cert_user(index);
    }else if(model->server_object() == arcirk::server::Devices
             || model->server_object() == arcirk::server::OnlineUsers){
        emit treeView->doubleClicked(index);
    }else if(model->server_object() == arcirk::server::DatabaseTables){
        json table = model->index(index.row(), 0, QModelIndex()).data().toString().toStdString();
        auto dlg = DialogSqlTableViewer(table.get<database::tables>(), m_client, this);
        dlg.exec();
    }

}


void MainWindow::onTreeViewDoubleClicked(const QModelIndex &index_)
{
    auto index = get_index(index_);
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = get_model();
    using namespace arcirk::database;
    using json = nlohmann::json;
    using namespace arcirk::server;

    try {
        if(model->server_object() == arcirk::server::DatabaseUsers){
            auto object = model->to_object(index);
            if(object["is_group"].get<int>() == 1){
                return;
            }

            auto query = builder::query_builder();

            std::string query_text = query.select().from("Users").where(nlohmann::json{
                                                                               {"ref", object["ref"].get<std::string>()}
                                                                           }, true).prepare();

            auto param = nlohmann::json::object();
            param["query_text"] = query_text;
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);
            auto result = m_client->exec_http_query(command, param);

            if(!result.is_object()){
                QMessageBox::critical(this, "Ошибка", "Ошибка на свервере!");
                return;
            }

            auto rows = result["rows"];
            if(!rows.is_array()){
                QMessageBox::critical(this, "Ошибка", "Ошибка на свервере!");
                return;
            }

            if(rows.size() == 0){
                QMessageBox::critical(this, "Ошибка", "Запись не найдена!");
                return;
            }

            auto struct_users = pre::json::from_json<arcirk::database::user_info>(rows[0]);
            auto parent = index.parent();
            QString parentName;
            if(parent.isValid()){
                parentName = parent.data().toString();
            }
            auto dlg = DialogUser(struct_users, parentName, this);
            dlg.setModal(true);
            dlg.exec();

            if(dlg.result() == QDialog::Accepted){
                nlohmann::json query_param = {
                    {"table_name", arcirk::enum_synonym(tables::tbUsers)},
                    {"query_type", "update"},
                    {"values", pre::json::to_json(struct_users)},
                    {"where_values", nlohmann::json({
                         {"ref", struct_users.ref}
                     })}
                };

                std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
                auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::ExecuteSqlQuery), json{
                                                     {"query_param", base64_param}
                                                 });
                if(resp.is_string()){
                    if(resp.get<std::string>() == WS_RESULT_SUCCESS){
                        model->set_object(index ,pre::json::to_json(struct_users));
                        update_icons(server_objects::DatabaseUsers, model);
                    }
                }
            }

        }else if(model->server_object() == arcirk::server::Devices){

            auto ref = model->data(model->index(index.row(), model->column_index("ref"), index.parent()), Qt::DisplayRole).toString();
            json param{
                {"device", ref.toStdString()}
            };

            auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::DeviceGetFullInfo), param);
            if(resp.is_object()){
                auto dlg = DialogDevice(resp, this);
                dlg.setModal(true);
                dlg.exec();

                if(dlg.result() == QDialog::Accepted){
                    auto dev = dlg.get_result();
                    nlohmann::json query_param = {
                        {"table_name", "Devices"},
                        {"query_type", "update"},
                        {"values", pre::json::to_json(dev)},
                        {"where_values", nlohmann::json({
                             {"ref", ref.toStdString()}
                         })}
                    };

                    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
                    resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::ExecuteSqlQuery), json{
                                                         {"query_param", base64_param}
                                                     });
                    if(resp.is_string()){
                        if(resp.get<std::string>() == WS_RESULT_SUCCESS){
                            model->set_object(index ,pre::json::to_json(dlg.get_view_result()));
                            update_icons(server_objects::Devices, model);
                        }else if(resp.get<std::string>() == WS_RESULT_ERROR){
                            displayError("Ошибка", "Ошибка изменения записи!");
                        }
                    }
                    update_icons(arcirk::server::Devices, model);
                }
            }
        }else if(model->server_object() == arcirk::server::Services){
            auto obj = arcirk::secure_serialization<arcirk::services::task_options>(model->to_object(index));
            auto dlg = DialogTask(obj, this);
            dlg.setModal(true);
            dlg.exec();
            if(dlg.result() == QDialog::Accepted){
                model->set_object(index, pre::json::to_json(obj));
                auto arr_service = model->to_array(index.parent());
                json param{};
                param["task_options"] = arr_service;
                m_client->send_command(server_commands::UpdateTaskOptions, param);
            }
        }else if(model->server_object() == arcirk::server::CertUsers){
            edit_cert_user(index);
        }else if(model->server_object() == arcirk::server::OnlineUsers){

            auto object = model->to_object(index);
            auto dlg = DialogInfo(object, object["app_name"].get<std::string>().c_str(), this);
            dlg.setModal(true);
            dlg.exec();

        }else if(model->server_object() == arcirk::server::server_objects::Certificates ||
                 model->server_object() == arcirk::server::server_objects::Containers ||
                 model->server_object() == arcirk::server::server_objects::LocalhostUserCertificates ||
                 model->server_object() == arcirk::server::server_objects::LocalhostUserContainers){
            onBtnInfoClicked();
        }
    } catch (const std::exception& e) {
        qCritical() << __FUNCTION__ << e.what();
    }

}


void MainWindow::onBtnDataImportClicked()
{
    auto model = get_model();
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    using namespace arcirk::server;

    if(model->server_object() == arcirk::server::DatabaseTables){
        auto model_ = new TreeItemModel(this);
        model_->set_column_aliases(m_colAliases);
        model_->set_server_object(arcirk::server::ProfileDirectory);
        //model_->fetchRoot("ProfileDirectory", "shared_files/files");
        delete model_;
    }else if(model->server_object() == arcirk::server::LocalhostUserCertificates){
        auto object = model->to_object(index);
        auto sha1 = object.value("sha1", "");
        auto name = object.value("first", "");
        if(!name.empty() && !sha1.empty()){
            if(QMessageBox::question(this, "Импорт", QString("Импортировать сертификат %1 в базу данных").arg(name.c_str())) == QMessageBox::No)
                return;
            localhost_cert_to_database(sha1.c_str());
        }
    }
}


void MainWindow::onBtnAddClicked()
{
    auto model = get_model();

    using namespace arcirk::server;
    using json = nlohmann::json;

    if(model->server_object() == arcirk::server::ProfileDirectory){
        auto index = treeView->current_index();
        if(!index.isValid()){
            QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
            return;
        }
        auto result = QFileDialog::getOpenFileName(this, "Выбрать файл на диске");
        if(!result.isEmpty()){
            ByteArray data{};
            try {
                arcirk::read_file(result.toStdString(), data);
                if(data.size() > 0){
//                    auto destantion = model->current_parent_path();
//                    QUrl url(result);
//                    auto file_name = url.fileName();
//                    json param{
//                        {"destantion", destantion.toUtf8().toBase64().toStdString()},
//                        {"file_name", file_name.toStdString()}//,
//                        //{"data", data}
//                    };
//                    auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::DownloadFile), param, data);
//                    if(resp.is_object()){
//                        int is_folder = model->index(index.row(), model->column_index("is_group"), index.parent()).data().toInt();
//                        if(is_folder == 0)
//                            model->add(resp,  index.parent());
//                        else
//                            model->add(resp, index);
//                    }
                }

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Ошибка", e.what());
                return;
            }

        }
    }else if(model->server_object() == arcirk::server::server_objects::Certificates){
        database_insert_certificate();
    }else if(model->server_object() == arcirk::server::server_objects::Containers){

        using namespace arcirk::cryptography;

        QStringList select = {
            "Добавить из каталога",
            "Добавить из устройства"
        };

        auto dlg = DialogSelectDevice(this, select, "Добавить контейнер");
        dlg.setModal(true);
        dlg.exec();

        if(dlg.result() != QDialog::Accepted){
            return;
        }

        int selection = dlg.currentSelection();

        if(selection == 0){

            QString dir = QFileDialog::getExistingDirectory(this, tr("Выбрать каталог"),
                                                         QDir::homePath(),
                                                         QFileDialog::ShowDirsOnly
                                                         | QFileDialog::DontResolveSymlinks);
            if(dir != ""){
//                QFile file(dir + QDir::separator() + "name.key");
//                if(file.open(QIODevice::ReadOnly)){
//                    QString data = QString::fromLocal8Bit(file.readAll());
//                    int ind = data.indexOf("\026");
//                    if(ind == -1){
//                        QMessageBox::critical(this, "Ошибка", "В выбранном каталоге не найдены данные контейнера!");
//                        delete cnt;
//                        return;
//                    }
//                }else{
//                    QMessageBox::critical(this, "Ошибка", "В выбранном каталоге не найдены данные контейнера!");
//                    delete cnt;
//                    return;
//                }

//                container = dir;
//                cnt->fromContainerName(dir);
            }
        }else if(selection == 2){

        }else{
            auto model_loc_cnt = m_models[arcirk::server::LocalhostUserContainers];
            if(model_loc_cnt->rowCount(QModelIndex()) == 0){
                auto model_json = current_user->getContainers();
                modelSetTable(arcirk::server::LocalhostUserContainers, model_json);
            }

            auto dlgSel = new DialogSelectInList(model_loc_cnt, "Выбор устройства", this);
            dlgSel->setModal(true);
            dlgSel->exec();
            if(dlgSel->result() == QDialog::Accepted){
                QStringList dlgResult = dlgSel->dialogResult();
                auto m_type = json(dlgResult[2].toStdString()).get<TypeOfStorgare>();
                if(m_type == TypeOfStorgare::storgareTypeLocalVolume){
                    auto vol = CryptContainer::get_local_volume(dlgResult[1]);
                    Q_ASSERT(!vol.isEmpty());
                    auto index = dlgResult[0].indexOf("@");
                    Q_ASSERT(index!=-1);
                    auto code = dlgResult[0].left(index);
                    QDirIterator it(vol, {code + ".*"}, QDir::Dirs);
                    //Ищем все каталоги с именем начинающимся с code
                    QString dir{};
                    auto cnt = CryptContainer(this);
                    cnt.set_volume(dlgResult[1]);
                    while (it.hasNext()) {
                        dir = it.next();
                        //qDebug() << dir;
                        //получаем имя контейнера из файла name.key
                        auto org_name = CryptContainer::get_local_original_name(dir);
                        if(org_name.isEmpty()){
                            dir = "";
                            continue;
                        }else{
                            if(org_name == dlgResult[0]){
                                cnt.from_dir(dir);
                                insert_container(cnt);
                            }
                        }
                    }
                }else if(m_type == TypeOfStorgare::storgareTypeRegistry){
                    if(current_user->getInfo().sid.empty())
                        return;
                    auto cnt = CryptContainer(this);
                    cnt.set_volume(dlgResult[1]);
                    cnt.from_registry(current_user->getInfo().sid.c_str(), dlgResult[0]);
                    insert_container(cnt);
                }

            }
        }

    }else if(model->server_object() == arcirk::server::server_objects::CertUsers){
        auto index = treeView->current_index();

        if(index.isValid()){
            auto parent_object = model->to_object(index);
            if(parent_object["is_group"] == 0){
                auto parent = index.parent();                
                if(!parent.isValid())
                {
                    QMessageBox::critical(this, "Ошибка", "Выберете группу!");
                    return;
                }
                parent_object = model->to_object(parent);
            }

            auto parent_struct = pre::json::from_json<arcirk::database::cert_users>(parent_object);
            add_cert_user(parent_struct);
        }else
            QMessageBox::critical(this, "Ошибка", "Выберете группу!");
    }else if(model->server_object() == arcirk::server::server_objects::Devices){
        auto object = arcirk::database::table_default_json(arcirk::database::tbDevices);
        auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::DeviceGetFullInfo), json{});
        resp["device"] = object;
        auto dlg = DialogDevice(resp, this);
        dlg.setModal(true);
        dlg.exec();
        if(dlg.result() == QDialog::Accepted){
            auto dev = dlg.get_result();
            nlohmann::json query_param = {
                {"table_name", "Devices"},
                {"query_type", "insert"},
                {"values", pre::json::to_json(dev)}
            };

            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::ExecuteSqlQuery), json{
                                                 {"query_param", base64_param}
                                             });
            if(resp.is_string()){
                if(resp.get<std::string>() == WS_RESULT_SUCCESS){
                    model->add(pre::json::to_json(dlg.get_view_result()));
                    update_icons(server_objects::Devices, model);
                }
            }
            update_icons(arcirk::server::Devices, model);
        }
    }else if(model->server_object() == arcirk::server::server_objects::LocalhostUserCertificates){

        auto dlg_sel = DialogSelect(this);
        dlg_sel.setModal(true);
        dlg_sel.exec();
        if(dlg_sel.result() == QDialog::Accepted){
            auto result = dlg_sel.get_result();
            if(result == 0){
                auto file_name = QFileDialog::getOpenFileName(this, "Выбрать файл на диске", "", tr("Файлы сетрификатов (*.cer *.crt)"));
                auto cert = CryptCertificate(this);
                cert.fromFile(file_name);
                if(cert.isValid()){
                    auto conts = current_user->getContainers();
                    if(conts.is_object() && !conts.empty()){
                        modelSetTable(arcirk::server::LocalhostUserContainers, conts);
                    }
                    auto dlg = DialogSelectInTree(m_models[arcirk::server::server_objects::LocalhostUserContainers], this);
                    dlg.setModal(true);
                    dlg.set_window_text("Выбор контейнера");
                    dlg.exec();
                    if(dlg.result() == QDialog::Accepted){
                        auto cnt = dlg.selectedObject();
                        //qDebug() << cnt.dump().c_str();
                        auto volume = cnt.value("volume", "");
                        std::string cnt_str;
                        if(!volume.empty())
                            cnt_str = volume + cnt.value("name", "");

                        auto res =cert.install(cnt_str.c_str(), this);
                        if(res)
                            trayShowMessage("Сертификат успешно установлен!");
                        else
                            displayError("Ошибка", "Ошибка установки сертификата!");

                        auto certs = current_user->getCertificates(true);
                        if(certs.is_object() && !certs.empty()){
                            modelSetTable(arcirk::server::LocalhostUserCertificates, certs);
                        }
                    }
                }
            }else if(result == 1){
                database_get_certificates_synch();

                auto dlg = DialogSelectInTree(m_models[server::server_objects::Certificates], this);
                dlg.set_window_text("Сертификаты в базе данных");
                dlg.setModal(true);
                dlg.exec();
                if(dlg.result() == QDialog::Accepted){
                    auto object = dlg.selectedObject();
                    auto cnt_str = object.value("private_key", "");
                    QString container_path;
                    if(cnt_str.empty()){
                        database_get_containers_synch();
                        auto dlg_cnt = DialogSelectInTree(m_models[server::server_objects::Containers], this);
                        dlg_cnt.set_window_text("Контейнеры в базе данных");
                        dlg_cnt.setModal(true);
                        dlg_cnt.exec();
                        if(dlg_cnt.result() == QDialog::Accepted){
                            auto obj_cnt = dlg_cnt.selectedObject();
                            cnt_str = obj_cnt.value("ref", "");
                        }
                    }
                    auto s = object.value("suffix", "");
                    if(s.empty())
                        s = "cer";
                    auto file_name = object.value("first", "") + "." + s;
                    json query_param{
                        {"table_name", arcirk::enum_synonym(tables::tbCertificates)},
                        {"where_values", json{
                                {"ref", object["ref"].get<std::string>()}
                            }}
                    };
                    json param{
                        {"file_name", file_name},
                        {"destantion", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                    };

                    auto ba_cert = m_client->exec_http_query_get("GetBlob", param);
                    auto cert = CryptCertificate(this);
                    if(!cert.fromByteArray(ba_cert)){
                        displayError("Установка сертификата", "Ошибка установки сертификата");
                        return;
                    }

                    if(!cnt_str.empty()){
                        auto file_name_cnt = "container.bin";
                        json query_param_cnt{
                            {"table_name", arcirk::enum_synonym(tables::tbContainers)},
                            {"where_values", json{
                                    {"ref", cnt_str}
                                }}
                        };
                        json param_cnt{
                            {"file_name", file_name_cnt},
                            {"destantion", QByteArray(query_param_cnt.dump().data()).toBase64().toStdString()}
                        };

                        auto ba_cnt = m_client->exec_http_query_get("GetBlob", param_cnt);
                        if(!ba_cnt.isEmpty()){
                            auto cnt = CryptContainer(this);
                            cnt.init_user_info();
                            cnt.from_data(ba_cnt);
                            if(cnt.isValid()){
                                if(!cnt.install(arcirk::cryptography::TypeOfStorgare::storgareTypeRegistry)){
                                    qCritical() << "Ошибка устнановки контейнера!";
                                }else
                                    container_path = QString("\\\\.\\%1\\%2").arg(arcirk::enum_synonym(arcirk::cryptography::TypeOfStorgare::storgareTypeRegistry).c_str(), cnt.originalName());

                        }
                        }
                    }

                    auto result = cert.install(container_path, this);
                    if(!result){
                        displayError("Установка сертификата", "Ошибка установки севртификата");
                    }else{
                        trayShowMessage("Сертификат успешно установлен!");
                        auto certs = current_user->getCertificates(true);
                        if(certs.is_object() && !certs.empty())
                        modelSetTable(LocalhostUserCertificates, certs);
                    }
                }
            }
        }
    }
}


void MainWindow::onBtnDeleteClicked()
{
    auto model = get_model();
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    using namespace arcirk::server;
    using json = nlohmann::json;

    auto object = model->to_object(index);

    if(model->server_object() == server_objects::ProfileDirectory){

        int ind = model->column_index("path");
        auto file_path = model->index(index.row(), ind, index.parent()).data().toString();
        auto file_name = model->index(index.row(), 0, index.parent()).data().toString();
        auto result = QMessageBox::question(this, "Удаление файла", QString("Удалить файл %1").arg(file_name));

        if(result == QMessageBox::Yes) {
            json param{
                    {"file_name", file_path.toStdString()}
            };
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ProfileDeleteFile);
            auto resp = m_client->exec_http_query(command, param);
            if (resp.get<std::string>() == "success")
                model->remove(index);
        }
    }else if(model->server_object() == server_objects::DatabaseUsers){
        int ind = model->column_index("first");
        int indRef = model->column_index("ref");
        auto name = model->index(index.row(), ind, index.parent()).data().toString();
        auto ref = model->index(index.row(), indRef, index.parent()).data().toString();
        auto result = QMessageBox::question(this, "Удаление пользователя или группы", QString("Удалить пользователя или группу пользователей %1").arg(name));

        if(result == QMessageBox::Yes){
            using namespace arcirk::database::builder;
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);

            json query_param = {
                {"query_text", QString("DELETE FROM Users WHERE ref = '%1' OR parent = '%1';").arg(ref).toStdString()}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(command, query_param);
            model->remove(index);

        }
    }else if(model->server_object() == server_objects::Devices){
        int ind = model->column_index("first");
        int indRef = model->column_index("ref");
        auto name = model->index(index.row(), ind, index.parent()).data().toString();
        auto ref = model->index(index.row(), indRef, index.parent()).data().toString();
        auto result = QMessageBox::question(this, "Удаление устройства", QString("Удалить устройство %1").arg(name));

        if(result == QMessageBox::Yes){
            using namespace arcirk::database::builder;
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);

            json query_param = {
                {"query_text", QString("DELETE FROM Devices WHERE ref = '%1';").arg(ref).toStdString()}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(command, query_param);
            model->remove(index);
            update_icons(arcirk::server::Devices, model);
        }
    }else if(model->server_object() == server_objects::Certificates){
        int ind = model->column_index("first");
        int indRef = model->column_index("ref");
        auto name = model->index(index.row(), ind, index.parent()).data().toString();
        auto ref = model->index(index.row(), indRef, index.parent()).data().toString();
        auto result = QMessageBox::question(this, "Удаление сертификата", QString("Удалить сертификат %1").arg(name));

        if(result == QMessageBox::Yes){
            using namespace arcirk::database::builder;
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);

            json query_param = {
                {"query_text", QString("DELETE FROM Certificates WHERE ref = '%1';").arg(ref).toStdString()}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(command, query_param);
            model->remove(index);

        }
    }else if(model->server_object() == server_objects::Containers){
        int ind = model->column_index("first");
        int indRef = model->column_index("ref");
        auto name = model->index(index.row(), ind, index.parent()).data().toString();
        auto ref = model->index(index.row(), indRef, index.parent()).data().toString();
        auto result = QMessageBox::question(this, "Удаление контейнера", QString("Удалить контейнер %1").arg(name));

        if(result == QMessageBox::Yes){
            using namespace arcirk::database::builder;
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);

            json query_param = {
                {"query_text", QString("DELETE FROM Containers WHERE ref = '%1';").arg(ref).toStdString()}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(command, query_param);
            model->remove(index);

        }
    }else if(model->server_object() == server_objects::CertUsers){
        auto cert_user = arcirk::secure_serialization<arcirk::database::cert_users>(object);
        auto result = QMessageBox::question(this, "Удаление пользователя или группы", QString("Удалить  %1").arg(cert_user.first.c_str()));

        if(result == QMessageBox::Yes){
            using namespace arcirk::database::builder;
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);

            QString query_text;
            if(cert_user.is_group == 0)
                query_text  = QString("DELETE FROM CertUsers WHERE ref = '%1';").arg(cert_user.ref.c_str());
            else
                query_text  = QString("DELETE FROM CertUsers WHERE ref = '%1' or parent = '%1';").arg(cert_user.ref.c_str());

            json query_param = {
                {"query_text", query_text.arg(cert_user.ref.c_str()).toStdString()}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(command, query_param);
            model->remove(index);
        }
    }else if(model->server_object() == server_objects::LocalhostUserCertificates){
        auto object = model->to_object(index);
        std::string name = object["first"];

        if(QMessageBox::question(this, "Удаление сертификата", QString("Удалить сертификат %1?").arg(name.c_str())) == QMessageBox::No)
            return;
        auto cert = CryptCertificate(this);
        cert.remove(object["sha1"].get<std::string>().c_str(), this);
        auto certs = current_user->getCertificates(true);
        if(certs.is_object() && !certs.empty())
            modelSetTable(LocalhostUserCertificates, certs);
    }

}
void MainWindow::onBtnSetLinkDeviceClicked()
{
    auto model = get_model();
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }
    using namespace arcirk::server;
    using json = nlohmann::json;
    if(model->server_object() == arcirk::server::OnlineUsers){
        auto dlg = DialogSelectInList(m_models[Devices], "Выбор устройства", this);
        dlg.setModal(true);
        dlg.exec();
        if(dlg.result() == QDialog::Accepted){
            auto res = dlg.dialogResult();
            auto ind = m_models[Devices]->column_index("ref");
            auto indUuid = model->column_index("session_uuid");
            auto ref = res[ind];
            auto uuid = model->data(model->index(index.row(), indUuid, index.parent()), Qt::DisplayRole).toString();
            json param{
                    {"remote_session", uuid.toStdString()},
                    {"device_id", ref.toStdString()}
            };
            m_client->send_command(arcirk::server::server_commands::SetNewDeviceId, param);
        }
    }else if(model->server_object() == arcirk::server::Certificates){
        database_get_containers_synch();
        auto dlg = DialogSelectInList(m_models[Containers], "Выбор контейнера", this);
        dlg.setModal(true);
        dlg.exec();
        if(dlg.result() == QDialog::Accepted){
            auto res = dlg.dialogResult();
            auto ind = m_models[Containers]->column_index("ref");
            auto ref = res[ind];
            auto query_param = nlohmann::json{
                {"table_name", arcirk::enum_synonym(arcirk::database::tables::tbCertificates)},
                {"where_values", nlohmann::json{
                                                {"ref", m_models[Certificates]->to_object(index)["ref"]}
                    }
                },
                {"query_type", "update"},
                {"values", nlohmann::json{
                        {"private_key", ref.toStdString()}
                    }
                }
            };
            m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                                       {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                                   });
        }
    }
}


void MainWindow::onMnuAboutTriggered()
{
    auto dlg = DialogAbout(this);
    dlg.setModal(true);
    dlg.exec();
}


void MainWindow::onMnuOptionsTriggered()
{
    if(!m_client->isConnected())
        return;
    auto result_http = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ServerConfiguration), nlohmann::json{});
    auto conf = arcirk::secure_serialization<arcirk::server::server_config>(result_http, __FUNCTION__);
    auto dlg = DialogSeverSettings(conf, m_client->conf(), this);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){

        m_client->write_conf();
        auto result = dlg.getResult();
        m_client->send_command(arcirk::server::server_commands::UpdateServerConfiguration, nlohmann::json{
                                   {"config", pre::json::to_json(result)}
                               });

    }
}

void MainWindow::onBtnTaskRestartClicked()
{
    auto model = get_model();
    if(model->server_object() == arcirk::server::Services){
        m_client->send_command(arcirk::server::server_commands::TasksRestart, nlohmann::json{});
    }
}


void MainWindow::onBtnStartTaskClicked()
{
    auto model = get_model();
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }
    if(QMessageBox::question(this, "Выполнить задачу", "Выполнить текущую задачу,") == QMessageBox::No)
        return;

    auto object = arcirk::secure_serialization<arcirk::services::task_options>(model->to_object(index));
    m_client->send_command(arcirk::server::server_commands::RunTask, nlohmann::json{
                               {"task_uuid", object.uuid},
                               {"custom_interval", 0}
                           });
}


void MainWindow::onBtnSendClientReleaseClicked()
{
    auto model = get_model();
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    if(model->server_object() == arcirk::server::ProfileDirectory){

    }else if(model->server_object() == arcirk::server::Certificates){
        auto ind = model->column_index("ref");
        auto ind_suffix = model->column_index("suffix");
        auto ind_first = model->column_index("first");
        if(ind == -1 || ind_suffix == -1 || ind_first == -1)
            return;
        auto ref = model->index(index.row(), ind, index.parent()).data(Qt::DisplayRole).toString();
        auto suffix = model->index(index.row(), ind_suffix, index.parent()).data(Qt::DisplayRole).toString();
        auto first = model->index(index.row(), ind_first, index.parent()).data(Qt::DisplayRole).toString();
        if(ref.isEmpty() && suffix.isEmpty())
            return;
        using json = nlohmann::json;
        auto destantion = QFileDialog::getSaveFileName(this, "Сохранить как", QStandardPaths::writableLocation(QStandardPaths::HomeLocation), "Файлы сертификаторв (*." + suffix + ")");
        if(!destantion.isEmpty()){
            auto file_name = first + "." + suffix;
            json query_param{
                {"table_name", arcirk::enum_synonym(tables::tbCertificates)},
                {"where_values", json{
                        {"ref", ref.toStdString()}
                    }}
            };
            json param{
                {"file_name", file_name.toStdString()},
                {"destantion", QByteArray(query_param.dump().data()).toBase64().toStdString()}
            };

            auto ba = m_client->exec_http_query_get("GetBlob", param);

            if(ba.isEmpty())
                return;

            QFile file(destantion);
            if(file.open(QIODevice::WriteOnly)){
                file.write(ba);
                file.close();
            }

        }

    }else if(model->server_object() == arcirk::server::Containers){
        auto ind = model->column_index("ref");
        auto ind_first = model->column_index("first");
        if(ind == -1 || ind_first == -1)
            return;
        auto ref = model->index(index.row(), ind, index.parent()).data(Qt::DisplayRole).toString();
        auto first = model->index(index.row(), ind_first, index.parent()).data(Qt::DisplayRole).toString();

        if(ref.isEmpty())
            return;

        auto m_name = first.split("@");
        Q_ASSERT(m_name.size() == 2);

        QString def_folder = m_name[0];

        using json = nlohmann::json;
        auto destantion = QFileDialog::getExistingDirectory(this, "Выгрузить контейнер", QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        if(!destantion.isEmpty()){

            json query_param{
                {"table_name", arcirk::enum_synonym(tables::tbContainers)},
                {"where_values", json{
                        {"ref", ref.toStdString()}
                    }}
            };
            json param{
                {"file_name", m_name[0].toStdString() + ".bin"},
                {"destantion", QByteArray(query_param.dump().data()).toBase64().toStdString()}
            };

            auto ba = m_client->exec_http_query_get("GetBlob", param);

            if(ba.isEmpty())
                return;

            QTemporaryFile file;
            if(file.open()){
                file.write(ba);
                file.close();
            }

            auto cnt = CryptContainer(this);
            cnt.from_file(file.fileName().toStdString());
            Q_ASSERT(cnt.isValid());

            bool exists = true;
            int max = 999;
            int count = 0;
            QString result;
            while (exists) {
                //auto suffix = std::format("{:03}", count);
                std::string suffix = fmt::format("{:03}", count);
                if(count >= max)
                    break;
                QDir dir(destantion + "/" + def_folder + "." + suffix.c_str());
                if(!dir.exists()){
                    exists = false;
                    result = dir.path();
                }
            }

            if(!exists){
                cnt.to_dir(result);
            }

        }

    }else if(model->server_object() == arcirk::server::LocalhostUserCertificates){
        QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить как"),
                                                     QDir::homePath(),
                                                     "Файл сертификата (*.cer)");
        if(fileName != ""){
            int ind = model->column_index("sha1");
            QString sha1 = model->index(index.row(), ind, index.parent()).data(Qt::DisplayRole).toString();
            if(!sha1.isEmpty()){
                CryptCertificate::save_as(sha1, fileName, this);
            }
        }
    }
}


void MainWindow::onBtnSendoToClientClicked()
{
//    auto model = (TreeItemModel*)ui->treeView->model();
//    auto index = ui->treeView->currentIndex();
//    if(!index.isValid()){
//        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
//        return;
//    }
//    if(model->server_object() == arcirk::server::OnlineUsers){
//        auto ind = model->column_index("session_uuid");
//        auto rec = model->data(model->index(index.row(), ind, index.parent()), Qt::DisplayRole).toString();
//        m_client->command_to_client(rec.toStdString(), "GetForms");
//    }
}


void MainWindow::onBtnInfoClicked()
{
    auto model = get_model();//(TreeItemModel*)ui->treeView->model();
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    using json = nlohmann::json;

    if(model->server_object() == arcirk::server::Containers
            || model->server_object() == arcirk::server::Certificates
            || model->server_object() == arcirk::server::LocalhostUserCertificates){
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

    }else  if(model->server_object() == arcirk::server::LocalhostUserContainers){
        auto object = model->to_object(index);
        QString vol = object.value("volume", "").c_str();
        vol.append(object.value("name", "").c_str());
        auto info = current_user->get_container_info(vol);
            auto dlg = DialogInfo(info, object.value("name", "").c_str(), this);
            dlg.setModal(true);
            dlg.exec();
    }
}


void MainWindow::onBtnAddGroupClicked()
{

    auto model = get_model();

    auto index = treeView->current_index();

    if(model->server_object() == arcirk::server::CertUsers){
        auto struct_users = arcirk::database::table_default_struct<database::cert_users>(arcirk::database::tables::tbCertUsers);
        struct_users.is_group = 1;

        QString parent_name;
        QString parent_uuid = NIL_STRING_UUID;
        QModelIndex parent;

        int is_group = 0;

        if(index.isValid()){
            auto object = model->to_object(index);
            is_group = object["is_group"];
            if(is_group == 0){
                parent = index.parent();
                if(parent.isValid()){
                    auto object_parent = model->to_object(parent);
                    parent_name = QString::fromStdString(object_parent["first"].get<std::string>());
                    parent_uuid = QString::fromStdString(object_parent["ref"].get<std::string>());
                }else
                    parent = QModelIndex();
            }else{
                parent = index;
                parent_name = QString::fromStdString(object["first"].get<std::string>());
                parent_uuid = QString::fromStdString(object["ref"].get<std::string>());
            }
        }

        struct_users.parent = parent_uuid.toStdString();

        auto dlg = DialogEditCertUser(struct_users, parent_name, json::array(), this);
        dlg.setModal(true);
        dlg.exec();

        if(dlg.result() == QDialog::Accepted){
            nlohmann::json query_param = {
                {"table_name", arcirk::enum_synonym(tables::tbCertUsers)},
                {"query_type", "insert"},
                {"values", pre::json::to_json(struct_users)}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                                 {"query_param", base64_param}
                                             });

            if(resp == WS_RESULT_ERROR)
                displayError("Ошибка", "Ошибка на сервере");
            else{
                auto obj = pre::json::to_json(struct_users);
                if(struct_users.parent == NIL_STRING_UUID)
                    parent = QModelIndex();
                model->add(obj, parent);
            }
        }
    }

}


void MainWindow::onBtnRegistryDeviceClicked()
{
    auto model = get_model();//(TreeItemModel*)ui->treeView->model();
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    using namespace arcirk::database;

    if(model->server_object() == arcirk::server::OnlineUsers){

        auto sess_info = arcirk::secure_serialization<arcirk::client::session_info>(model->to_object(index), __FUNCTION__);

        if(sess_info.device_id == NIL_STRING_UUID){
            QMessageBox::critical(this, "Ошибка", "Устройство имеет пустой идентификатор!");
            return;
        }

        if(QMessageBox::question(this, "Регистрация устройства", QString("Зарегистрировать устройство %1").arg(sess_info.host_name.c_str())) == QMessageBox::No)
            return;

        auto dev = table_default_struct<devices>(tables::tbDevices);
        dev.address = sess_info.address;
        dev.deviceType = arcirk::enum_synonym(arcirk::server::device_types::devDesktop);
        dev.first = sess_info.host_name;
        dev.second = sess_info.product;
        dev.ref = sess_info.device_id;

        nlohmann::json query_param = {
            {"table_name", arcirk::enum_synonym(tables::tbDevices)},
            {"query_type", "insert"},
            {"values", pre::json::to_json(dev)}
        };
        std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
        QString error_message;
        auto error = [&error_message](const QString& what, const QString& command, const QString& err) -> void
        {
            qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << what << command << err;

            if(err.indexOf("UNIQUE constraint failed: Devices.ref") != -1){
                error_message = "Устройство уже зарегистрировано!";
            }else
                error_message = "Ошибка регистрации!";

        };
        auto c_err = connect(m_client, &WebSocketClient::error, error);
        auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                             {"query_param", base64_param}
                                         });

        std::string result = WS_RESULT_SUCCESS;
        if(resp.is_string())
            result = resp.get<std::string>();

        if(result == WS_RESULT_SUCCESS){
            trayShowMessage("Устройство успешно зарегистрировано!");
        }else{
            if(!error_message.isEmpty()){
                displayError("Регистрация устройства", error_message);
            }
        }
        disconnect(c_err);

    }
}

void MainWindow::onTrayTriggered()
{
    qDebug() << __FUNCTION__;

    auto *action = dynamic_cast<QAction*>( sender() );
    QString type_action = action->property("type").toString();

    auto dt = action->property("data").toString().toStdString();

    if(type_action == "mstsc"){        
        auto item = pre::json::from_json<arcirk::client::mstsc_options>(json::parse(dt));
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

//        QDir dir(mpl_.firefox_path.c_str());
        QString exeFile = "firefox";
#ifdef Q_OS_WINDOWS
        exeFile.append(".exe");
#endif
//        QFile exe(dir.path() + "/" + exeFile);
        QFile exe(m_client->conf().firefox.c_str());
        if(!exe.exists()){
            QString file = QFileDialog::getOpenFileName(this, tr("Путь к firefox"),
                                                        QDir::homePath(),
                                                        exeFile);
            if(file != ""){
                QFileInfo fi(file);
                exe.setFileName(file);
                m_client->conf().firefox = QDir::toNativeSeparators(fi.absolutePath()).toStdString();
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

void MainWindow::onAppExit()
{
    qDebug() << __FUNCTION__;

    QApplication::exit();
}

void MainWindow::onWindowShow()
{
    qDebug() << __FUNCTION__;

    setVisible(true);
}

void MainWindow::trayMessageClicked()
{
//    QMessageBox::information(nullptr, tr("Systray"),
//                             tr("Sorry, I already gave what help I could.\n"
//                                "Maybe you should try asking a human?"));
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    qDebug() << __FUNCTION__ << reason;

    if(reason == QSystemTrayIcon::DoubleClick){
        setVisible(true);
    }else if(reason == QSystemTrayIcon::Context){
        qDebug() << "QSystemTrayIcon::Context";
    }

}

void MainWindow::trayShowMessage(const QString& msg, int isError)
{
    if(!isError)
        trayIcon->showMessage("Менеджер сертификатов", msg);
    else{
        displayError("Ошибка", msg);
    }

}

void MainWindow::onOpenTreeModelSettingsDialog(TreeItemModel *model)
{
//    auto dlg = DialogTreeModelSettings(model->conf(), this);
//    dlg.exec();
}

void MainWindow::setVisible(bool visible)
{
    QMainWindow::setVisible(visible);
}

QIcon MainWindow::get_link_icon(const QString &link)
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

void MainWindow::run_mstsc_link(const arcirk::client::mstsc_options &opt)
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
        //std::string result_ = arcirk::to_utf(data.toStdString(), "cp866");
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

void MainWindow::closeEvent(QCloseEvent *event)
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

void MainWindow::onBtnRegistryUserClicked()
{
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }
    auto model = get_model();

    if(model->server_object() == arcirk::server::OnlineUsers){
        auto object = pre::json::from_json<arcirk::client::session_info>(model->to_object(index));
        if(QMessageBox::question(this, "Регистрация пользователя", QString("Зарегистировать пользователя '%1'?").arg(object.system_user.c_str())) == QMessageBox::No)
            return;

//        auto m_tree = new TreeItemModel(this);
//        //m_tree->set_group_only(true);

//        //m_tree->use_hierarchy("first");

//        m_tree->set_column_aliases(m_colAliases);
        auto m_tree = new ITree<arcirk::database::cert_users>(this);
        m_tree->set_fetch_expand(true);
        m_tree->set_user_sql_where(json{{"is_group", 1}});
        m_tree->set_hierarchical_list(true);
        m_tree->set_connection(root_tree_conf::typeConnection::httpConnection,
                               http_conf(m_client->http_url().toString().toStdString(), m_client->conf().hash, "CertUsers"));
        //m_tree->fetchRoot(arcirk::enum_synonym(arcirk::database::tables::tbCertUsers).c_str());

        auto dlg_sel_group = DialogSelectInTree(m_tree, this);
        dlg_sel_group.allow_sel_group(true);
        dlg_sel_group.set_window_text("Выбор группы");
        dlg_sel_group.setModal(true);
        dlg_sel_group.exec();

        if(dlg_sel_group.result() == QDialog::Accepted){
            auto group = dlg_sel_group.selectedObject();
            auto parent_struct = pre::json::from_json<arcirk::database::cert_users>(group);
            add_cert_user(parent_struct, object.host_name.c_str(), object.user_name.c_str(), object.user_uuid.c_str(), object.system_user.c_str(), object.sid.c_str());
        }
        delete m_tree;
    }
}

void MainWindow::add_cert_user(const arcirk::database::cert_users &parent, const QString& host, const QString& name, const QString& uuid, const QString& system_user, const QString& sid)
{
    using namespace arcirk::server;
    using json = nlohmann::json;

    auto struct_users = arcirk::database::table_default_struct<arcirk::database::cert_users>(arcirk::database::tables::tbCertUsers);
    struct_users.parent = parent.ref;
    struct_users.is_group = 0;
    struct_users.host = host.toStdString();
    struct_users.first = name.toStdString();
    struct_users.second = name.toStdString();
    struct_users.uuid = uuid.toStdString();
    struct_users.system_user = system_user.toStdString();
    struct_users.sid = sid.toStdString();

    //m_models[DatabaseUsers]->fetchRoot("Users");

    auto types = json::array({enum_synonym(device_types::devDesktop), enum_synonym(device_types::devServer)});
    json query_param = {
        {"table_name", arcirk::enum_synonym(tables::tbDevices)},
        {"query_type", "select"},
        {"values", json{"first", "ref"}},
        {"where_values", json{{"deviceType", types}}}
    };
    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
    auto dev = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                         {"query_param", base64_param}
                                     });
    auto devices = dev.value("rows", json::array());

    auto dlg = DialogEditCertUser(struct_users, parent.first.c_str(), m_models[arcirk::server::server_objects::DatabaseUsers], devices, this);

    if(!uuid.isEmpty()){
        json query_param = {
            {"table_name", arcirk::enum_synonym(tables::tbUsers)},
            {"query_type", "select"},
            {"values", json{"first"}},
            {"where_values", json{{"ref", uuid.toStdString()}}}
        };
        std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
        auto us = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                             {"query_param", base64_param}
                                         });
        auto arr = us.value("rows", json::array());
        if(arr.size() > 0){
            std::string us_name = arr[0].value("first", "");
            dlg.set_1c_parent(us_name.c_str());
        }
    }

    connect(&dlg, &DialogEditCertUser::selectRemoteUser, this, &MainWindow::onSelectRemoteUser);
    connect(this, &MainWindow::selectRemoteSystemUser, &dlg, &DialogEditCertUser::onSelectRemoteUser);

    if(dlg.exec() == QDialog::Accepted){
        json obj;
        if(is_cert_user_exists(struct_users.host.c_str(), struct_users.system_user.c_str(), obj)){
            displayError("Ошибка", QString("Пользователь %1/%2 уже зарегистрирован!").arg(struct_users.host.c_str(), struct_users.system_user.c_str()));
            return;
        }
        json query_param = {
            {"table_name", "CertUsers"},
            {"query_type", "insert"},
            {"values", pre::json::to_json(struct_users)}
        };
        QString error_message;
        auto error = [&error_message](const QString& what, const QString& command, const QString& err) -> void
        {
            qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << what << command << err;

            if(err.indexOf("UNIQUE constraint failed: CertUsers.ref") != -1){
                error_message = "Пользователь уже зарегистрирован!";
            }else
                error_message = "Ошибка регистрации!";

        };
        auto c_err = connect(m_client, &WebSocketClient::error, error);

        std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
        auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::ExecuteSqlQuery), json{
                                             {"query_param", base64_param}
                                         });

        std::string result = "success";
        if(resp.is_string())
            result = resp.get<std::string>();

        if(result == WS_RESULT_SUCCESS){
            trayShowMessage("Пользователь успешно зарегистрирован!");
            //m_models[CertUsers]->fetchRoot("CertUsers");
        }else{
            if(!error_message.isEmpty()){
                displayError("Регистрация пользователя", error_message);
            }
        }
        disconnect(c_err);

    }
}

bool MainWindow::is_cert_user_exists(const QString &host, const QString &system_user, nlohmann::json& object)
{
    using namespace arcirk::server;
    using json = nlohmann::json;

    json query_param = {
        {"table_name", arcirk::enum_synonym(tables::tbCertUsers)},
        {"query_type", "select"},
        {"values", json{}},
        {"where_values", json{
                {"host", host.toStdString()},
                {"system_user", system_user.toStdString()}
            }
        }
    };
    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
    auto us = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                         {"query_param", base64_param}
                                     });
    auto arr = us.value("rows", json::array());
    if(arr.size() > 0){
        object = arr[0];
    }
    return arr.size() > 0;
}

QModelIndex MainWindow::is_user_online(const QString &host, const QString &system_user, const QString& app_name)
{
    using namespace arcirk::database;
    using json = nlohmann::json;
    using namespace arcirk::server;

    auto query_param = json{
            {"table", true},
            {"uuid_form", NIL_STRING_UUID},
            {"empty_column", false}
    };

    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
    auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ServerOnlineClientsList), query_param);
    if(resp  != WS_RESULT_ERROR){
        if(resp.is_object()){
            auto online_users = m_models[OnlineUsers];
            modelSetTable(OnlineUsers, resp);
            for (auto i = 0; i < online_users->rowCount(QModelIndex()); i++) {
                auto object = online_users->to_object(online_users->index(i, 0, QModelIndex()));
                if(!object.empty()){
                    auto struct_obj = arcirk::secure_serialization<client::session_info>(object, __FUNCTION__);
                    if(struct_obj.host_name == host.toStdString() && struct_obj.system_user == system_user.toStdString()){
                        if(app_name.isEmpty())
                            return online_users->index(i, 0, QModelIndex());
                        else
                            if(app_name.toStdString() == struct_obj.app_name)
                                return online_users->index(i, 0, QModelIndex());
                    }
                }
            }
        }
    }

    return QModelIndex();
}

void MainWindow::onBtnEditCacheClicked()
{
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = get_model();//(TreeItemModel*)ui->treeView->model();
    using namespace arcirk::database;
    using json = nlohmann::json;
    using namespace arcirk::server;

    if(model->server_object() == arcirk::server::CertUsers){
        auto is_group = model->data(model->index(index.row(), model->column_index("is_group"), index.parent()), Qt::DisplayRole).toInt();
        if(is_group == 1)
            return;
        auto obj = model->to_object(index);
        auto struct_user = pre::json::from_json<arcirk::database::cert_users>(obj);
        //m_models[DatabaseUsers]->fetchRoot("Users");

        auto types = json::array({enum_synonym(device_types::devDesktop), enum_synonym(device_types::devServer)});
        json query_param = {
            {"table_name", arcirk::enum_synonym(tables::tbDevices)},
            {"query_type", "select"},
            {"values", json{"first", "ref"}},
            {"where_values", json{{"deviceType", types}}}
        };
        std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
        auto dev = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                             {"query_param", base64_param}
                                         });
        auto rows = dev.value("rows", json::array());
        auto dlg = DialogCertUserCache(struct_user, m_models[server_objects::DatabaseUsers], m_client->conf().server_host.c_str(), this);
        dlg.set_is_localhost(struct_user.host == current_user->host().toStdString() && struct_user.system_user == current_user->user_name().toStdString());

        connect(&dlg, &DialogCertUserCache::getData, this, &MainWindow::onCertUserCache);
        connect(this, &MainWindow::certUserData, &dlg, &DialogCertUserCache::onCertUserCache);
        connect(this, &MainWindow::mozillaProfiles, &dlg, &DialogCertUserCache::doMozillaProfiles);
        connect(&dlg, &DialogCertUserCache::getMozillaProfiles, this, &MainWindow::onMozillaProfiles);
        connect(&dlg, &DialogCertUserCache::getCertificates, this, &MainWindow::doGetCertUserCertificates);
        connect(this, &MainWindow::certUserCertificates, &dlg, &DialogCertUserCache::onCertificates);
        connect(&dlg, &DialogCertUserCache::selectHosts, this, &MainWindow::doSelectHosts);
        connect(this, &MainWindow::selectHosts, &dlg, &DialogCertUserCache::onSelectHosts);
        connect(&dlg, &DialogCertUserCache::selectDatabaseUser, this, &MainWindow::doSelectDatabaseUser);
        connect(this, &MainWindow::selectDatabaseUser, &dlg, &DialogCertUserCache::onSelectDatabaseUser);
        connect(&dlg, &DialogCertUserCache::getContainers, this, &MainWindow::doGetCertUserContainers);
        connect(this, &MainWindow::certUserContainers, &dlg, &DialogCertUserCache::onContainers);
        connect(this, &MainWindow::availableCertificates, &dlg, &DialogCertUserCache::onAvailableCertificates);
        connect(&dlg, &DialogCertUserCache::selectCertificate, this, &MainWindow::onSelectCertificate);
        connect(this, &MainWindow::selectCertificate, &dlg, &DialogCertUserCache::onSelectCertificate);

        dlg.setModal(true);
        dlg.exec();
        if(dlg.result() == QDialog::Accepted){
            auto n_object = pre::json::to_json(struct_user);
            auto query_param = nlohmann::json{
                {"table_name", enum_synonym(tables::tbCertUsers)},
                {"where_values", nlohmann::json{{"ref", struct_user.ref}}},
                {"values", n_object},
                {"query_type", "update"}
            };

            m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                                       {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                                   });
            model->set_object(index, n_object);
            if(struct_user.system_user == current_user->user_name().toStdString() &&
                    struct_user.host == current_user->host().toStdString()){
                current_user->set_database_cache(struct_user.cache);
                auto cache = current_user->cache();
                auto mstsc_param = cache.value("mstsc_param", json::object());
                auto detailed_records = mstsc_param.value("detailed_records", json::array());
                update_rdp_files(detailed_records);
                createDynamicMenu();
            }
        }
    }
}

QString MainWindow::rdp_file_text()
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

QString MainWindow::cache_mstsc_directory()
{
    auto app_data = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    app_data.append("/mstsc");
    QDir f(app_data);
    if(!f.exists())
        f.mkpath(f.path());
    return f.path();
}

void MainWindow::onBtnSystemUsersClicked()
{
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = get_model();

    using namespace arcirk::database;
    using json = nlohmann::json;
    using namespace arcirk::server;

    if(model->server_object() == Devices){
        auto object = model->to_object(index);

        auto type = object["device_type"].get<device_types>();
        if(type != device_types::devServer && type != device_types::devDesktop){
            QMessageBox::critical(this, "Ошибка", "Для данного устройства не доступен список пользователей!");
            return;
        }

        auto query_param = json{
                {"table", true},
                {"uuid_form", NIL_STRING_UUID},
                {"empty_column", false}
        };

        std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
        auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ServerOnlineClientsList), query_param);
        if(resp  != WS_RESULT_ERROR){
            if(resp.is_object()){
                auto online_users = m_models[OnlineUsers];
                modelSetTable(OnlineUsers, resp);
                //auto index = TreeItemModel::find_in_table(online_users, object["ref"].get<std::string>().c_str(), online_users->column_index("device_id"));
                auto index = online_users->find(online_users->column_index("device_id"), object["ref"].get<std::string>().c_str(),  QModelIndex());
                if(!index.isValid()){
                    QMessageBox::critical(this, "Ошибка", "Устройство не в сети!");
                    return;
                }

                auto session = arcirk::secure_serialization<arcirk::client::session_info>(online_users->to_object(index));
                auto result = get_local_users_details(session.host_name);
                if(!result.empty()){
                    auto usrs = new TreeItemModel(this);
                    usrs->set_table(result);
                    auto dlg = DialogSelectInTree(usrs, {}, this);
                    dlg.set_window_text(QString("Пользователи на хосте %1").arg(session.host_name.c_str()));
                    dlg.exec();
                    delete usrs;
                }
            }
        }

    }
}

TreeItemModel *MainWindow::get_model()
{
//    auto sort_model = (TreeSortModel*)ui->treeView->model();
//    if(sort_model == 0)
//        return nullptr;
//    auto model = (TreeItemModel*)sort_model->sourceModel();
//    if(model == 0)
//        return nullptr;
//    return model;
    return treeView->get_model();
}

QModelIndex MainWindow::get_index(const QModelIndex& proxy_index)
{
    if(!proxy_index.isValid())
        return QModelIndex();
    auto sort_model = (TreeSortModel*)treeView->model();
    Q_ASSERT(sort_model != 0);
    return sort_model->mapToSource(proxy_index);
}

void MainWindow::onCommandToClient(const arcirk::server::server_response &message)
{
    qDebug() << __FUNCTION__;
    using json = nlohmann::json;

    auto param = json::parse(QByteArray::fromBase64(QByteArray::fromStdString(message.param)).toStdString());
    auto command = param.value("command", "");
    qDebug() << command.c_str();
    if(command == "ResetCache"){

    }else if(command == "onGetCertificatesList"){
        auto certs = json::parse(QByteArray::fromBase64(QByteArray::fromStdString(param["parameters"].get<std::string>())).toStdString());
        emit availableCertificates(certs);
    }else if(command == "GetLocalUsers"){
        auto usrs = json::parse(QByteArray::fromBase64(QByteArray::fromStdString(param["parameters"].get<std::string>())).toStdString());
        emit selectGetLocalUsers(usrs);
    }

}

void MainWindow::doGetMessages(const json &param)
{
    qDebug() << __FUNCTION__;
    if(m_client->isConnected())
        m_client->send_command(arcirk::server::server_commands::GetMessages, param);
}

void MainWindow::onSendMessage(const QUuid &recipient, const QString &message, const json &param)
{
    qDebug() << __FUNCTION__;
    m_client->send_message(recipient.toString(QUuid::WithoutBraces).toStdString(),
                           message.toStdString(), param);

}

void MainWindow::doUserMessage(const arcirk::server::server_response &message)
{
    qDebug() << __FUNCTION__;
    emit userMessage(message);
}

void MainWindow::onSelectGetLocalUsers(const nlohmann::json &result)
{
    auto model = new TreeItemModel(this);
    model->set_table(result);
    auto dlg = DialogSelectInTree(model, {}, this);
    if(dlg.exec() == QDialog::Accepted){
        auto object = secure_serialization<win_user_info>(dlg.selectedObject());
        emit selectRemoteSystemUser(object);
    }
}

void MainWindow::onSelectRemoteUser(const QString &host)
{
    auto result = get_local_users_details(host.toStdString());
    if(result.empty()){
        displayError("Ошибка", "Клиент не всети.");
        return;
    }

    onSelectGetLocalUsers(result);

}

void MainWindow::onOpenTableViewDialog(tables table)
{
    auto dlg = DialogSqlTableViewer(table, m_client, this);
    dlg.exec();
}

void MainWindow::onDownloadAnydesk()
{
    infoIco->setVisible(true);
    infoIco->movie()->start();
    infoBar->setText("Загрузка файла anydesk.apk");

    auto manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply){
        // Если в процесе получения данных произошла ошибка
        if(reply->error()){
            // Сообщаем об этом и показываем информацию об ошибках
            qDebug() << "ERROR";
            qDebug() << reply->errorString();
            infoIco->setVisible(false);
            infoIco->movie()->stop();
            infoBar->setText(QString("Подключен: %1 (%2)").arg(m_client->conf().server_host.c_str(), m_client->server_conf().ServerName.c_str()));
        } else {
            // В противном случае создаём объект для работы с файлом
            //auto temp = QTemporaryDir();
            QFile file(QTemporaryDir().path() + QDir::separator() + "anydesk.apk");
            if(file.exists())
                file.remove();
            //QFile *file = new QFile("C:/example/file.xml");
            // Создаём файл или открываем его на перезапись ...
            if(file.open(QFile::WriteOnly)){
                file.write(reply->readAll());  // ... и записываем всю информацию со страницы в файл
                file.close();                  // закрываем файл
                //qDebug() << "Downloading is completed";
                onDownloadAnydeskComlete(file.fileName()); // Посылаем сигнал о завершении получения файла
            }
        }
    });

    QUrl url("https://download.anydesk.com/anydesk.apk"); // URL, к которому будем получать данные
    QNetworkRequest request;    // Отправляемый запрос
    request.setUrl(url);        // Устанавлвиваем URL в запрос
    manager->get(request);      // Выполняем запрос
}

void MainWindow::onDownloadAnydeskComlete(const QString &file)
{
    //qDebug() << file;

    //QFile f(file);
    infoBar->setText("Загрузка файла anydesk.apk на сервер");
    if(!QFile::exists(file)){
        displayError("Ошибка", "Ошибка загрузки файла anydesk.apk!");
    }else{
        ByteArray data{};
        arcirk::read_file(file.toStdString(), data);
        QString destantion  = "html\\files";
        json param{
            {"destantion", destantion.toUtf8().toBase64().toStdString()},
            {"file_name", "anydesk.apk"}
        };
        auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::DownloadFile), param, data);
        if(resp == WS_RESULT_ERROR)
            displayError("Ошибка", "Ошибка копирования файла на сервер!");
        else{
            if(resp.is_object()){
                trayShowMessage("Файл успешно загружен на сервер!");
            }
        }
    }
    infoIco->setVisible(false);
    infoIco->movie()->stop();
    infoBar->setText(QString("Подключен: %1 (%2)").arg(m_client->conf().server_host.c_str(), m_client->server_conf().ServerName.c_str()));


}

void MainWindow::verify_certificate(TreeItemModel *model)
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

void MainWindow::update_toolbar(const arcirk::server::server_objects &key)
{

    //using json = nlohmann::json;
    using namespace arcirk::server;

    QVector<QToolButton*> m_edit_buttons{
        ui->btnAdd, ui->btnEdit, ui->btnDelete, ui->btnAddGroup
    };
    QVector<QToolButton*> m_export_buttons{
        ui->btnDataImport, ui->btnDataExport
    };
    QVector<QToolButton*> m_tasks_buttons{
        ui->btnStartTask, ui->btnTaskRestart
    };
    QVector<QToolButton*> m_register_buttons{
        ui->btnRegistryUser, ui->btnRegistryDevice
    };
    auto btn_save = ui->btnSendClientRelease;
    auto btn_info = ui->btnInfo;
    auto btn_refresh = ui->btnRefresh;
    auto btn_edit = ui->btnEdit;
    auto btn_connect = ui->btnSetLinkDevice;
    auto btn_cert_cache = ui->btnEditCache;
    auto btn_add_groupe = ui->btnAddGroup;
    auto btn_move_to = ui->btnMoveTo;

    if(key == server_objects::Root || key == server_objects::Database ||
       key == server_objects::CertManager ||
            key == server_objects::LocalhostUser){
        ui->widgetToolBar->setEnabled(false);
    }else
        ui->widgetToolBar->setEnabled(true);

    if(key == server_objects::OnlineUsers){
        set_enabled_buttons(m_edit_buttons, false);
        set_enabled_buttons(m_export_buttons, false);
        set_enabled_buttons(m_register_buttons, true);
        set_enabled_buttons(m_tasks_buttons, false);
        btn_save->setEnabled(false);
        btn_info->setEnabled(false);
        btn_refresh->setEnabled(true);
        btn_connect->setEnabled(true);
        btn_cert_cache->setEnabled(false);
        btn_move_to->setEnabled(false);
    }else if(key == server_objects::DatabaseUsers){
        set_enabled_buttons(m_edit_buttons, true);
        set_enabled_buttons(m_export_buttons, false);
        set_enabled_buttons(m_register_buttons, false);
        set_enabled_buttons(m_tasks_buttons, false);
        btn_save->setEnabled(false);
        btn_info->setEnabled(false);
        btn_refresh->setEnabled(true);
        btn_cert_cache->setEnabled(false);
        btn_move_to->setEnabled(false);
    }else if(key == server_objects::ProfileDirectory){
        set_enabled_buttons(m_edit_buttons, true);
        set_enabled_buttons(m_export_buttons, false);
        set_enabled_buttons(m_register_buttons, false);
        set_enabled_buttons(m_tasks_buttons, false);
        btn_save->setEnabled(true);
        btn_edit->setEnabled(false);
        btn_info->setEnabled(false);
        btn_connect->setEnabled(false);
        btn_refresh->setEnabled(true);
        btn_cert_cache->setEnabled(false);
        btn_move_to->setEnabled(true);
        btn_save->setToolTip("Обновить версию мобильного приложения");
    }else if(key == server_objects::Services){
        set_enabled_buttons(m_edit_buttons, false);
        set_enabled_buttons(m_export_buttons, false);
        set_enabled_buttons(m_register_buttons, false);
        set_enabled_buttons(m_tasks_buttons, true);
        btn_save->setEnabled(false);
        btn_edit->setEnabled(true);
        btn_info->setEnabled(false);
        btn_refresh->setEnabled(true);
        btn_connect->setEnabled(false);
        btn_cert_cache->setEnabled(false);
        btn_move_to->setEnabled(false);
    }else if(key == server_objects::CertUsers){
        set_enabled_buttons(m_edit_buttons, true);
        set_enabled_buttons(m_export_buttons, false);
        set_enabled_buttons(m_register_buttons, false);
        set_enabled_buttons(m_tasks_buttons, false);
        btn_save->setEnabled(false);
        btn_info->setEnabled(false);
        btn_refresh->setEnabled(true);
        btn_connect->setEnabled(false);
        btn_cert_cache->setEnabled(true);
        btn_move_to->setEnabled(true);
    }else if(key == server_objects::Devices){
        set_enabled_buttons(m_edit_buttons, true);
        set_enabled_buttons(m_export_buttons, false);
        set_enabled_buttons(m_register_buttons, false);
        set_enabled_buttons(m_tasks_buttons, false);
        btn_save->setEnabled(false);
        btn_info->setEnabled(false);
        btn_refresh->setEnabled(true);
        btn_connect->setEnabled(false);
        btn_cert_cache->setEnabled(false);
        btn_add_groupe->setEnabled(false);
        btn_move_to->setEnabled(false);
    }else if(key == server_objects::Certificates
             || key == server_objects::Containers
             || key == server_objects::LocalhostUserCertificates
             || key == server_objects::LocalhostUserContainers){
        set_enabled_buttons(m_edit_buttons, true);
        set_enabled_buttons(m_export_buttons, false);
        set_enabled_buttons(m_register_buttons, false);
        set_enabled_buttons(m_tasks_buttons, false);
        btn_save->setEnabled(true);
        btn_info->setEnabled(true);
        btn_refresh->setEnabled(true);
        btn_connect->setEnabled(true);
        btn_cert_cache->setEnabled(false);
        btn_add_groupe->setEnabled(false);
        btn_move_to->setEnabled(false);
        btn_save->setToolTip("Сохранить на диск");
    }else if(key == server_objects::DatabaseTables){
        set_enabled_buttons(m_edit_buttons, false);
        set_enabled_buttons(m_export_buttons, false);
        set_enabled_buttons(m_register_buttons, false);
        set_enabled_buttons(m_tasks_buttons, false);
        btn_save->setEnabled(false);
        btn_info->setEnabled(false);
        btn_refresh->setEnabled(true);
        btn_connect->setEnabled(false);
        btn_cert_cache->setEnabled(false);
        btn_add_groupe->setEnabled(false);
        btn_move_to->setEnabled(false);
        btn_edit->setEnabled(true);

    }
}

void MainWindow::sortByColumn(QTreeView *tree, int column, Qt::SortOrder ord)
{
    if(!tree){
        treeView->sortByColumn(column, ord);
    }else
        tree->sortByColumn(column, ord);
}

void MainWindow::setIndentation(server::server_objects key)
{
//    using namespace arcirk::server;

//    if (m_models[key]->hierarchical_list())
//        ui->treeView->resetIndentation();
//    else
//        ui->treeView->setIndentation(5);
}

json MainWindow::get_local_users_details(const std::string &host)
{
    bool is_localhost = current_user->host().toStdString() == host;

    if(is_localhost){
        auto cmd = CommandLine(this);
        QString cryptoPro = cryptoProDirectory().path();
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
    }else{
        auto online_users = m_models[arcirk::server::server_objects::OnlineUsers];
        //auto index = online_users->find(host.c_str(), online_users->column_index("host_name"), QModelIndex());
        auto index = online_users->find(json{
                                            {"host_name", host},
                                            {"app_name", arcirk::enum_synonym(arcirk::server::application_names::ProfileManager)}
                                        }, QModelIndex());
        if(index.isValid()){
            auto object = arcirk::secure_serialization<arcirk::client::session_info>(online_users->to_object(index));
            m_client->command_to_client(object.session_uuid, "GetLocalUsers", {});
        }
        return {};
    }
}

void MainWindow::load_table_demo(database::tables table)
{

    if(table == database::tables::tbMessages){

        QVector<QString> uuids({"0ec3a955-7d38-47ee-8ce2-cd710b2b478d", "f3ccb2f2-d431-11e9-ab42-08606e7d17fa"});

        bool d = false;

        foreach (auto uuid, uuids) {
            json query_param = {
                {"first", uuid.toStdString()},
                {"second", m_client->currentUserUuid().toString(QUuid::WithoutBraces).toStdString()}
            };
            auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::GetChannelToken), query_param);

            if(resp == WS_RESULT_ERROR || resp.is_null())
                return;

            std::string channel_token = resp.get<std::string>();
            if(channel_token.empty())
                return;

            qDebug() <<  channel_token.c_str();

//            query_param = {
//                {"uuid", channel_token}
//            };
            query_param = {
                {"uuid", uuid.toStdString()}
            };

            resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::IsChannel), query_param);

            if(resp == WS_RESULT_ERROR || resp.is_null())
                return;

            bool is_chanel = false;
            if(resp.is_boolean()){
                is_chanel = resp.get<bool>();
            }

            int start_date = arcirk::add_day(arcirk::start_day(arcirk::current_date()), -10);
            auto curr_uuid = m_client->currentUserUuid().toString(QUuid::WithoutBraces).toStdString() ;
            json vals_array = json::array();
            for (int i = 0; i < 10; ++i) { //цикл по дням
                int current_day = arcirk::add_day(start_date, i);
                int end_day = arcirk::end_day(arcirk::seconds_to_date(current_day));

                boost::random::mt19937 rng;
                boost::random::uniform_int_distribution<> n(0, 10);
                int count = n(rng);

                for (int it = 0; it < count; ++it) { //цикл сообщений до 10
                    boost::random::uniform_int_distribution<> six(current_day, end_day);
                    auto msg = arcirk::database::table_default_struct<database::messages>(tables::tbMessages);
                    msg.date = six(rng);
                    msg.parent = current_day;
                    msg.first = is_chanel ? curr_uuid : d ? curr_uuid : uuid.toStdString();
                    msg.second = !is_chanel ? curr_uuid : !d ? curr_uuid : uuid.toStdString();
                    msg.token = channel_token;
                    msg.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
                    vals_array += pre::json::to_json(msg);
                    d = !d;
                }

            }
            query_param = {
                {"table_name", "Messages"},
                {"values_array", vals_array}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::InsertToDatabaseFromArray), json{
                                              {"query_param", base64_param}
                                          });

            if(resp == WS_RESULT_ERROR || resp.is_null())
                return;

        }
    }

}

void MainWindow::onBtnRefreshClicked()
{
    auto model = get_model();//(TreeItemModel*)ui->treeView->model();
//    auto index = ui->treeView->currentIndex();
//    if(!index.isValid()){
//        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
//        return;
//    }

    using namespace arcirk::server;

    if(model->server_object() == server_objects::ProfileDirectory){

    }else if(model->server_object() == server_objects::Certificates){
        database_get_certificates_asynch();
    }else if(model->server_object() == server_objects::Containers){
        database_get_containers_asynch();
    }else if(model->server_object() == server_objects::DatabaseTables){
        if(m_client->isConnected())
            m_client->send_command(arcirk::server::server_commands::GetDatabaseTables, nlohmann::json{});
    }
}


void MainWindow::onMnuUpdatePriceCheckerTriggered()
{
    if(!m_client->isConnected())
        return;

    QString result = QString::fromStdString(m_client->conf().price_checker_repo) + "/android-build-release-signed.apk";
    if(!QFile(result).exists())
    {
        QMessageBox::critical(this, "Ошибка", "Не верный путь к репозиторию!");
        return;
    }else{
        if(QMessageBox::question(this, "Загрузка версии", "Загузить новую версию мобильного приложения?") == QMessageBox::No)
            return;
    }

    ByteArray data{};
    try {
        arcirk::read_file(result.toStdString(), data);
        if(data.size() > 0){
            QString destantion  = "html\\files";
            auto ver = WebSocketClient::get_version();
            auto dlg = DialogSetVersion(ver, this);
            if(dlg.exec()){
                ver = dlg.getResult();
            }else
                return;
            json param{
                {"destantion", destantion.toUtf8().toBase64().toStdString()},
                {"file_name", QString("checker_v%1_%2_%3.apk").arg(QString::number(ver.major), QString::number(ver.minor), QString::number(ver.path)).toStdString()}
            };
            infoIco->movie()->start();
            infoIco->setVisible(true);
            infoBar->setText("Установка новой версии Прайс-чекера...");
            auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::DownloadFile), param, data);
            infoIco->setVisible(false);
            infoIco->movie()->stop();
            infoBar->setText(QString("Подключен: %1 (%2)").arg(m_client->conf().server_host.c_str(), m_client->server_conf().ServerName.c_str()));
            if(resp == WS_RESULT_ERROR)
                displayError("Ошибка", "Ошибка копирования файла на сервер!");
            else{
                if(resp.is_object()){
                    trayShowMessage("Файл успешно загружен на сервер!");
                }
            }
        }

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", e.what());
        return;
    }


}


void MainWindow::onBtnMoveToClicked()
{
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = get_model();

    using namespace arcirk::database;
    using json = nlohmann::json;
    using namespace arcirk::server;

    if(model->server_object() == server_objects::CertUsers){

        auto m_tree = new TreeItemModel(this);
        //m_tree->set_group_only(true);
        //m_tree->use_hierarchy("first");
        m_tree->set_column_aliases(m_colAliases);
        //m_tree->fetchRoot(arcirk::enum_synonym(arcirk::database::tables::tbCertUsers).c_str());

        auto dlg_sel_group = DialogSelectInTree(m_tree, this);
        dlg_sel_group.allow_sel_group(true);
        dlg_sel_group.set_window_text("Выбор группы");

        if(dlg_sel_group.exec() == QDialog::Accepted){
            auto group = arcirk::secure_serialization<cert_users>(dlg_sel_group.selectedObject());
            auto item = arcirk::secure_serialization<cert_users>(model->to_object(index));
            if(group.ref == item.parent)
                return;

            auto new_parent = model->find(model->column_index("ref"), group.ref.c_str(), QModelIndex());
            json query_param = {
                {"table_name", arcirk::enum_synonym(tables::tbCertUsers)},
                {"query_type", "update"},
                {"values", json{{"parent", group.ref}}},
                {"where_values", json{{"ref", item.ref}}}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                                 {"query_param", base64_param}
                                             });
            if(new_parent.isValid()){ //группа может не быть загружена в дерево
                if(resp == WS_RESULT_SUCCESS)
                    model->move_to(index, new_parent);
            }else{
                if(resp == WS_RESULT_SUCCESS)
                    model->remove(index);
            }
        }

        delete m_tree;
    }
}


void MainWindow::onMnuMessanegerTriggered()
{
    qDebug() << __FUNCTION__;

    if(m_messanager != 0){
        m_messanager->setModal(false);
        m_messanager->open();
        auto param = json::object();
        param["sender"] = m_client->currentSession().toByteArray(QUuid::WithoutBraces).toStdString();
        param["recipient"] = SHARED_CHANNEL_UUID;
        doGetMessages(param);
    }

}


void MainWindow::onMnuQueryConsoleTriggered()
{

    m_query_console->exec();
}

