#include "dialogremoteusers.h"
#include "ui_dialogremoteusers.h"
#include <QMessageBox>
#include <QEventLoop>

DialogRemoteUsers::DialogRemoteUsers(const nlohmann::json& jsonModel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRemoteUsers)
{
    ui->setupUi(this);

    createColumnAliases();;

    model = new TreeItemModel(this);
    model->set_table(jsonModel);
    model->set_column_aliases(m_colAliases);
    ui->treeView->setModel(model);
    ui->treeView->resizeColumnToContents(0);

    m_result = -1;
    m_management = false;

    updateIcons();
}

DialogRemoteUsers::~DialogRemoteUsers()
{
    delete ui;
}

void DialogRemoteUsers::accept()
{
    auto table = ui->treeView;
    auto index = table->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto object = model->to_object(index);
    if(QString(object["state"].get<std::string>().c_str()).trimmed() != "Активно"){
        QMessageBox::critical(this, "Ошибка", "К выбранному сеансу подключиться не возможно!");
        return;
    }

    //qDebug() << object.dump().c_str();
    int iID = std::atoi(object["ID"].get<std::string>().c_str());
    QString user = object["user"].get<std::string>().c_str();

    if(user.trimmed().left(1) == ">"){
        QMessageBox::critical(this, "Ошибка", "Выбранный сеанс яляется текущим!");
        return;
    }

    m_result = iID;

    QDialog::accept();
}

int DialogRemoteUsers::seletedId()
{
    return m_result;
}

bool DialogRemoteUsers::management()
{
    return m_management;
}

void DialogRemoteUsers::on_btnSetOnlineOnly_toggled(bool checked)
{

}

void DialogRemoteUsers::createColumnAliases()
{
    m_colAliases.insert("ID", "ID");
    m_colAliases.insert("user", "Пользователь");
    m_colAliases.insert("seanse", "Сеанс");
    m_colAliases.insert("state", "Статус");
    m_colAliases.insert("act", "Время бездействия");
    m_colAliases.insert("date", "Дата подключения");
    m_colAliases.insert("time", "Время подключения");
}

void DialogRemoteUsers::updateIcons()
{

    for(int row = 0; row < model->rowCount(QModelIndex()); row++){
        auto index = model->index(row, 0, QModelIndex());
        auto object = model->to_object(index);
        QString state = object.value("state", "").c_str();
        if(state.trimmed() == "Активно"){
            model->set_row_image(index, QIcon(":/img/desktopOnline.png"));
        }else
            model->set_row_image(index, QIcon(":/img/desktopInlegal.png"));
    }

}

void DialogRemoteUsers::on_chkMnanager_toggled(bool checked)
{
    m_management = checked;
}

