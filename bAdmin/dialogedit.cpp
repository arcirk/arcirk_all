#include "dialogedit.h"
#include "ui_dialogedit.h"
#include <QUuid>
#include "dialogselectintree.h"
#include <QStringListModel>
#include <QMessageBox>

DialogEditCertUser::DialogEditCertUser(arcirk::database::cert_users& source, const QString& parent_name, const json& dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEdit),
    source_(source)
{
    ui->setupUi(this);

    bool is_new = false;
    auto ref = source_.ref;
    if(ref.empty() || ref == NIL_STRING_UUID){
        source_.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        is_new = true;
    }
    auto first = source_.first;
    auto second = source_.second;
    is_group = source_.is_group;

    ui->lblUUID->setText(QString::fromStdString(source_.ref));
    ui->txtFirst->setText(QString::fromStdString(first));
    ui->txtSecond->setText(QString::fromStdString(second));
    ui->chIsGroup->setCheckState(is_group == 0 ? Qt::Unchecked : Qt::Checked);
    ui->txtParent->setText(parent_name);
    ui->txtSystemUser->setText(source_.system_user.c_str());
    ui->txtSid->setText(source_.sid.c_str());

    formControl();

    model_users = nullptr;

    QStringList lst{""};
    for (auto itr = dev.begin(); itr != dev.end(); ++itr) {
        auto it = *itr;
        lst.push_back(it["first"].get<std::string>().c_str());
    }

    auto lst_ = new QStringListModel(lst, this);
    ui->cmbHosts->setModel(lst_);
    ui->cmbHosts->setCurrentText(source_.host.c_str());

    ui->txtSystemUser->setEnabled(!is_group);
    ui->lblSid->setVisible(!is_group);
    ui->txtSid->setVisible(!is_group);

    ui->txtUUID_user->setText(source_.uuid.c_str());

    if(is_new)
        setWindowTitle("Новый пользователь");
    else
        setWindowTitle(source_.first.c_str());

    auto buttons = ui->buttonBox->buttons();
    foreach (auto btn, buttons) {
        if(btn->text() == "Cancel")
            btn->setText("Отмена");
    }
}

DialogEditCertUser::DialogEditCertUser(arcirk::database::cert_users &source, const QString &parent_name, TreeItemModel *model, const json& dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEdit),
    source_(source)
{
    ui->setupUi(this);

    bool is_new = false;
    auto ref = source_.ref;
    if(ref.empty() || ref == NIL_STRING_UUID){
        source_.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        is_new = true;
    }
    auto first = source_.first;
    auto second = source_.second;
    is_group = source_.is_group;

    ui->lblUUID->setText(QString::fromStdString(source_.ref));
    ui->txtFirst->setText(QString::fromStdString(first));
    ui->txtSecond->setText(QString::fromStdString(second));
    ui->chIsGroup->setCheckState(is_group == 0 ? Qt::Unchecked : Qt::Checked);
    ui->txtParent->setText(parent_name);
    ui->txtSystemUser->setText(source.system_user.c_str());
    ui->txtSid->setText(source_.sid.c_str());
    ui->txtUUID_user->setText(source_.uuid.c_str());

    formControl();

    model_users = model;

    QStringList lst{""};
    for (auto itr = dev.begin(); itr != dev.end(); ++itr) {
        auto it = *itr;
        lst.push_back(it["first"].get<std::string>().c_str());
    }

    auto lst_ = new QStringListModel(lst, this);
    ui->cmbHosts->setModel(lst_);
    ui->cmbHosts->setCurrentText(source_.host.c_str());

    ui->txtSystemUser->setEnabled(!is_group);
    ui->lblSid->setVisible(!is_group);
    ui->txtSid->setVisible(!is_group);

    if(is_new)
        setWindowTitle("Новый пользователь");
    else
        setWindowTitle(source_.first.c_str());

    auto buttons = ui->buttonBox->buttons();
    foreach (auto btn, buttons) {
        if(btn->text() == "Cancel")
            btn->setText("Отмена");
    }
}

DialogEditCertUser::~DialogEditCertUser()
{
    delete ui;
}

void DialogEditCertUser::accept()
{
    source_.first = ui->txtFirst->text().trimmed().toStdString();
    source_.second = ui->txtSecond->text().trimmed().toStdString();
    source_.host = ui->cmbHosts->currentText().toStdString();
    source_.system_user = ui->txtSystemUser->text().toStdString();
    source_.sid = ui->txtSid->text().toStdString();

    if(is_group)
        source_.uuid = source_.ref;

    QDialog::accept();
}

void DialogEditCertUser::setServerObject(arcirk::server::server_objects value)
{
    srv_object = value;
}

void DialogEditCertUser::set_1c_parent(const QString &name)
{
    ui->txtParentUserName->setText(name);
}

void DialogEditCertUser::formControl()
{
    bool v = is_group == 0 ? false : true;
    ui->lblHost->setVisible(!v);
    ui->lblParentUserName->setVisible(!v);
    ui->btnSelectUser->setVisible(!v);
    ui->btnSelectSid->setVisible(!v);
    ui->cmbHosts->setVisible(!v);
    ui->txtParentUserName->setVisible(!v);
    ui->txtSystemUser->setVisible(!v);
    ui->txtUUID_user->setVisible(!v);
    ui->lblSystemUser->setVisible(!v);
    ui->lblUuid_user->setVisible(!v);

}

void DialogEditCertUser::on_btnSelectUser_clicked()
{

    if(!model_users)
        return;

    auto dlg = DialogSelectInTree(model_users, {"ref", "parent", "is_group", "deletion_mark"}, this);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto sel_object = dlg.selectedObject();
        source_.uuid = sel_object["ref"].get<std::string>();
        ui->txtUUID_user->setText(source_.uuid.c_str());
        ui->txtParentUserName->setText(sel_object["first"].get<std::string>().c_str());
        ui->txtFirst->setText(sel_object["first"].get<std::string>().c_str());
        ui->txtSecond->setText(sel_object["second"].get<std::string>().c_str());

    }

}


void DialogEditCertUser::on_buttonBox_accepted()
{
    accept();
}


void DialogEditCertUser::on_btnSetRoot_clicked()
{
    if(QMessageBox::question(this, "Очистка", "Очистить?") == QMessageBox::No)
        return;

    ui->txtParent->setText("");
    source_.parent = NIL_STRING_UUID;
}


void DialogEditCertUser::on_btnSelectSid_clicked()
{
    if(ui->cmbHosts->currentText().isEmpty())
    {
        QMessageBox::critical(this, "Ошибка", "Не выбран хост");
        return;
    }

    emit selectRemoteUser(ui->cmbHosts->currentText());
}

void DialogEditCertUser::onSelectRemoteUser(const arcirk::cryptography::win_user_info &value)
{
    ui->txtSid->setText(value.sid.c_str());
    ui->txtSystemUser->setText(value.user.c_str());
}

