#ifndef DIALOGEDIT_H
#define DIALOGEDIT_H

#include <QDialog>
#include "shared_struct.hpp"
#include "tree_model.h"

using namespace arcirk::tree_model;

namespace Ui {
class DialogEdit;
}

using json = nlohmann::json;

class DialogEditCertUser : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditCertUser(arcirk::database::cert_users& source, const QString& parent_name, const json& dev, QWidget *parent = nullptr);
    explicit DialogEditCertUser(arcirk::database::cert_users& source, const QString& parent_name, TreeItemModel* model, const json& dev, QWidget *parent = nullptr);
    ~DialogEditCertUser();

    void accept() override;

    void setServerObject(arcirk::server::server_objects value);

    void set_1c_parent(const QString& name);

private slots:
    void on_btnSelectUser_clicked();
    void on_buttonBox_accepted();
    void on_btnSetRoot_clicked();
    void on_btnSelectSid_clicked();
public slots:
    void onSelectRemoteUser(const arcirk::cryptography::win_user_info& value);
private:
    Ui::DialogEdit *ui;
    arcirk::database::cert_users& source_;
    arcirk::server::server_objects srv_object;
    int is_group;
    TreeItemModel* model_users;

    void formControl();

signals:
    void selectRemoteUser(const QString& host);
};

#endif // DIALOGEDIT_H
