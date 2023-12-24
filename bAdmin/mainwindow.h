#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "websocketclient.h"
#include <QLabel>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QTimer>


#include <QStandardItemModel>
//#include "shared_struct.hpp"

#include "global/arcirk_qt.hpp"
#include "gui/treeviewwidget.h"

#include "certuser.h"
#include "cryptcontainer.h"

#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QCloseEvent>

#include "cryptcertificate.h"
#include "cryptcontainer.h"
#include "dialogmessageslist.h"
#include "dialogqueryconsole.h"
#include "dialogsetversion.h"

using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;
using namespace arcirk::query_builder_ui;

#define BANK_CLIENT_FILE "sslgate.url"
#define BANK_CLIENT_USB_KEY "BankUsbKey"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onTrayTriggered();
    void onAppExit();
    void onWindowShow();
    void trayMessageClicked();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void trayShowMessage(const QString& msg, int isError = false);
    void onOpenTreeModelSettingsDialog(TreeItemModel* model);
    void onMnuConnectTriggered();
    void onMnuExitTriggered();
    void onMnuDisconnectTriggered();
    void onTreereeWidgetItemClicked(QTreeWidgetItem *item, int column);
    void onBtnEditClicked();
    void onTreeViewDoubleClicked(const QModelIndex &index);
    void onBtnDataImportClicked();
    void onBtnAddClicked();
    void onBtnDeleteClicked();
    void onBtnSetLinkDeviceClicked();
    void onMnuAboutTriggered();
    void onMnuOptionsTriggered();
    void onBtnTaskRestartClicked();
    void onBtnStartTaskClicked();
    void onBtnSendClientReleaseClicked();
    void onBtnSendoToClientClicked(); //
    void onBtnInfoClicked();
    void onBtnAddGroupClicked();
    void onBtnRegistryDeviceClicked();
    void onBtnRegistryUserClicked();
    void onBtnEditCacheClicked();
    void onBtnSystemUsersClicked();
    void onBtnRefreshClicked();
    void onMnuUpdatePriceCheckerTriggered();
    void onBtnMoveToClicked();
    void onMnuMessanegerTriggered();
    void onMnuQueryConsoleTriggered();

    void reconnect();
    void displayError(const QString& what, const QString& err);
    void connectionSuccess(); //при успешной авторизации
    void connectionChanged(bool state);

    void onSavePluginFile(const QString& uuidUser, const QString& fileName);
    void onAddTreeItem(const QModelIndex& index, const json& data);
    void onInstallPlugin(const json& param, const std::string& ref);

private:
    Ui::MainWindow *                         ui;
    WebSocketClient *                        m_client;
    QLabel *                                 infoBar;
    QLabel *                                 infoIco;
    QMap<arcirk::server::server_objects,
    TreeItemModel*>                          m_models;
    QMap<QString, QString>                   m_colAliases;
    QMap<arcirk::server::application_names,
    QPair<QIcon, QIcon>>                     app_icons;
    CertUser *                               current_user;
    bool                                     use_firefox;
    std::string                              firefox_path;

    QTreeWidgetItem *                        root_item;
    TreeViewWidget*                          treeView;

    DialogMessagesList* m_messanager;
    DialogQueryConsole* m_query_console;

    QSystemTrayIcon *trayIcon;
    QMenu           *trayIconMenu;
    QAction         *quitAction;
    QAction         *showAction;

    QProcess * mozillaApp;

    TreeItemModel* get_model();
    QModelIndex get_index(const QModelIndex& proxy_index);

    QVector<arcirk::server::server_objects> m_hierarhy_objects{
        arcirk::server::server_objects::ProfileDirectory,
        arcirk::server::server_objects::CertUsers,
        arcirk::server::server_objects::DatabaseUsers
    };
    QMap<arcirk::server::server_objects,
    QVector<QString>>   m_order_columns{
        qMakePair(arcirk::server::server_objects::Certificates, QVector<QString>{
                    "first", "subject", "issuer", "not_valid_before", "not_valid_after", "private_key", "parent_user"
                }),
        qMakePair(arcirk::server::server_objects::LocalhostUserCertificates, QVector<QString>{
                    "first", "subject", "issuer", "not_valid_before", "not_valid_after", "private_key", "parent_user"
                }),
        qMakePair(arcirk::server::server_objects::Containers, QVector<QString>{
                    "first", "subject", "issuer", "not_valid_before", "not_valid_after", "parent_user"
                }),
        qMakePair(arcirk::server::server_objects::OnlineUsers, QVector<QString>{
                    "app_name", "host_name", "address", "user_name", "role", "start_date", "session_uuid", "user_uuid", "device_id"
                }),
        qMakePair(arcirk::server::server_objects::Services, QVector<QString>{
                    "synonum", "name", "allowed", "start_task", "end_task", "interval"
                }),
        qMakePair(arcirk::server::server_objects::Devices, QVector<QString>{
                "device_type", "first", "second", "address", "organization", "subdivision", "warehouse", "price"
            }),
        qMakePair(arcirk::server::server_objects::DatabaseUsers, QVector<QString>{
               "first","second","ref","parent", "is_group", "deletion_mark"
            }),
    };

    void createModels();
    void createColumnAliases();
    bool openConnectionDialog();

    void formControl();

    void write_conf();

    QTreeWidgetItem * addTreeNode(const QString &text, const QVariant &key, const QString &imagePath);
    QTreeWidgetItem * findTreeItem(const QString& key);
    QTreeWidgetItem * findTreeItem(const QString& key, QTreeWidgetItem * parent);
    QModelIndex findInTable(QAbstractItemModel * model, const QString& value, int column, bool findData = true);

    void tableSetModel(const QString& key);
    void tableResetModel(arcirk::server::server_objects key, const QByteArray& resp = "");
    void modelSetTable(arcirk::server::server_objects key, const nlohmann::json& data);

    void resetModel(arcirk::server::server_objects key, const json& table);
    void resetUI(arcirk::server::server_objects key);
    server::server_objects serverObject(const std::string& value){
        json k = value;
        auto key = k.get<server::server_objects>();
        return key;
    }

    json get_table(const std::string& resp){
        auto str = QByteArray::fromBase64(resp.data());
        return json::parse(str);
    }

    json get_server_conf_table(const std::string& resp){
        auto srv_conf = json::parse(QByteArray::fromBase64(resp.data()).toStdString());
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
        return object;
    }

    void createTrayActions();
    void createTrayIcon();
    void createDynamicMenu();

    void fillDefaultTree();

    void get_online_users();
    void get_online_users_synh();

    void edit_cert_user(const QModelIndex &index);
    void add_cert_user(const arcirk::database::cert_users& parent,
                       const QString& host = "",
                       const QString& name = "",
                       const QString& uuid = "",
                       const QString& system_user = "",
                       const QString& sid = "");
    bool is_cert_user_exists(const QString& host, const QString& system_user, nlohmann::json& object);
    QModelIndex is_user_online(const QString& host, const QString& system_user, const QString& app_name = "");

    void setVisible(bool visible) override;

    QIcon get_link_icon(const QString& link);

    void run_mstsc_link(const client::mstsc_options& opt);
    QString rdp_file_text();
    QString cache_mstsc_directory();
    void update_rdp_files(const nlohmann::json& data);

    void database_get_containers_synch();
    void database_get_containers_asynch();
    void database_get_certificates_asynch();
    void database_get_certificates_synch();
    void database_get_deviceview_asynch();

    std::string database_set_certificate(CryptCertificate& cert, const QString& file_name);
    void database_insert_certificate();
    void localhost_cert_to_database(const QString& sha1);
    void insert_container(CryptContainer& cnt);

    void set_enable_form(bool value);
    void verify_certificate(TreeItemModel *model);
    void update_icons(server::server_objects key, TreeItemModel* model);
    void update_columns();
    void update_toolbar(const server::server_objects &key);
    void set_enabled_buttons(const QVector<QToolButton*> btns, bool value){
        foreach (auto btn, btns) {
            btn->setEnabled(value);
        }
    }
    void sortByColumn(QTreeView * tree = nullptr, int column = 0, Qt::SortOrder ord = Qt::SortOrder::AscendingOrder);
    void setIndentation(server::server_objects key);

    json get_local_users_details(const std::string& host);

    void load_table_demo(database::tables table);

signals:
    void setConnectionChanged(bool state);
    void certUserData(const QString& host, const QString& system_user, const nlohmann::json& data);
    void mozillaProfiles(const QStringList& lst);
    void certUserCertificates(const arcirk::client::cryptopro_data& data);
    void certUserContainers(const arcirk::client::cryptopro_data& data);
    void selectHosts(const nlohmann::json& hosts);
    void selectDatabaseUser(const nlohmann::json &user);
    void availableCertificates(const json& table);
    void selectCertificate(const json& cert);

    //messages
    void readMessages(const arcirk::server::server_response& message);
    void userMessage(const arcirk::server::server_response &message);

    void clientConnectionSuccess(const QUuid &sess, const QUuid &uuid);

    void selectGetLocalUsers(const nlohmann::json& result);
    void selectRemoteSystemUser(const win_user_info& result);

    void clientLeave(const arcirk::client::session_info& info);
    void clientJoin(const arcirk::client::session_info& info);

    void openTreeModelSettingsDialog(TreeItemModel* model);

    void openTableViewDialog(database::tables table);
    void doEndInstallPlugin(const QString& file_name);

public slots:
    void openConnection();
    void closeConnection();
    void serverResponse(const arcirk::server::server_response& message);

    void onCertUserCache(const QUrl &ws, const QString& host, const QString& system_user, QWidget *parent);
    void onMozillaProfiles(const QString& host, const QString& system_user);

    void doGetCertUserCertificates(const QString& host, const QString& system_user);
    void doGetCertUserContainers(const QString& host, const QString& system_user);

    void doSelectHosts();
    void doSelectDatabaseUser();

    void onTreeFetch();
    void onCanFetch(const QModelIndex &parent) const;

    void wsError(const QString& what, const QString& command, const QString& err);

    //void doAvailableCertificates(const QString& user_uuid);
    void onSelectCertificate();
    void onCommandToClient(const arcirk::server::server_response &message);

    //messages
    void doGetMessages(const json& param);
    void onSendMessage(const QUuid& recipient, const QString& message, const json& param);
    void doUserMessage(const arcirk::server::server_response &message);
    void onSelectGetLocalUsers(const nlohmann::json& result);

    void onSelectRemoteUser(const QString& host);

    void onOpenTableViewDialog(database::tables table);

    void onDownloadAnydesk();
    void onDownloadAnydeskComlete(const QString& file);

    void onInstallPrivatePlugin(const QString& file_name, const QString& task_ref);

};
#endif // MAINWINDOW_H
