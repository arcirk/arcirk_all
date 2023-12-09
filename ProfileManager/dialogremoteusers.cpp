#include "dialogremoteusers.h"
#include "ui_dialogremoteusers.h"
#include <QMessageBox>
#include <QEventLoop>
#include <QPushButton>
#include <QDialogButtonBox>

DialogRemoteUsers::DialogRemoteUsers(const nlohmann::json& jsonModel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRemoteUsers)
{
    ui->setupUi(this);

    treeView = new TreeViewWidget(this);
    ui->verticalLayout->addWidget(treeView);

    createColumnAliases();;

    auto model = new TreeItemModel(this);
    model->set_hierarchical_list(false);
    model->set_column_aliases(m_colAliases);
    model->set_columns_order(QList<QString>{
            "ID",
            "user",
            "seanse",
            "state",
            "act",
            "date",
            "time"
    });
    model->set_table(jsonModel);
    treeView->setModel(model);
    treeView->hideColumn(model->column_index("ref"));
    treeView->hideColumn(model->column_index("parent"));
    //ui->treeView->resizeColumnToContents(0);

    m_result = -1;
    m_management = false;

    updateIcons();

    auto btn = ui->buttonBox->button(QDialogButtonBox::Cancel);
    if(btn)
        btn->setText("Отмена");
}

DialogRemoteUsers::~DialogRemoteUsers()
{
    delete ui;
}

void DialogRemoteUsers::accept()
{
    //auto table = ui->treeView;
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto object = treeView->get_model()->to_object(index);
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

    auto model = treeView->get_model();
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

