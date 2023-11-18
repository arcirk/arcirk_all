#include "querybuilderarbitraryexpressiondialog.h"
#include "ui_querybuilderarbitraryexpressiondialog.h"
#include <QToolButton>
#include <QMessageBox>
#include "queryeditoraddgroupdialog.h"
#include "querybuildesqlitesupportitemdialog.h"
#include "querybuilderselectdialog.h"
#include "iface/iface.hpp"

using namespace arcirk::tree_model;
using namespace arcirk::query_builder_ui;


QueryBuilderArbitraryExpressionDialog::QueryBuilderArbitraryExpressionDialog(const QVector<ibase_object_structure>& rows,
                                                                             const QString& user_function,
                                                                             QWidget *parent,
                                                                             const http_conf& conf) :
    QDialog(parent),
    ui(new Ui::QueryBuilderArbitraryExpressionDialog)
{
    ui->setupUi(this);

    treeFields = new TreeViewWidget(this, "QueryBuilderArbitraryExpressionDialog");
    treeFunctions = new TreeViewWidget(this, "QueryBuilderArbitraryExpressionDialog");

    ui->splitter->insertWidget(0, treeFields);
    ui->verticalLayout_2->addWidget(treeFunctions);

    m_http_conf = conf;

    textEdit = new CodeEditorWidget(this);
    ui->splitter_2->addWidget(textEdit);

    auto model = ibase_objects_init(treeFields, this, "query");
    //add_items<ITreeIbaseModel, ibase_object_structure>(rows, treeFields);
    model->add_items(rows);
    //hide_column(treeFields, "data_type", true);
    treeFields->setColumnHidden(model->column_index("data_type"), true);

    treeFields->set_drag_drop_behavior(behaviorText);
    treeFields->set_drag_data_column("query");
    treeFields->set_drag_group(false);

    highlighter = new QSourceHighliter(textEdit->document());
    highlighter->setCurrentLanguage(QSourceHighliter::CodeSQL);

    textEdit->setPlainText(user_function);

    auto header = treeFields->header();
    header->resizeSection(0, 200);

    model->set_enable_drag(true);
    //model->set_drag_data_column("full_name");

    help_model_init();
    init_connections();
}

QueryBuilderArbitraryExpressionDialog::QueryBuilderArbitraryExpressionDialog(const json &rows,
                                                                             const QString &user_function,
                                                                             QWidget *parent,
                                                                             const http_conf& conf) :
    QDialog(parent),
    ui(new Ui::QueryBuilderArbitraryExpressionDialog)
{

    ui->setupUi(this);
    treeFields = new TreeViewWidget(this, "QueryBuilderArbitraryExpressionDialog");
    treeFunctions = new TreeViewWidget(this, "QueryBuilderArbitraryExpressionDialog");
    ui->splitter->insertWidget(0, treeFields);
    ui->verticalLayout_2->addWidget(treeFunctions);

    m_http_conf = conf;

    textEdit = new CodeEditorWidget(this);
    ui->splitter_2->addWidget(textEdit);

    QVector<ibase_object_structure> items{};

    for (auto row = rows.begin(); row != rows.end(); ++row) {
        auto r = secure_serialization<ibase_object_structure>(*row);
        items.push_back(r);
    }

    auto model = ibase_objects_init(treeFields, this, "name");
    model->add_items(items);
    //add_items<ITreeIbaseModel, ibase_object_structure>(items, treeFields);
    //hide_column(treeFields, "data_type", true);
    treeFields->setColumnHidden(model->column_index("data_type"), true);
    treeFields->set_drag_drop_behavior(behaviorText);
    treeFields->set_drag_data_column("query");
    treeFields->set_drag_group(false);

    highlighter = new QSourceHighliter(textEdit->document());
    highlighter->setCurrentLanguage(QSourceHighliter::CodeSQL);

    textEdit->setPlainText(user_function);

    auto header = treeFields->header();
    header->resizeSection(0, 200);

    model->set_enable_drag(true);
   //model->set_drag_data_column("full_name");

    help_model_init();

    init_connections();
}

QueryBuilderArbitraryExpressionDialog::~QueryBuilderArbitraryExpressionDialog()
{
    delete ui;
}

void QueryBuilderArbitraryExpressionDialog::accept()
{
    query_ = textEdit->toPlainText();
    return QDialog::accept();
}

void QueryBuilderArbitraryExpressionDialog::setTableModel(const json &model)
{
    auto rootItem = ibase_object_structure();
    rootItem.object_type = "rootItem";

}

void QueryBuilderArbitraryExpressionDialog::onBtnViewFunctionToggled(bool checked)
{
    treeFunctions->setVisible(checked);
}


void QueryBuilderArbitraryExpressionDialog::onBtnViewFieldsToggled(bool checked)
{
    treeFields->setVisible(checked);
}

void QueryBuilderArbitraryExpressionDialog::onSetData(const ibase_object_structure &table, const QVector<ibase_object_structure> &fields)
{

}

void QueryBuilderArbitraryExpressionDialog::help_model_init()
{

    auto order = QVector<QString>({"name"});
    auto model = new ITreeSQliteSupportModel(order, this);
    model->set_column_aliases(QMap<QString, QString>{qMakePair("name", "Наименование")});
//    model->enable_drag(true);
//    model->set_drag_data_column("fun", true);
    if(!m_http_conf.host.empty()){
        //model->use_http(true);
        model->set_fetch_expand(true);
        model->set_connection(root_tree_conf::httpConnection, m_http_conf);
        //model->set_http_conf(QUrl(m_http_conf.host.c_str()), m_http_conf.token.c_str(), m_http_conf.table.c_str());
        //model->fetchMore(QModelIndex());
    }

    treeFunctions->setModel(model);
    for (int i = 1; i < model->columnCount(); ++i) {
        treeFunctions->hideColumn(i);
    }

    treeFunctions->header()->setVisible(false);
    treeFunctions->set_drag_drop_behavior(behaviorText);
    treeFunctions->set_drag_data_column("fun");
    treeFunctions->set_drag_group(false);
    treeFunctions->data_is_base64(true);
}

void QueryBuilderArbitraryExpressionDialog::init_connections()
{
    connect(ui->btnViewFields, &QToolButton::clicked, this, &QueryBuilderArbitraryExpressionDialog::onBtnViewFieldsToggled);
    connect(ui->btnViewFunction, &QToolButton::clicked, this, &QueryBuilderArbitraryExpressionDialog::onBtnViewFunctionToggled);
    connect(ui->btnType, &QToolButton::toggled, this, &QueryBuilderArbitraryExpressionDialog::onBtnTypeToggled);
    connect(ui->btnAdd, &QToolButton::clicked, this, &QueryBuilderArbitraryExpressionDialog::onBtnAddClicked);
    connect(ui->btnAddGroup, &QToolButton::clicked, this, &QueryBuilderArbitraryExpressionDialog::onBtnAddGroupClicked);
    connect(ui->btnEdit, &QToolButton::clicked, this, &QueryBuilderArbitraryExpressionDialog::onBtnEditClicked);
    connect(ui->btnMoveToParent, &QToolButton::clicked, this, &QueryBuilderArbitraryExpressionDialog::onBtnMoveToParentClicked);
}

void QueryBuilderArbitraryExpressionDialog::onBtnTypeToggled(bool checked)
{
    auto model = treeFields->get_model();
    auto index = model->column_index("data_type");
    treeFields->setColumnHidden(index, !checked);
//    if(!checked)
//        ui->treeFields->resizeColumnToContents(0);
}


void QueryBuilderArbitraryExpressionDialog::onBtnAddClicked()
{
    auto index = treeFunctions->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = (ITreeSQliteSupportModel*)treeFunctions->get_model();
    if(!model->is_group(index)){
        QMessageBox::critical(this, "Ошибка", "Элемент не является группой!");
        return;
    }

    std::string parent = NIL_STRING_UUID;
    QString parent_name;
    auto object = model->object(index);
    if(object.is_group != 0){
        parent = object.ref;
        parent_name = object.name.c_str();
    }

    auto item = sqlite_functions_info();
    item.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    item.parent = parent;
    item.is_group = 0;
    auto dlg = QueryBuildeSqliteSupportItemDialog(item, parent_name, this);
    if(dlg.exec() == QDialog::Accepted){
        model->http_insert(item, index);
    }

}

void QueryBuilderArbitraryExpressionDialog::onBtnAddGroupClicked()
{
    auto index = treeFunctions->current_index();
    std::string parent = NIL_STRING_UUID;
    QString parent_name;
    auto model = (ITreeSQliteSupportModel*)treeFunctions->get_model();
    if(index.isValid()){
        auto object = model->object(index);
        if(object.is_group != 0){
            parent = object.ref;
            parent_name = object.name.c_str();
        }
    }

    auto item = sqlite_functions_info();
    item.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    item.parent = parent;
    item.is_group = 1;
    auto dlg = QueryEditorAddGroupDialog(item, parent_name, this);
    if(dlg.exec() == QDialog::Accepted){
        model->http_insert(item, index);
    }

}



void QueryBuilderArbitraryExpressionDialog::onBtnEditClicked()
{
    auto index = treeFunctions->currentIndex();
    if(!index.isValid())
        return;
    QString parent_name;
    auto model = (ITreeSQliteSupportModel*)treeFunctions->get_model();
    auto object = model->object(index);
    auto parent_object = model->object(index.parent());
    parent_name = parent_object.name.c_str();
    if(object.is_group == 0){
        auto dlg = QueryBuildeSqliteSupportItemDialog(object, parent_name, this);
        if(dlg.exec() == QDialog::Accepted){
            model->http_update(object, index);
        }
    }else{
        auto dlg = QueryEditorAddGroupDialog(object, parent_name, this);
        if(dlg.exec() == QDialog::Accepted){
            model->http_update(object, index);
        }
    }

}

void QueryBuilderArbitraryExpressionDialog::onBtnMoveToParentClicked()
{
    auto index = treeFunctions->current_index();
    if(!index.isValid())
        return;

    auto m_model = (ITreeSQliteSupportModel*)treeFunctions->get_model();
    auto object = m_model->object(index);

    auto order = QList<QString>({"name"});
    auto model = new ITreeSQliteSupportModel(order,this);
    //model->set_group_only(true);
    model->set_column_aliases(QMap<QString, QString>{qMakePair("name", "Наименование")});
    //model->set_drag_data_column("fun", true);
    if(!m_http_conf.host.empty()){
       // model->use_http(true);
        model->set_fetch_expand(true);
        model->set_connection(root_tree_conf::httpConnection, m_http_conf);
//        model->set_http_conf(QUrl(m_http_conf.host.c_str()), m_http_conf.token.c_str(), m_http_conf.table.c_str());
//        model->fetchMore(QModelIndex());
    }

    auto dlg = QueryBuilderSelectDialog(model, true, this);
    if(dlg.exec() == QDialog::Accepted){
        auto destantion = secure_serialization<sqlite_functions_info>(dlg.selectedObject());
        auto verify = m_model->find(QUuid::fromString(destantion.ref), index);
        if(verify.isValid()){
            QMessageBox::critical(this, "Ошибка", "Не возможно переместить объект!");
            return;
        }
        auto new_parent = m_model->find(QUuid::fromString(destantion.ref), QModelIndex());
        if(new_parent.isValid())
            m_model->move_to(index, new_parent);
    }
}

