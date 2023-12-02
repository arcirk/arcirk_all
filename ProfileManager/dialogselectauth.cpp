#include "dialogselectauth.h"
#include "ui_dialogselectauth.h"


DialogSelectAuth::DialogSelectAuth(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSelectAuth)
{
    ui->setupUi(this);
}

DialogSelectAuth::~DialogSelectAuth()
{
    delete ui;
}

json DialogSelectAuth::get_result() const
{
    auto result = json::object();
    result["user"] = ui->txtUserName->text().toStdString();
    if(!ui->txtPassword->text().isEmpty()){
        auto pwd = ui->txtPassword->text();
        result["password"] = arcirk::crypt(pwd.toStdString(), CRYPT_KEY);
    }else
        result["password"] = "";
    return result;

}

void DialogSelectAuth::on_toolButton_toggled(bool checked)
{
    auto echoMode = checked ? QLineEdit::Normal : QLineEdit::Password;
    QString image = checked ? ":/img/viewPwd.svg" : ":/img/viewPwd1.svg";
    auto btn = dynamic_cast<QToolButton*>(sender());
    btn->setIcon(QIcon(image));
    ui->txtPassword->setEchoMode(echoMode);
}

