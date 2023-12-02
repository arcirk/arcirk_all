#ifndef DIALOGMAIN_H
#define DIALOGMAIN_H

#include "global/arcirk_qt.hpp"
//#include "shared_struct_qt.hpp"
#include <QDialog>
#include "websocketclient.h"
#include <QLabel>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QTimer>


//#include <QStandardItemModel>

#include "certuser.h"

#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QCloseEvent>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDataStream>

//#include "treeviewmodel.h"
#include "tree_model.h"
#include "gui/treeviewwidget.h"
#include "iface/iface.hpp"

#define BANK_CLIENT_FILE "sslgate.url"
#define BANK_CLIENT_USB_KEY "BankUsbKey"

QT_BEGIN_NAMESPACE
namespace Ui { class DialogMain; }
QT_END_NAMESPACE

using json = nlohmann::json;
using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;

typedef ITree<services::task_options> ITreeTasksModel;

struct param_bank
{
    param_bank() {}

    QList<QByteArray> fields;
    QList<QString> names;

    void fromRaw(const QByteArray& data){
        if(data.size() == 0)
            return;
        QDataStream stream {data};
        stream >> fields;
        stream >> names;

    }

};

class DialogMain : public QDialog
{
    Q_OBJECT

public:
    DialogMain(QWidget *parent = nullptr);
    ~DialogMain();

    void accept() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::DialogMain *ui;
    IWClient *      m_client;
    QLabel *               infoBar;
    QMap<QString, QString> m_colAliases;
    CertUser *             current_user;
    QString                current_url;

    QProcess * mozillaApp;

    json mstsc_param;
    arcirk::client::mpl_options mpl_;
    arcirk::client::cryptopro_data crypt_data;
    QStringList m_moz_profiles{};
    bool is_localhost_;

    QSystemTrayIcon *trayIcon;
    QMenu           *trayIconMenu;
    QAction         *quitAction;
    QAction         *showAction;
    QAction         *checkIpAction;
    QAction         *openFiefox;
    QAction         *installCertAction;

    QStringList     m_local_addresses;

    bool            m_write_log;
    bool            m_show_alerts;

    TreeViewWidget * treeViewMpl;
    TreeViewWidget * treeCertificates;
    TreeViewWidget * treeContainers;
    TreeViewWidget * treeViewMstsc;
    TreeViewWidget * treeAvailableCerts;
    TreeViewWidget * treeTasks;

    void column_aliases();
    void reconnect();
    void displayError(const QString& what, const QString& err);
    void connectionSuccess(); //при успешной авторизации
    void connectionChanged(bool state);

    QModelIndex findInTable(QAbstractItemModel * model, const QString& value, int column, bool findData = true);

    void createTrayActions();
    void createTrayIcon();
    void createDynamicMenu();

    void read_data_chche();
    void read_cache(const nlohmann::json &data);
    void read_mstsc_param();
    void write_mstsc_param(nlohmann::json &data);
    void read_mpl_options();
    void write_mpl_options(nlohmann::json &data);
    void read_crypt_data();
    void write_crypt_data(nlohmann::json &data);
    void read_available_certificates();
    void write_available_certificates(nlohmann::json &data);
    void read_tasks_param();
    void write_tasks_param(nlohmann::json &data);

    void write_cache();

    QIcon get_link_icon(const QString &link);
    void run_mstsc_link(const arcirk::client::mstsc_options &opt);

    template<typename T>
    T getStructData(const std::string &name, const json &source);

    void form_control();
    void edit_row(const QModelIndex &current_index = QModelIndex());

    void reset_data();
    void update_mpl_items_icons();

    void set_address();
    void editMstsc(const QString& host, const QString& usr, const QString& pwd, const QString& name);
    void openMstsc(const QString &name);

    void update_cache_on_server();
    void update_rdp_files(const nlohmann::json &items);
    QString cache_mstsc_directory();
    QString rdp_file_text();

    void send_notify(const std::string& command, const std::string& message, const nlohmann::json& param);

    void save_cache_to_local();

    void open_user_mstsc_session(const json result);

    bool delete_certificate(const std::string& sha1);
    bool delete_container(const std::string& name);

    void install_certificate(const std::string& sha1, const std::string& cnt);

    void verify_certificate(TreeItemModel* model);

    void write_temp_log(const QString& fun, const QString& message){
        if(!m_write_log)
            return;
        auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        auto fileName= path + "/fun.log";
        QFile f_log(fileName);
        f_log.open(QIODevice::Append | QIODevice::Text);
        auto msg = fun;
        msg.append(" ");
        msg.append(message);
        msg.append("\n");
        f_log.write(msg.toUtf8());
        f_log.close();
    }

    json get_local_users_details();

    void create_tasks_model();


    //////
    json plugin_param(const QString& plugin_path);

signals:
    void setConnectionChanged(bool state);

    void getData(const QUrl &ws, const QString& host, const QString& system_user, QWidget *parent);
    void getMozillaProfiles(const QString& host, const QString& system_user);
    void setMozillaProfiles(const QStringList& lst);
    void selectHosts();
    void setSelectHosts(const json& hosts);
    void selectDatabaseUser();
    void setSelectDatabaseUser(const json& user);

public slots:
    void openConnection();
    void closeConnection();
    void serverResponse(const arcirk::server::server_response& message);
    void onCommandToClient(const arcirk::server::server_response& message);

private slots:
    void onTrayTriggered();
    void onTrayMstscConnectToSessionTriggered();
    void onAppExit();
    void onWindowShow();
    void trayMessageClicked();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void trayShowMessage(const QString& msg, int isError = false);
    void onInstallCertificate();
    void openMozillaFirefox();
    void onCheckIP();
    void onUserInfo(const json& info);
    void onCertUserCache(const QString& host, const QString& system_user, const nlohmann::json& data);
    void onProfilesChanged(const QStringList& lst);
    void onCertificates(const arcirk::client::cryptopro_data& crypt_data);
    void onContainers(const arcirk::client::cryptopro_data& crypt_data);
    void onSelectHosts(const json& hosts);
    void doSelectHosts();
    void doSelectDatabaseUser();
    void onSelectDatabaseUser(const json& user);
    void onTasksButtonClick();
    void onTxtServerEditingFinished();
    void onTxtServerUserEditingFinished();
    void onBtnSelectUserClicked();
    void onBtnPwdViewToggled(bool checked);
    void onBtnPwdEditToggled(bool checked);
    void onRadioRunAsAdminToggled(bool checked);
    void onRadioRunAsToggled(bool checked);
    void onBtnMstscAddClicked();
    void onBtnMstscEditClicked();
    void onTreeViewMstscDoubleClicked(const QModelIndex &index);
    void onBtnMstscMoveUpClicked();
    void onBtnMstscMoveDownClicked();
    void onBtnImportSettingsClicked();
    void onBtnMplItemAddClicked();
    void onBtnMplItemEditClicked();
    void onBtnMplItemDeleteClicked();
    void onBtnMplItemMoveUpClicked();
    void onBtnMplItemMoveDownClicked();
    void onTreeViewMplDoubleClicked(const QModelIndex &index);
    void onBtnResetCertIlstClicked();
    void onBtnCertInfoClicked();
    void onBtnCertAddClicked();
    void onBtnCertDeleteClicked();
    void onBtnMstscRemoveClicked();
    void onChUseSidToggled(bool checked);
    void onButtonBoxAccepted();
    void onButtonBoxRejected();
    void onBtnMstscClicked();
    void onBtnSelectPathFirefoxClicked();
    void onTabCryptTabBarClicked(int index);
    void onBtnProfileCopyClicked();
    void onBtnMstscCopyClicked();
    void onChkWriteLogToggled(bool checked);
    void onBtnTaskStartClicked();
    void onBtnTaskDeleteClicked();
    void onBtnParamClicked();
    void onTreeTaskDoubleClicked(const QModelIndex &index);
};
#endif // DIALOGMAIN_H
