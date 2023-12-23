#include "dialogcertusercache.h"
#include "ui_dialogcertusercache.h"
#include "arcirk.hpp"
#include <QFloat16>
#include <QUrl>
#include "dialogselectintree.h"
#include "websocketclient.h"
#include <QStringListModel>
#include <QMap>
#include "dialogmstsc.h"
#include <QMessageBox>
#include "dialogmplitem.h"
#include <QVector>
#include <dialoginfo.h>
#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>
#include "commandline.h"
#include "dialogtask.h"
#include "taskparamdialog.h"
#include "facelib.h"

DialogCertUserCache::DialogCertUserCache(arcirk::database::cert_users& obj, TreeItemModel * users_model,
                                         const QString& def_url, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCertUserCache),
    object(obj),
    current_url(def_url)
{
    ui->setupUi(this);

    treeViewMpl = new TreeViewWidget(this);
    ui->verticalGeneral->addWidget(treeViewMpl);
    treeCertificates = new TreeViewWidget(this);
    ui->verticalLayoutCerts->addWidget(treeCertificates);
    treeContainers = new TreeViewWidget(this);
    ui->verticalLayoutConts->addWidget(treeContainers);
    treeAvailableCerts = new TreeViewWidget(this);
    ui->verticalLayoutAvCerts->addWidget(treeAvailableCerts);
    treeViewMstsc = new TreeViewWidget(this);
    treeViewMstsc->enable_sort(false);
    ui->verticalLayoutMstsc->addWidget(treeViewMstsc);
    treeTasks = new TreeViewWidget(this);
    treeTasks->setObjectName("treeTasks");
    treeTasks->enable_sort(false);
    ui->verticalLayoutTsacs->addWidget(treeTasks);

    column_aliases();

    cache = json::object();

    if(!object.cache.empty())
        cache= json::parse(object.cache);

    users_model_ = users_model;

    create_tasks_model();

    read_cache(cache);

    is_localhost_ = false;

    auto buttons = ui->buttonBox->buttons();
    foreach (auto btn, buttons) {
        if(btn->text() == "Cancel")
            btn->setText("Отмена");
    }

    connect(ui->btnCertInfo, &QToolButton::clicked, this, &DialogCertUserCache::onBtnCertInfoClicked);
    connect(ui->btnMplItemEdit, &QToolButton::clicked, this, &DialogCertUserCache::onBtnMplItemEditClicked);
    connect(ui->btnMplItemAdd, &QToolButton::clicked, this, &DialogCertUserCache::onBtnMplItemAddClicked);
    connect(ui->btnMstscMoveUp, &QToolButton::clicked, this, &DialogCertUserCache::onBtnMstscMoveUpClicked);
    connect(ui->btnParam, &QToolButton::clicked, this, &DialogCertUserCache::onBtnParamClicked);
    connect(ui->btnMplItemMoveDown, &QToolButton::clicked, this, &DialogCertUserCache::onBtnMplItemMoveDownClicked);
    connect(ui->btnMplItemMoveUp, &QToolButton::clicked, this, &DialogCertUserCache::onBtnMplItemMoveUpClicked);
    connect(ui->btnMplItemDelete, &QToolButton::clicked, this, &DialogCertUserCache::onBtnMplItemDeleteClicked);
    connect(ui->btnProfileCopy, &QToolButton::clicked, this, &DialogCertUserCache::onBtnProfileCopyClicked);
    connect(ui->btnCertDelete, &QToolButton::clicked, this, &DialogCertUserCache::onBtnCertDeleteClicked);
    connect(ui->btnMstscCopy, &QToolButton::clicked, this, &DialogCertUserCache::onBtnMstscCopyClicked);
    connect(ui->btnMstscMoveDown, &QToolButton::clicked, this, &DialogCertUserCache::onBtnMstscMoveDownClicked);
    connect(ui->btnMstscRemove, &QToolButton::clicked, this, &DialogCertUserCache::onBtnMstscRemoveClicked);
    connect(ui->btnMstscEdit, &QToolButton::clicked, this, &DialogCertUserCache::onBtnMstscEditClicked);
    connect(ui->btnMstscAdd, &QToolButton::clicked, this, &DialogCertUserCache::onBtnMstscAddClicked);
    connect(ui->btnMstsc, &QToolButton::clicked, this, &DialogCertUserCache::onBtnMstscClicked);
    connect(ui->btnSelectUser, &QToolButton::clicked, this, &DialogCertUserCache::onBtnSelectUserClicked);
    connect(ui->btnImportSettings, &QToolButton::clicked, this, &DialogCertUserCache::onBtnImportSettingsClicked);
    connect(ui->btnResetCertIlst, &QToolButton::clicked, this, &DialogCertUserCache::onBtnResetCertIlstClicked);
    connect(ui->btnCertAdd, &QToolButton::clicked, this, &DialogCertUserCache::onBtnCertAddClicked);
    connect(ui->btnSelectPathFirefox, &QToolButton::clicked, this, &DialogCertUserCache::onBtnSelectPathFirefoxClicked);
    connect(ui->btnPwdView, &QToolButton::toggled, this, &DialogCertUserCache::onBtnPwdViewToggled);
    connect(ui->btnPwdEdit, &QToolButton::toggled, this, &DialogCertUserCache::onBtnPwdEditToggled);
    connect(ui->radioRunAsAdmin, &QRadioButton::toggled, this, &DialogCertUserCache::onRadioRunAsAdminToggled);
    connect(ui->radioRunAs, &QRadioButton::toggled, this, &DialogCertUserCache::onRadioRunAsToggled);
    connect(treeViewMpl, &QTreeView::doubleClicked, this, &DialogCertUserCache::onTreeViewMplDoubleClicked);
    connect(treeViewMstsc, &QTreeView::doubleClicked, this, &DialogCertUserCache::onTreeViewMstscDoubleClicked);
    connect(ui->txtServer, &QLineEdit::editingFinished, this, &DialogCertUserCache::onTxtServerEditingFinished);
    connect(ui->txtServerUser, &QLineEdit::editingFinished, this, &DialogCertUserCache::onTxtServerUserEditingFinished);
    connect(ui->btnClear, &QToolButton::clicked, this, &DialogCertUserCache::onBtnClearClicked);
    connect(ui->tabCrypt, &QTabWidget::tabBarClicked, this, &DialogCertUserCache::onTabCryptTabBarClicked);
    connect(ui->btnInstallPlugin, &QToolButton::clicked, this, &DialogCertUserCache::onBtnInstallBpugin);
}

DialogCertUserCache::~DialogCertUserCache()
{
    delete ui;
}

void DialogCertUserCache::accept()
{

    if(ui->chkStandradPass->isChecked()){
        cli_param.hash = WebSocketClient::generateHash(cli_param.user_name.c_str(), cli_param.user_uuid.c_str()).toStdString();
    }else{
        if(ui->btnPwdEdit->isChecked()){
            cli_param.hash = WebSocketClient::generateHash(cli_param.user_name.c_str(), ui->lblPwd->text()).trimmed().toStdString();
        }
    }
    cache["client_param"] = pre::json::to_json(cli_param);
    cache["server_config"] = pre::json::to_json(srv_conf);

    cache["standard_password"] = ui->chkStandradPass->isChecked();
    cache["auto_connect"] = ui->chkAutoConnect->isChecked();

    cache["use_sid"] = ui->chUseSid->isChecked();
    cache["allow_profile_manager"] = ui->chkAllowProfileManager->isChecked();
    cache["allow_cryptopro"] = ui->chkAllowCryptoPro->isChecked();
    cache["allow_mstsc"] = ui->chkAllowMstsc->isChecked();
    cache["enable_task"] = ui->chkEnableTask->isChecked();

    write_mstsc_param();
    write_mpl_options();
    write_crypt_data();
    write_available_certificates();
    write_tasks_param(cache);

    cache["write_log"] = ui->chkWriteLog->isChecked();

    object.cache = cache.dump();

    QDialog::accept();
}


void DialogCertUserCache::editMstsc(const QString& host, const QString& usr, const QString& pwd, const QString& name)
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

void DialogCertUserCache::openMstsc(const QString &name)
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



void DialogCertUserCache::set_is_localhost(bool value)
{
    is_localhost_ = value;
    ui->btnMstsc->setEnabled(value);
}


void DialogCertUserCache::onTxtServerEditingFinished()
{
    QUrl url(ui->txtServer->text());
    srv_conf.ServerHost = url.host().toStdString();
    srv_conf.ServerPort = url.port();
    srv_conf.ServerSSL = url.scheme() == "wss" ? true : false;

}


void DialogCertUserCache::onTxtServerUserEditingFinished()
{
    cli_param.user_name = ui->txtServerUser->text().trimmed().toStdString();
}


void DialogCertUserCache::onBtnSelectUserClicked()
{
    if(!users_model_)
        return;

    auto dlg = DialogSelectInTree(users_model_, {"ref", "parent", "is_group", "deletion_mark"}, this);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto sel_object = dlg.selectedObject();
        cli_param.user_uuid = sel_object["ref"];
        ui->txtServerUser->setText(sel_object["first"].get<std::string>().c_str());
        onTxtServerUserEditingFinished();
    }

}


void DialogCertUserCache::onBtnPwdViewToggled(bool checked)
{
    auto echoMode = checked ? QLineEdit::Normal : QLineEdit::Password;
    QString image = checked ? ":/img/viewPwd.svg" : ":/img/viewPwd1.svg";
    auto btn = dynamic_cast<QToolButton*>(sender());
    btn->setIcon(QIcon(image));
    ui->lblPwd->setEchoMode(echoMode);
}


void DialogCertUserCache::onBtnPwdEditToggled(bool checked)
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
        onBtnPwdViewToggled(false);
        ui->btnPwdView->setChecked(false);
    }
}

void DialogCertUserCache::read_available_certificates(){
//    using namespace arcirk::database;
//    using json = nlohmann::json;
//    auto table = cache.value("available_certificates", json::object());
//    auto model = treeAvailableCerts->get_model(); //(TreeItemModel*)ui->treeAvailableCerts->model();
//    QVector<QString> m_order{
//        "first",
//        "subject",
//        "issuer",
//        "not_valid_before",
//        "not_valid_after",
//        "parent_user",
//        "private_key"
//    };
//    if(!model){
//        model = new TreeItemModel(this);
//        model->set_column_aliases(m_colAliases);
//        model->set_rows_icon(tree::item_icons_enum::Item, QIcon(":/img/cert16NoKey.png"));
//        auto sort_model = new TreeSortModel(this);
//        sort_model->setSourceModel(model);
//        ui->treeAvailableCerts->setModel(sort_model);
//    }
//    if(!table.empty()){
//        model->set_table(table);
//        model->set_columns_order(m_order);
//    }else{
//       // auto a_certs = table_default_json(tables::tbAvailableCertificates);
//        auto a_certs = pre::json::to_json(arcirk::database::certificates_view());
//        auto table = json::object();
//        auto columns = json::array();
//        auto rows = json::array();
//        for (auto itr = a_certs.items().begin(); itr != a_certs.items().end(); ++itr) {
//            columns += itr.key();
//        }
//        table["columns"] = columns;
//        table["rows"] = rows;
//        model->set_table(table);
//        model->set_columns_order(m_order);
//    }
//    ui->treeAvailableCerts->resizeColumnToContents(0);
//    auto cols = model->columnNames();
//    foreach (auto itr, cols) {
//        if(m_order.indexOf(itr) == -1){
//            auto ind = model->column_index(itr);
//            if(ind != -1)
//                ui->treeAvailableCerts->hideColumn(ind);
//        }
//    }
//    verify_certificate(model);
    using namespace arcirk::database;
    using json = nlohmann::json;
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

void DialogCertUserCache::write_available_certificates(){
    auto model = treeAvailableCerts->get_model();
    auto table = model->to_table_model(QModelIndex());
    cache["available_certificates"] = table;
}

void DialogCertUserCache::read_mstsc_param()
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

    mstsc_param = cache.value("mstsc_param", json::object());
    //auto model = new TreeItemModel(this);
    auto model = new ITree<arcirk::client::mstsc_options>(this);
    model->set_column_aliases(m_aliases);
    model->set_rows_icon(tree::item_icons_enum::Item, QIcon(":/img/mstsc.png"));
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

//        auto detailed_records = mstsc_param.value("detailed_records", json::array());
//        auto model_json = json::object();
//        auto columns = json::array();
//        auto tmp = pre::json::to_json(arcirk::client::mstsc_options());
//        for (auto itr = tmp.items().begin(); itr != tmp.items().end(); ++itr) {
//            columns += itr.key();
//        }
//        model_json["columns"] = columns;
//        model_json["rows"] = detailed_records;
//        model->set_table(model_json);
//        model->set_columns_order(m_order);
//        treeViewMstsc->setModel(model);
////        if(ui->treeViewMstsc->model()->columnCount() > 0)
////            ui->treeViewMstsc->resizeColumnToContents(0);
///
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
}

void DialogCertUserCache::write_mstsc_param()
{
    mstsc_param["enable_mstsc"] = ui->chkEnableMstsc->checkState() == Qt::CheckState::Checked;
    mstsc_param["enable_mstsc_users_sess"] = ui->chkEnableMstscUserSessions->checkState() == Qt::CheckState::Checked;
    auto mstsc_admin = ui->txtRuAsName->text().trimmed();
    mstsc_param["mstsc_admin"] = mstsc_admin.isEmpty() ? "admin" : mstsc_admin.toStdString();
    auto model = treeViewMstsc->get_model();
    if(model)
        mstsc_param["detailed_records"] = model->to_array(QModelIndex());

    cache["mstsc_param"] = mstsc_param;
}

void DialogCertUserCache::read_mpl_options()
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

    //auto model = new TreeItemModel(this);
    auto model = new ITree<mpl_item>(this);
    model->set_hierarchical_list(false);
    model->set_column_aliases(QMap<QString,QString>{
                                  qMakePair("name", "Наименование"),
                                  qMakePair("url", "Ссылка"),
                                  qMakePair("profile", "Профиль")
                              });
    //model->use_hierarchy("name");
    if(mpl_.mpl_list.size() > 0){
        auto table_str = arcirk::byte_array_to_string(mpl_.mpl_list);
        if(json::accept(table_str)){
            auto table_j = json::parse(table_str);
            if(table_j.is_object())
                model->set_table(table_j);
        }
        model->set_columns_order(QVector<QString>{"name", "profile", "url"});
    }
    treeViewMpl->enable_sort(false);
    treeViewMpl->setModel(model);
    treeViewMpl->hideColumn(model->column_index("ref"));
    treeViewMpl->hideColumn(model->column_index("parent"));
    update_mpl_items_icons();
}

void DialogCertUserCache::write_mpl_options()
{
    mpl_.use_firefox = ui->chUseFirefox->isChecked();
    mpl_.firefox_path = ui->txtFirefoxPath->text().toStdString();

    auto model = treeViewMpl->get_model();
    mpl_.mpl_list.clear();
    if(model){
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

    cache["mpl_options"] = pre::json::to_json(mpl_);
}

void DialogCertUserCache::read_crypt_data()
{
    if(crypt_data.cryptopro_path.empty()){
        crypt_data.cryptopro_path = "C:/Program Files (x86)/Crypto Pro/CSP";
    }
    ui->txtCryptoProPath->setText(crypt_data.cryptopro_path.c_str());
    onCertificates(crypt_data);
    onContainers(crypt_data);
}

void DialogCertUserCache::write_crypt_data()
{
    crypt_data.cryptopro_path = ui->txtCryptoProPath->text().toStdString();
    auto model = treeCertificates->get_model(); //(TreeItemModel*)ui->treeCertificates->model();
    crypt_data.certs.clear();
    if(model){
        auto table = model->to_table_model(QModelIndex());
        auto table_ba = arcirk::string_to_byte_array(table.dump());
        crypt_data.certs = ByteArray(table_ba.size());
        std::copy(table_ba.begin(), table_ba.end(), crypt_data.certs.begin());
    }

    model = treeContainers->get_model(); //(TreeItemModel*)ui->treeContainers->model();
    crypt_data.conts.clear();
    if(model){
        auto table = model->to_table_model(QModelIndex());
        auto table_ba = arcirk::string_to_byte_array(table.dump());
        crypt_data.conts = ByteArray(table_ba.size());
        std::copy(table_ba.begin(), table_ba.end(), crypt_data.conts.begin());
    }

    cache["crypt_data"] = pre::json::to_json(crypt_data);
}

template<typename T>
T DialogCertUserCache::getStructData(const std::string &name, const json &source)
{
    auto result = T();
    try {
        auto obg = source.value(name, json::object());
        if(!obg.empty())
            result = pre::json::from_json<T>(obg);
    } catch (const std::exception& e) {
       qCritical() <<  e.what();
    }

    return result;
}

void DialogCertUserCache::read_cache(const nlohmann::json &data)
{
    if(!data.empty()){
        cli_param = getStructData<arcirk::client::client_param>("client_param", data);
        srv_conf = getStructData<arcirk::server::server_config>("server_config", data);
        mpl_ = getStructData<arcirk::client::mpl_options>("mpl_options", data);
        crypt_data = getStructData<arcirk::client::cryptopro_data>("crypt_data", data);
    }else{
        cli_param = arcirk::client::client_param();
        srv_conf = arcirk::server::server_config();
        mpl_ = arcirk::client::mpl_options();
        crypt_data = arcirk::client::cryptopro_data();
    }

    if(!srv_conf.ServerHost.empty()){
        QString scheme = srv_conf.ServerSSL ? "wss" : "ws";
        QUrl url{};
        url.setHost(srv_conf.ServerHost.c_str());
        url.setPort(srv_conf.ServerPort);
        url.setScheme(scheme);
        ui->txtServer->setText(url.toString());
    }else{
        QUrl url(current_url);
        srv_conf.ServerSSL = url.scheme() == "wss" ? true : false;
        srv_conf.ServerHost = url.host().toStdString();
        srv_conf.ServerPort = url.port();
        ui->txtServer->setText(url.toString());
    }


    read_mstsc_param();

    ui->chkStandradPass->setCheckState(cache.value("standard_password", true) ? Qt::Checked : Qt::Unchecked);
    ui->chkAutoConnect->setCheckState(cache.value("auto_connect", true) ? Qt::Checked : Qt::Unchecked);
    ui->txtServerUser->setText(cli_param.user_name.c_str());
    ui->chUseSid->setChecked(cache.value("use_sid", true));
    ui->chkAllowCryptoPro->setChecked(cache.value("allow_cryptopro", false));
    ui->chkAllowMstsc->setChecked(cache.value("allow_mstsc", false));
    ui->chkAllowProfileManager->setChecked(cache.value("allow_profile_manager", false));
    ui->chkEnableTask->setChecked(cache.value("enable_task", false));

    read_mpl_options();

    read_crypt_data();

    read_available_certificates();

    form_control();

    auto m_write_log = cache.value("write_log", false);
    ui->chkWriteLog->setChecked(m_write_log);

    read_tasks_param();
}

void DialogCertUserCache::form_control()
{
    bool as_admin = ui->radioRunAsAdmin->isChecked();
    ui->txtRuAsName->setEnabled(!as_admin);
    ui->btnSetMstscPwd->setEnabled(!as_admin);

    update_mpl_items_icons();
}

void DialogCertUserCache::edit_row(const QModelIndex &current_index)
{
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
    connect(&dlg, &DialogMstsc::selectHost, this, &DialogCertUserCache::doSelectHosts);
    connect(this, &DialogCertUserCache::setSelectHosts, &dlg, &DialogMstsc::onSelectHost);
    connect(&dlg, &DialogMstsc::selectDatabaseUser, this, &DialogCertUserCache::doSelectDatabaseUser);
    connect(this, &DialogCertUserCache::setSelectDatabaseUser, &dlg, &DialogMstsc::onSelectDatabaseUser);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto row = pre::json::to_json(mstsc);
        //auto model = (TreeItemModel*)ui->treeViewMstsc->model();
        model->set_object(index, row);
    }
}

void DialogCertUserCache::update_mpl_items_icons()
{
    auto model = treeViewMpl->get_model();
    if(!model)
        return;

    for (int i = 0; i < model->rowCount(QModelIndex()); ++i) {
        auto index = model->index(i, 0, QModelIndex());
        auto object = model->to_object(index);
        auto row = pre::json::from_json<arcirk::client::mpl_item>(object);
        QString link = row.url.c_str();
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

    //ui->treeViewMpl->resizeColumnToContents(0);
}

void DialogCertUserCache::onCertUserCache(const QString &host, const QString &system_user, const nlohmann::json &data)
{
    Q_UNUSED(host);
    Q_UNUSED(system_user);
    cache = data;
    read_cache(cache);
}

void DialogCertUserCache::onMozillaProfiles()
{
    emit getMozillaProfiles(object.host.c_str(), object.system_user.c_str());
}

void DialogCertUserCache::doMozillaProfiles(const QStringList &lst)
{
    emit setMozillaProfiles(lst);
}

void DialogCertUserCache::onProfilesChanged(const QStringList &lst)
{
    m_moz_profiles = lst;
}

void DialogCertUserCache::onCertificates(const arcirk::client::cryptopro_data &data)
{
    //ui->treeCertificates->setModel(nullptr);

//    if(data.certs.size() > 0){
//        QVector<QString> m_order{
//            "first",
//            "subject",
//            "issuer",
//            "not_valid_before",
//            "not_valid_after",
//            "parent_user",
//            "sha1"
//        };
//        auto model = new TreeItemModel(this);
//        auto data_s = arcirk::byte_array_to_string(data.certs);
//        auto table = json::parse(data_s);
//        model->set_table(table);
//        model->set_columns_order(m_order);
//        model->set_rows_icon(tree::item_icons_enum::Item, QIcon(":/img/cert16NoKey.png"));
//        model->set_column_aliases(m_colAliases);
//        auto sort_model = new TreeSortModel(this);
//        sort_model->setSourceModel(model);
//        ui->treeCertificates->setModel(sort_model);
//        ui->treeCertificates->hideColumn(model->column_index("cache"));
//        ui->treeCertificates->hideColumn(model->column_index("second"));
//        ui->treeCertificates->resizeColumnToContents(0);
//        verify_certificate(model);
//    }
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
        auto table = json::parse(data_s);
        model->set_columns_order(m_order);
        model->set_rows_icon(tree::item_icons_enum::Item,  QIcon(":/img/cert16NoKey.png"));
        model->set_column_aliases(m_colAliases);

        model->set_table(table);
        //model->reset();
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

void DialogCertUserCache::onContainers(const arcirk::client::cryptopro_data &data)
{
//    ui->treeContainers->setModel(nullptr);

//    if(data.conts.size() > 0){
//        QVector<QString> m_order{
//            "name",
//            "volume",
//            "type"
//        };
//        auto model = new TreeItemModel(this);
//        auto data_s = arcirk::byte_array_to_string(data.conts);
//        auto table = json::parse(data_s);
//        model->set_table(table);
//        model->set_columns_order(m_order);
//        model->set_column_aliases(m_colAliases);
//        auto sort_model = new TreeSortModel(this);
//        sort_model->setSourceModel(model);
//        ui->treeContainers->setModel(sort_model);

//        using namespace arcirk::cryptography;
//        int ind = model->column_index("type");
//        auto parent = QModelIndex();
//        for (int i = 0; i < model->rowCount(parent); ++i) {
//            json vol = model->index(i, ind, parent).data(Qt::DisplayRole).toString().trimmed().toStdString();
//            auto volume = vol.get<TypeOfStorgare>();
//            if(volume == TypeOfStorgare::storgareTypeRegistry)
//                model->set_row_image(model->index(i,0,parent), QIcon(":/img/registry16.png"));
//            else if(volume == TypeOfStorgare::storgareTypeLocalVolume)
//                model->set_row_image(model->index(i,0,parent), QIcon(":/img/desktop.png"));
//        }

//        ui->treeContainers->resizeColumnToContents(0);
//    }
    if(data.conts.size() > 0){
        QVector<QString> m_order{
            "name",
            "volume",
            "type"
        };
        auto model = new TreeItemModel(this);
        auto data_s = arcirk::byte_array_to_string(data.conts);
        auto table = json::parse(data_s);
        model->set_hierarchical_list(false);
        model->set_columns_order(m_order);
        model->set_column_aliases(m_colAliases);
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

void DialogCertUserCache::onSelectHosts(const json &hosts)
{
    emit setSelectHosts(hosts);
}

void DialogCertUserCache::doSelectHosts()
{
    emit selectHosts();
}

void DialogCertUserCache::doSelectDatabaseUser()
{
    emit selectDatabaseUser();
}

void DialogCertUserCache::onBtnInstallBpugin()
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
    path /= QString(object.ref.c_str());
    path /= QString(obj.script_synonum.c_str());

    auto param = json::object({
        {"file_name", path.path().toStdString()}
    });

    emit doInstallPlugin(param, obj.uuid);
}

void DialogCertUserCache::onBtnInstallBpuginPrivate(const json &param, const std::string &ref)
{
    emit doInstallPlugin(param, ref);
}

void DialogCertUserCache::onSelectDatabaseUser(const json &user)
{
    emit setSelectDatabaseUser(user);
}

void DialogCertUserCache::onAvailableCertificates(const json &table)
{
    if(table.is_object()){
        auto model = treeAvailableCerts->get_model();
        model->clear();
        model->set_table(table);
    }
}

void DialogCertUserCache::onSelectCertificate(const json cert)
{
    qDebug() << __FUNCTION__;

    auto cert_ = arcirk::secure_serialization<arcirk::database::certificates_view>(cert, __FUNCTION__);
    auto model = treeAvailableCerts->get_model();
    if(!model){
        model = new TreeItemModel(this);
    }
    if(model->rowCount(QModelIndex()) == 0){
        auto table = arcirk::table_from_row(cert);
        model->set_table(table);
    }else
        model->add(cert);
}

void DialogCertUserCache::onEndInstallPlugin(const QString &file_name)
{
    emit doEndInstallPlugin(file_name);
}


void DialogCertUserCache::onRadioRunAsAdminToggled(bool checked)
{
    Q_UNUSED(checked);
    form_control();
}


void DialogCertUserCache::onRadioRunAsToggled(bool checked)
{
    Q_UNUSED(checked);
    form_control();
}


void DialogCertUserCache::onBtnMstscAddClicked()
{
    auto mstsc = arcirk::client::mstsc_options();
    auto dlg = DialogMstsc(mstsc, this);
    connect(&dlg, &DialogMstsc::selectHost, this, &DialogCertUserCache::doSelectHosts);
    connect(this, &DialogCertUserCache::setSelectHosts, &dlg, &DialogMstsc::onSelectHost);
    connect(&dlg, &DialogMstsc::selectDatabaseUser, this, &DialogCertUserCache::doSelectDatabaseUser);
    connect(this, &DialogCertUserCache::setSelectDatabaseUser, &dlg, &DialogMstsc::onSelectDatabaseUser);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto row = pre::json::to_json(mstsc);
        auto model = treeViewMstsc->get_model();
        model->add(row);
    }
}


void DialogCertUserCache::onBtnMstscEditClicked()
{
    edit_row();
}


void DialogCertUserCache::onTreeViewMstscDoubleClicked(const QModelIndex &index)
{
    auto index_ = treeViewMstsc->get_index(index);
    edit_row(index_);
}


void DialogCertUserCache::onBtnMstscMoveUpClicked()
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


void DialogCertUserCache::onBtnMstscMoveDownClicked()
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


void DialogCertUserCache::onBtnImportSettingsClicked()
{
    if(QMessageBox::question(this, "Импорт настроек", "Перезаполнить данные?") == QMessageBox::No)
        return;
    QUrl ws(ui->txtServer->text());
    Q_ASSERT(ws.isValid());
    emit getData(ws, object.host.c_str(), object.system_user.c_str(), this);
}

void DialogCertUserCache::onBtnMplItemAddClicked()
{
    auto row = arcirk::client::mpl_item();
    auto dlg = DialogMplItem(row, m_moz_profiles, this);
    connect(&dlg, &DialogMplItem::getMozillaProfiles, this, &DialogCertUserCache::onMozillaProfiles);
    connect(this, &DialogCertUserCache::setMozillaProfiles, &dlg, &DialogMplItem::mozillaProfiles);
    connect(&dlg, &DialogMplItem::profilesChanged, this, &DialogCertUserCache::onProfilesChanged);
    dlg.setWindowTitle("Новая запись");
    dlg.setModal(true);
    dlg.exec();

    if(dlg.result() == QDialog::Accepted){
        auto model = treeViewMpl->get_model();
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
//            auto columns = json::array();
//            auto rows = json::array();
//            rows += object;
//            for (auto it = object.items().begin(); it != object.items().end(); ++it) {
//                columns += it.key();
//            }
//            json table = json::object();
//            table["columns"]  = columns;
//            table["rows"] = rows;
            auto table = arcirk::table_from_row(object);
            model->set_table(table);
            model->set_columns_order(QVector<QString>{"name", "profile", "url"});
        }else
            model->add(object);
    }

    update_mpl_items_icons();
}


void DialogCertUserCache::onBtnMplItemEditClicked()
{
    auto model = treeViewMpl->get_model();
    if(!model)
        return;
    auto index = treeViewMpl->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }
    
    auto row = pre::json::from_json<arcirk::client::mpl_item>(model->to_object(index));
    auto dlg = DialogMplItem(row, m_moz_profiles, this);
    connect(&dlg, &DialogMplItem::getMozillaProfiles, this, &DialogCertUserCache::onMozillaProfiles);
    connect(this, &DialogCertUserCache::setMozillaProfiles, &dlg, &DialogMplItem::mozillaProfiles);
    connect(&dlg, &DialogMplItem::profilesChanged, this, &DialogCertUserCache::onProfilesChanged);
    dlg.setWindowTitle(row.name.c_str());
    dlg.setModal(true);
    dlg.exec();

    if(dlg.result() == QDialog::Accepted){
        //auto model = (TreeItemModel*)ui->treeViewMpl->model();
        auto object = pre::json::to_json(row);
        model->set_object(index, object);
    }

    update_mpl_items_icons();
}


void DialogCertUserCache::onBtnMplItemDeleteClicked()
{
    auto model = treeViewMpl->get_model();
    if(!model)
        return;
    auto index = treeViewMpl->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }
    
    auto row = pre::json::from_json<arcirk::client::mpl_item>(model->to_object(index));
    if(QMessageBox::question(this, "Удаление", QString("Удалить настройку '%1'").arg(row.name.c_str())) == QMessageBox::Yes){
        model->remove(index);
    }
}


void DialogCertUserCache::onBtnMplItemMoveUpClicked()
{
    auto index = treeViewMpl->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    if(index.row() < 1)
        return;

    auto model = treeViewMpl->get_model();
    model->move_up(index);
    update_mpl_items_icons();
}


void DialogCertUserCache::onBtnMplItemMoveDownClicked()
{
    auto index = treeViewMpl->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = treeViewMpl->get_model();

    if(index.row() >= model->rowCount(index.parent()) - 1)
        return;

    model->move_down(index);
    update_mpl_items_icons();
}


void DialogCertUserCache::onTreeViewMplDoubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    onBtnMplItemEditClicked();
}


void DialogCertUserCache::onBtnResetCertIlstClicked()
{
    auto currentTab = ui->tabCrypt->currentIndex();
    if(currentTab == 0)
        emit getCertificates(object.host.c_str(), object.system_user.c_str());
    else if(currentTab == 1)
        emit getContainers(object.host.c_str(), object.system_user.c_str());
//    else if(currentTab == 2)
//        emit getAvailableCertificates(object.ref.c_str());
}


void DialogCertUserCache::onBtnCertInfoClicked()
{
    auto currentTab = ui->tabCrypt->currentIndex();
    if(currentTab == 0){
        auto model = treeCertificates->get_model();// (TreeItemModel*)ui->treeCertificates->model();
        if(!model)
            return;
        auto index = treeCertificates->current_index(); //get_index(ui->treeCertificates->currentIndex(), ui->treeCertificates);
        if(!index.isValid()){
            QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
            return;
        }

        using json = nlohmann::json;


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
//        auto model = (TreeItemModel*)ui->treeContainers->model();
//        if(!model)
//            return;
//        auto index = ui->treeContainers->currentIndex();
//        if(!index.isValid()){
//            QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
//            return;
//        }
//        auto object = model->get_object(index);
//        QString vol = object.value("volume", "").c_str();
//        vol.append(object.value("name", "").c_str());
//        auto info = current_user->get_container_info(vol);
//        auto dlg = DialogInfo(info, object.value("name", "").c_str(), this);
//        dlg.setModal(true);
//        dlg.exec();
    }else if(currentTab == 2){
        auto model = treeAvailableCerts->get_model(); //(TreeItemModel*)ui->treeAvailableCerts->model();
        if(!model)
            return;
        auto index = treeAvailableCerts->current_index(); //get_index(ui->treeAvailableCerts->currentIndex(), ui->treeAvailableCerts);
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

void DialogCertUserCache::column_aliases()
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


void DialogCertUserCache::onBtnCertAddClicked()
{
    auto tab = ui->tabCrypt->currentIndex();
    if(tab == 0){

    }else if(tab == 1){

    }else if(tab == 2){
        emit selectCertificate();
    }
}


void DialogCertUserCache::onBtnCertDeleteClicked()
{
    auto tab = ui->tabCrypt->currentIndex();
    if(tab == 0){

    }else if(tab == 1){

    }else if(tab == 2){
        auto index = treeAvailableCerts->current_index();// get_index(ui->treeAvailableCerts->currentIndex(), ui->treeAvailableCerts);
        if(!index.isValid()){
            QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
            return;
        }

        if(QMessageBox::question(this, "Удаление", "Удалить выбранный сертификат?") == QMessageBox::No)
            return;

        auto model = treeAvailableCerts->get_model(); //(TreeItemModel*)ui->treeAvailableCerts->model();
        model->remove(index);

    }
}


void DialogCertUserCache::onBtnMstscRemoveClicked()
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


void DialogCertUserCache::onBtnMstscClicked()
{
    qDebug() << __FUNCTION__;

    auto table = treeViewMstsc;
    auto index = table->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = table->get_model();
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


void DialogCertUserCache::onBtnSelectPathFirefoxClicked()
{
    auto folder = QFileDialog::getExistingDirectory(this, "Выбор каталога", ui->txtFirefoxPath->text());
    if(!folder.isEmpty())
        ui->txtFirefoxPath->setText(folder);
}

QString DialogCertUserCache::cache_mstsc_directory()
{
    auto app_data = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    app_data.append("/mstsc");
    QDir f(app_data);
    if(!f.exists())
        f.mkpath(f.path());
    return f.path();
}

TreeItemModel *DialogCertUserCache::get_model(QTreeView *view)
{
    auto sort_model = (TreeSortModel*)view->model();
    //Q_ASSERT(sort_model != 0);
    if(sort_model == 0)
        return nullptr;
    auto model = (TreeItemModel*)sort_model->sourceModel();
    if(model == 0)
        return nullptr;
    //Q_ASSERT(model != 0);
    return model;
}

QModelIndex DialogCertUserCache::get_index(QModelIndex proxy_index, QTreeView *view)
{
    if(!proxy_index.isValid())
        return QModelIndex();
    auto sort_model = (TreeSortModel*)view->model();
    Q_ASSERT(sort_model != 0);
    auto model = (TreeItemModel*)sort_model->sourceModel();
    Q_ASSERT(model != 0);
    return model->find(proxy_index.column(), proxy_index.data().toString(), QModelIndex());
}

void DialogCertUserCache::onBtnMstscCopyClicked()
{
    auto table = treeViewMstsc;
    auto index = table->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = table->get_model();
    auto object = model->to_object(index);
    object["uuid"] = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();

    model->add(object);

}


void DialogCertUserCache::onBtnProfileCopyClicked()
{
    auto table = treeViewMpl;
    auto index = table->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = table->get_model();
    auto object = model->to_object(index);

    model->add(object);
}

void DialogCertUserCache::verify_certificate(TreeItemModel *model)
{
    for (int itr = 0; itr < model->rowCount(QModelIndex()); ++itr) {
        auto index = model->index(itr, 0, QModelIndex());
        auto object = model->to_object(index);
        auto dt_str = object.value("not_valid_after", "");
        if(!dt_str.empty()){           
            auto dt = QDateTime::fromString(dt_str.c_str(), "dd.MM.yyyy  hh:mm:ss");
            //qDebug() << dt;
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


void DialogCertUserCache::onTasksButtonClick()
{
    auto btn = qobject_cast<QToolButton*>(sender());
    Q_ASSERT(btn!=0);

    if(btn->objectName() == "btnTaskAdd"){
        auto task = services::task_options();
        task.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        task.name = "Новая задача 1";

        auto dlg = DialogTask(task, this);
        connect(&dlg, &DialogTask::doInstallPlugin, this, &DialogCertUserCache::onBtnInstallBpuginPrivate);
        connect(this, &DialogCertUserCache::doEndInstallPlugin, &dlg, &DialogTask::onEndInstallPlugin);

        if(dlg.exec() == QDialog::Accepted){
            auto model = (ITree<services::task_options>*)treeTasks->get_model();
            model->add_struct(task);
            if(!dlg.currentScript().isEmpty()){
                emit doSavePluginFile(object.ref.c_str(), dlg.currentScript());
            }
        }
    }else if(btn->objectName() == "btnTaskEdit"){
        auto index = treeTasks->current_index();
        if(!index.isValid()) return;
        auto model = (ITree<services::task_options>*)treeTasks->get_model();
        auto task = model->object(index);
        auto dlg = DialogTask(task, this);
        connect(&dlg, &DialogTask::doInstallPlugin, this, &DialogCertUserCache::onBtnInstallBpuginPrivate);
        connect(this, &DialogCertUserCache::doEndInstallPlugin, &dlg, &DialogTask::onEndInstallPlugin);

        if(dlg.exec() == QDialog::Accepted){
            model->set_struct(task, index);
            if(!dlg.currentScript().isEmpty()){
                emit doSavePluginFile(object.ref.c_str(), dlg.currentScript());
            }
        }
    }
}
void DialogCertUserCache::read_tasks_param()
{
    auto tasks_param = cache.value("tasks_param", json::array());
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

}

void DialogCertUserCache::write_tasks_param(nlohmann::json &data)
{
    auto model = (ITreeTasksModel*)treeTasks->get_model();
    data["tasks_param"] = model->to_array(QModelIndex());
}

void DialogCertUserCache::create_tasks_model()
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
    });
    model->set_enable_rows_icons(false);
    model->set_columns_order(QList<QString>{"allowed", "synonum", "start_task", "end_task", "interval"});
    //    //model->set_widget("allowed", item_editor_widget_roles::widgetCheckBoxRole);
    //    auto delegate = new TreeItemDelegate(true, ui->treeTasks);
    //    ui->treeTasks->setItemDelegate(delegate);
    //    ui->treeTasks->setEditTriggers(QAbstractItemView::AllEditTriggers);
    //    ui->treeTasks->setIndentation(0);
    //    model->reset();
    treeTasks->setModel(model);

    connect(ui->btnTaskAdd, &QToolButton::clicked, this, &DialogCertUserCache::onTasksButtonClick);
    connect(ui->btnTaskEdit, &QToolButton::clicked, this, &DialogCertUserCache::onTasksButtonClick);
    connect(ui->btnTaskDelete, &QToolButton::clicked, this, &DialogCertUserCache::onTasksButtonClick);
    connect(treeTasks, &QTreeView::doubleClicked, this, &DialogCertUserCache::onTreeTaskDoubleClicked);
    connect(ui->btnTaskStart, &QToolButton::clicked, this, &DialogCertUserCache::onBtnTaskStartClicked);
    connect(ui->btnTaskDelete, &QToolButton::clicked, this, &DialogCertUserCache::onBtnTaskDeleteClicked);

    auto header = treeTasks->header();
    header->resizeSection(0, 20);
    header->setSectionResizeMode(0, QHeaderView::Fixed);
}

json DialogCertUserCache::plugin_param(const QString &plugin_path)
{
    //    try {
    //        QPluginLoader loader(plugin_path);

    //    } catch (...) {
    //    }
    return {};
}

void DialogCertUserCache::onBtnTaskStartClicked()
{
    //    auto index = treeTasks->current_index();
    //    if(!index.isValid()){
    //        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
    //        return;
    //    }

    //    auto model = (ITreeTasksModel*)treeTasks->get_model();
    //    auto object = model->object(index);
    //    if(object.type_script == 1){
    //        QString plugin_name = object.script.c_str();
    //        if(plugin_name.isEmpty()){
    //            QMessageBox::critical(this, "Ошибка", "Не указан файл плагина!");
    //            return;
    //        }

    //        QFile f(plugin_name);
    //        if(!f.exists()){
    //            QMessageBox::critical(this, "Ошибка", "Файл плагина не найден!");
    //            return;
    //        }
    //        QPluginLoader loader(f.fileName());
    //        if( loader.load() ) {
    //            qDebug() << "Плагин загружен";
    //            QObject *obj = loader.instance();
    //            IPlugin* plugin
    //                = qobject_cast<IPlugin*>(obj);
    //            if(plugin){
    //                json param = json::object();
    //                param["destantion"] = "\\\\192.168.10.11\\обмен\\Отдел ИТ\\Борисоглебский\\temp\\";
    //                param["host"] = m_client->http_url().toString().toStdString();
    //                param["token"] = m_client->token().toStdString();
    //                plugin->setParam(param.dump().c_str());
    //                if(plugin->isValid()){
    //                    //auto res = plugin->accept();
    //                }
    ////                auto res = pluginIF->readData();
    ////                if(res.size() > 0){
    ////                    auto r = param_bank();
    ////                    r.fromRaw(res);
    ////                    const void* poTemp = (const void*)res->constData();
    ////                    const param_bank* poStruct = static_cast< const param_bank* >(poTemp);
    ////                    qDebug() << poStruct->fiels.size();
    ////                    foreach (auto itr, poStruct->fiels) {
    ////                        qDebug() << itr.size();
    ////                    }
    ////                }
    //            }
    //        }else{
    //            qDebug() << "Ошибка загрузки плагина" << loader.errorString();
    //        }
    //        loader.unload();
    //    }

}


void DialogCertUserCache::onBtnTaskDeleteClicked()
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

void DialogCertUserCache::onBtnParamClicked()
{

    auto index = treeTasks->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = (ITree<services::task_options>*)treeTasks->get_model();
    auto task_data_ = model->object(index);
    if(task_data_.type_script == 1){
        if(task_data_.script_synonum.empty())
            return;
        QPath p(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        p /= "plugins";
        p /= QString(task_data_.uuid.c_str());

        QDir dir(p.path());
        if(!dir.exists())
            dir.mkpath(p.path());

        p /= task_data_.script_synonum.c_str();

        if(!p.exists()){
            if(QMessageBox::question(this, "Параметры плагина", "Для настройки параметров требуется установить плагин на текущий компьютер. Продолжить?") == QMessageBox::No)
                return;
            QPath path(QString("html\\client_data\\plugins"));
            path /= QString(task_data_.uuid.c_str());
            path /= task_data_.script_synonum.c_str();

            auto param = json::object({
                {"file_name", path.path().toStdString()}
            });

            emit doInstallPlugin(param, task_data_.uuid);
        }else{
            using namespace arcirk::plugins;
            try {
                auto loader = new QPluginLoader(p.path(), this);
                QObject *obj = loader->instance();
                IAIPlugin* plugin
                    = qobject_cast<IAIPlugin*>(obj);
                if(plugin){
                    if(task_data_.param.size() > 0){
                        plugin->setParam(arcirk::byte_array_to_string(task_data_.param).c_str());
                    }
                    if(plugin->editParam(this)){
                        auto param_t = plugin->param();
                        task_data_.param = ByteArray(param_t.size());
                        std::copy(param_t.begin(), param_t.end(), task_data_.param.begin());
                    }
                    loader->unload();
                }
                delete loader;
            } catch (const std::exception& e) {
                qCritical() << e.what();
            }

        }
    }
}

void DialogCertUserCache::onTreeTaskDoubleClicked(const QModelIndex &index)
{
    auto tree = qobject_cast<TreeViewWidget*>(sender());
    Q_ASSERT(tree!=0);

    if(tree->objectName() == "treeTasks"){
        auto index_ = treeTasks->get_index(index);
        if(!index_.isValid()) return;
        auto model = (ITree<services::task_options>*)treeTasks->get_model();
        auto task = model->object(index_);
        auto dlg = DialogTask(task, this);
        connect(&dlg, &DialogTask::doInstallPlugin, this, &DialogCertUserCache::onBtnInstallBpuginPrivate);
        connect(this, &DialogCertUserCache::doEndInstallPlugin, &dlg, &DialogTask::onEndInstallPlugin);
        if(dlg.exec() == QDialog::Accepted){
            model->set_struct(task, index_);
            if(!dlg.currentScript().isEmpty()){
                emit doSavePluginFile(object.ref.c_str(), dlg.currentScript());
            }
        }
    }

}


void DialogCertUserCache::onBtnClearClicked()
{
    if(QMessageBox::question(this, "Удаление", "Удалить все записи?") == QMessageBox::No)
        return;
    auto model = treeAvailableCerts->get_model();
    model->clear();
}


void DialogCertUserCache::onTabCryptTabBarClicked(int index)
{
    ui->btnClear->setEnabled(index == 2);
}

