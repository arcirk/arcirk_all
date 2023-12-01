#ifndef DIALOGCERTUSERCACHE_H
#define DIALOGCERTUSERCACHE_H

#include <QDialog>
#include "shared_struct.hpp"
#include "tree_model.h"
#include "gui/treeviewwidget.h"
#include <QTreeView>
#include "iface/iface.hpp"

using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;

typedef ITree<arcirk::services::task_options> ITreeTasksModel;


namespace Ui {
class DialogCertUserCache;
}

using json = nlohmann::json;

class DialogCertUserCache : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCertUserCache(arcirk::database::cert_users& obj, TreeItemModel * users_model,
                                 const QString& def_url = "ws://localhost:8080",
                                 QWidget *parent = nullptr);
    ~DialogCertUserCache();

    void accept() override;

    void set_is_localhost(bool value);

private slots:

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
    void onBtnMstscClicked();
    void onBtnSelectPathFirefoxClicked();
    void onBtnMstscCopyClicked();
    void onBtnProfileCopyClicked();
    void onBtnTaskStartClicked();
    void onBtnTaskDeleteClicked();
    void onBtnParamClicked();
    void onTreeTaskDoubleClicked(const QModelIndex &index);
    void onTasksButtonClick();
    void onBtnClearClicked();
    void onTabCryptTabBarClicked(int index);

private:
    Ui::DialogCertUserCache *ui;
    arcirk::database::cert_users& object;
    json mstsc_param;
    arcirk::client::client_param cli_param;
    arcirk::server::server_config srv_conf;
    arcirk::client::mpl_options mpl_;
    arcirk::client::cryptopro_data crypt_data;
    json cache;
    TreeItemModel * users_model_;
    QString current_url;
    QStringList m_moz_profiles{};
    QMap<QString, QString> m_colAliases;

    TreeViewWidget * treeViewMpl;
    TreeViewWidget * treeCertificates;
    TreeViewWidget * treeContainers;
    TreeViewWidget * treeViewMstsc;
    TreeViewWidget * treeAvailableCerts;
    TreeViewWidget * treeTasks;


    bool is_localhost_;

    void column_aliases();

    void read_mstsc_param();
    void write_mstsc_param();

    void read_mpl_options();
    void write_mpl_options();

    void read_crypt_data();
    void write_crypt_data();

    void read_cache(const nlohmann::json& data);

    void form_control();
    void edit_row(const QModelIndex &current_index = QModelIndex());

    void reset_data();

    void update_mpl_items_icons();

    void read_available_certificates();
    void write_available_certificates();

    void read_tasks_param();
    void write_tasks_param(nlohmann::json &data);

    template<typename T>
    T getStructData(const std::string& name, const json& source);

    void editMstsc(const QString& host, const QString& usr, const QString& pwd, const QString& name);
    void openMstsc(const QString &name);
    QString cache_mstsc_directory();

    TreeItemModel* get_model(QTreeView * view);
    QModelIndex get_index(QModelIndex proxy_index, QTreeView *view);
    void verify_certificate(TreeItemModel *model);
    //////
    json plugin_param(const QString& plugin_path);
    void create_tasks_model();
signals:
    void getData(const QUrl &ws, const QString& host, const QString& system_user, QWidget *parent);
    void getMozillaProfiles(const QString& host, const QString& system_user);
    void setMozillaProfiles(const QStringList& lst);
    void getCertificates(const QString& host, const QString& system_user);
    void getContainers(const QString& host, const QString& system_user);
    void selectHosts();
    void setSelectHosts(const json& hosts);
    void selectDatabaseUser();
    void setSelectDatabaseUser(const json& user);
    //void getAvailableCertificates(const QString& user_uuid);
    void selectCertificate();

public slots:
   void onCertUserCache(const QString& host, const QString& system_user, const nlohmann::json& data);
   void onMozillaProfiles();
   void doMozillaProfiles(const QStringList& lst);
   void onProfilesChanged(const QStringList& lst);
   void onCertificates(const arcirk::client::cryptopro_data& crypt_data);
   void onContainers(const arcirk::client::cryptopro_data& crypt_data);
   void onSelectHosts(const json& hosts);
   void doSelectHosts();
   void doSelectDatabaseUser();
   void onSelectDatabaseUser(const json& user);
   void onAvailableCertificates(const json& table);
   void onSelectCertificate(const json cert);
};

#endif // DIALOGCERTUSERCACHE_H
