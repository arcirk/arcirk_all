#include "dialogtemptable.h"
#include "ui_dialogtemptable.h"
#include "tree_model.h"
//#include "stringvalidator.h"
#include <QToolButton>
#include <QLineEdit>
#include "querybuildertableswidget.h"

using namespace arcirk::tree_model;
using namespace arcirk::query_builder_ui;

DialogTempTable::DialogTempTable(ibase_object_structure& root, QVector<ibase_object_structure>& details, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTempTable),
    root_(root),
    details_(details)
{
    ui->setupUi(this);

    treeView = new TreeViewWidget(this);
    ui->verticalLayout->addWidget(treeView);

    ui->txtName->setText(root.name.c_str());
    ui->txtAlias->setText(root.alias.c_str());
    bool is_veiw = root.object_type == "view";
    ui->lblAlias->setVisible(is_veiw);
    ui->txtAlias->setVisible(is_veiw);
    ui->txtName->setEnabled(!is_veiw);
    //ui->txtAlias->textChanged(ui->txtAlias->text());
    //ui->txtName->textChanged(ui->txtName->text());

    auto model = ibase_objects_init(treeView, this, "alias", false, false);
    model->set_column_aliases(QMap<QString, QString>({qMakePair("alias", "Имя поля"), qMakePair("data_type", "Тип значения")}));
    model->set_user_role_data("alias", arcirk::tree::user_role::WidgetRole, item_editor_widget_roles::widgetTextLineRole);
    model->set_user_role_data("data_type", arcirk::tree::user_role::WidgetRole, item_editor_widget_roles::widgetComboBoxRole);
    model->set_user_role_data("data_type", arcirk::tree::user_role::UserRoleExt, QVariant(sqlite_types_qt));
    auto rows = json::array();
    auto columns = json::array();
    auto table = json::object();
    auto obj = pre::json::to_json(root);
    foreach (auto itr, details) {
        rows += pre::json::to_json(itr);
    }

    for(auto itr = obj.items().begin(); itr != obj.items().end(); ++itr) {
        columns += itr.key();
    }

    table["columns"] = columns;
    table["rows"] = rows;

    model->set_table(table);

    auto buttons = ui->buttonBox->buttons();
    foreach (auto btn, buttons) {
        if(btn->text() == "Cancel")
            btn->setText("Отмена");
    }

    connect(ui->txtAlias, &QLineEdit::textChanged, this, &DialogTempTable::onTxtAliasTextChanged);
    connect(ui->txtName, &QLineEdit::textChanged, this, &DialogTempTable::onTxtNameTextChanged);
    connect(ui->btnAdd, &QToolButton::clicked, this, &DialogTempTable::onBtnAddClicked);
    connect(ui->btnAddCopy, &QToolButton::clicked, this, &DialogTempTable::onBtnAddCopyClicked);
    connect(ui->btnDelete, &QToolButton::clicked, this, &DialogTempTable::onBtnDeleteClicked);
}

DialogTempTable::~DialogTempTable()
{
    delete ui;
}

void DialogTempTable::accept()
{
    root_.name = ui->txtName->text().toStdString();
    if(ui->txtName->isEnabled())
        root_.alias = root_.name;
    else
        root_.alias = ui->txtAlias->text().toStdString();

    root_.query = root_.name;

    auto model = (ITreeIbaseModel*)treeView->get_model();
    details_  = model->array(QModelIndex());
    return QDialog::accept();
}

void DialogTempTable::onBtnAddClicked()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto empty = ibase_object_structure();
    empty.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    empty.alias = "НовоеПоле";// QString(QString("НовоеПоле") + QString::number(model->rowCount())).toStdString();
    empty.name = QString("НовоеПоле").toStdString();
    empty.query = QString("НовоеПоле").toStdString();
    empty.parent = root_.ref;
    empty.object_type = "field";
    empty.package_ref = root_.package_ref;
    model->add_struct(empty);

}


void DialogTempTable::onTxtNameTextChanged(const QString &arg1)
{
    QRegularExpression rx("[A-Za-z\\d]+");
    QValidator * validator = new QRegularExpressionValidator(rx, this);
    QLineEdit *sender_ = qobject_cast<QLineEdit *>(sender());
    //const QValidator *validator = sender_->validator();
    if (validator) {
        int pos;
        QString s = arg1;
        QString style;
        if (validator->validate(s, pos) != QValidator::Acceptable){
            style = "color: red";
        }else
            style = "";
        sender_->setStyleSheet(style);
        delete validator;
    }

}


void DialogTempTable::onTxtAliasTextChanged(const QString &arg1)
{
    onTxtNameTextChanged(arg1);
}


void DialogTempTable::onBtnAddCopyClicked()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto index = treeView->current_index();
    if(index.isValid()){
        auto object = model->object(index);
        object.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        model->add_struct(object);
    }
}


void DialogTempTable::onBtnDeleteClicked()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto index = treeView->current_index();
    if(index.isValid())
        model->remove(index);
}

