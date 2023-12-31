#include "querybuilderadditionallywidget.h"
#include "ui_querybuilderadditionallywidget.h"
#include <QCheckBox>
#include <QRadioButton>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSplitter>
#include <QLabel>
#include "querybuilderfieldexdialog.h"
#include "iface/iface.hpp"

using namespace arcirk::tree_model;
using namespace arcirk::query_builder_ui;
using namespace arcirk::database;

QueryBuilderAdditionallyWidget::QueryBuilderAdditionallyWidget(QWidget *parent, const QUuid& packade_uuid) :
    QWidget(parent),
    ui(new Ui::QueryBuilderAdditionallyWidget)
{
    ui->setupUi(this);
    m_packade_uuid = packade_uuid;

    connect(ui->chkLimit, &QCheckBox::toggled, this, &QueryBuilderAdditionallyWidget::onChkLimitToggled);
    connect(ui->radioSelect, &QRadioButton::clicked, this, &QueryBuilderAdditionallyWidget::onRatioClicked);
    connect(ui->radioCreateTempTable, &QRadioButton::clicked, this, &QueryBuilderAdditionallyWidget::onRatioClicked);
    connect(ui->radioDeleteTempTable, &QRadioButton::clicked, this, &QueryBuilderAdditionallyWidget::onRatioClicked);
    connect(ui->radioCreateTable, &QRadioButton::clicked, this, &QueryBuilderAdditionallyWidget::onRatioClicked);
    connect(ui->radioDeleteTable, &QRadioButton::clicked, this, &QueryBuilderAdditionallyWidget::onRatioClicked);

    ui->radioSelect->setChecked(true);

    m_toolBarField = new TableToolBar(this);
    treeView = new TreeViewWidget(this);
    treeView->setModel(create_model("qbAdditionalTable"));
    treeView->setTableToolBar(m_toolBarField);
    lblTop = new QLabel(this);
    lblTop->setText("Поля таблицы");
    ui->verticalLayout_5->addWidget(lblTop);
    ui->verticalLayout_5->addWidget(m_toolBarField);
    ui->verticalLayout_5->addWidget(treeView);

    lblBottom = new QLabel(this);
    lblBottom->setText("Индексы таблицы");
    m_toolBarIndexs = new TableToolBar(this);
    treeViewIndexes = new TreeViewWidget(this);
    treeViewIndexes->setModel(create_model("qbAdditionalIndex"));
    treeViewIndexes->setTableToolBar(m_toolBarIndexs);

    ui->verticalLayout_6->addWidget(lblBottom);
    ui->verticalLayout_6->addWidget(m_toolBarIndexs);
    ui->verticalLayout_6->addWidget(treeViewIndexes);

    treeView->enable_sort(false);
    treeViewIndexes->enable_sort(false);

    hide_columns(treeView);
    hide_columns(treeViewIndexes);

    m_current_type = 0;
    m_radio.insert(0, ui->radioSelect);
    m_radio.insert(1, ui->radioCreateTempTable);
    m_radio.insert(2, ui->radioDeleteTempTable);
    m_radio.insert(3, ui->radioCreateTable);
    m_radio.insert(4, ui->radioDeleteTable);

    auto m_connection = QSqlDatabase::database("private.sqlite");
    auto rc = m_connection.exec(QString("select * from qbPackets where ref = '%1'").arg(packade_uuid.toString(QUuid::WithoutBraces)));

    while (rc.next()) {
        ui->txtTenpTableName->setText(rc.value("name").toString());
        m_current_type = rc.value("type").toInt();

        ui->chkDistinct->setChecked(rc.value("use_distinct").toInt() > 0);
        ui->chkLimit->setChecked(rc.value("use_limit").toInt() > 0);
        ui->spinBox->setValue(rc.value("row_limit").toInt());
    }

    if(m_current_type >=0 && m_current_type < m_radio.size()){
        m_radio[m_current_type]->setChecked(true);
        form_control(m_radio[m_current_type]);
    }

    connect(treeView, &TreeViewWidget::toolBarItemClicked, this, &QueryBuilderAdditionallyWidget::onToolBarFieldItemClicked);
    connect(treeViewIndexes, &TreeViewWidget::toolBarItemClicked, this, &QueryBuilderAdditionallyWidget::onToolBarIndexItemClicked);

}

QueryBuilderAdditionallyWidget::~QueryBuilderAdditionallyWidget()
{
    delete ui;
}

void QueryBuilderAdditionallyWidget::save_to_database()
{
    //QList<QString> m_btn{"radioSelect", "radioCreateTempTable", "radioDeleteTempTable", "radioCreateTable", "radioDeleteTable"};
    auto m_connection = QSqlDatabase::database("private.sqlite");
    Q_ASSERT(m_connection.isOpen());
    QString m_packade = m_packade_uuid.toString(QUuid::WithoutBraces);
    auto obj = builder::query_builder::object<query_builder_packet>(m_connection, QString("select * from qbPackets where ref = '%1'").arg(m_packade));
    obj.name = ui->txtTenpTableName->text().toStdString();
    obj.row_limit = ui->spinBox->value();
    obj.type = m_current_type;
    obj.use_limit = ui->chkLimit->isChecked();
    obj.use_distinct = ui->chkDistinct->isChecked();
    auto b = builder::query_builder();
    b.use(pre::json::to_json(obj));
    m_connection.exec(b.update("qbPackets", true).where(json{{"ref", obj.ref}}, true).prepare().c_str());
    auto model = treeView->get_model();
    model->reset_sql_data();
}

void QueryBuilderAdditionallyWidget::onChkLimitToggled(bool checked)
{
    ui->spinBox->setEnabled(checked);
}


void QueryBuilderAdditionallyWidget::onRatioClicked()
{
    QRadioButton* btn = qobject_cast<QRadioButton*>(sender());
    Q_ASSERT(btn!=0);
    form_control(btn);
    QList<QString> m_btn{"radioSelect", "radioCreateTempTable", "radioDeleteTempTable", "radioCreateTable", "radioDeleteTable"};
    selectedQueryType((sql_global_query_type)m_btn.indexOf(btn->objectName()));
    m_current_type  = m_btn.indexOf(btn->objectName());
}

void QueryBuilderAdditionallyWidget::onToolBarFieldItemClicked(const QString &buttonName)
{
    if(buttonName == "add_item"){
        onBtnAddField();
    }else if(buttonName == "edit_item"){
        onBtnEditField();
    }
}

void QueryBuilderAdditionallyWidget::onToolBarIndexItemClicked(const QString &buttonName)
{
    auto index = treeView->current_index();
    if(!index.isValid())
        return;

    auto model = (ITreeIbaseModel*)treeView->model();
    auto object = model->object(index);

    auto dlg = QueryBuilderFieldExDialog(this);
    dlg.set_database_structure(m_structurte);
    dlg.setData(object);
    if(dlg.exec()){
        auto result = dlg.getField();
        auto model = (ITreeIbaseModel*)treeView->get_model();
        model->set_struct(result, index);
    }

}

void QueryBuilderAdditionallyWidget::onBtnAddField()
{
    auto dlg = QueryBuilderFieldExDialog(this);
    dlg.set_database_structure(m_structurte);
    if(dlg.exec()){
        auto result = dlg.getField();
        auto model = (ITreeIbaseModel*)treeView->get_model();
        model->add_struct(result);
    }
}

void QueryBuilderAdditionallyWidget::onBtnEditField()
{
    auto index = treeView->current_index();
    if(!index.isValid())
        return;
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto dlg = QueryBuilderFieldExDialog(this);
    dlg.set_database_structure(m_structurte);
    dlg.setData(model->object(index));
    if(dlg.exec()){
        auto result = dlg.getField();
        auto model = (ITreeIbaseModel*)treeView->get_model();
        model->set_struct(result, index);
    }
}

void QueryBuilderAdditionallyWidget::form_control(QWidget* btn)
{
    if(btn==0)
        return;
    lblBottom->setEnabled(false);
    lblTop->setEnabled(false);
    treeView->setEnabled(false);
    treeViewIndexes->setEnabled(false);
    m_toolBarField->setEnabled(false);
    m_toolBarIndexs->setEnabled(false);
    if(btn->objectName() == "radioSelect"){
        ui->txtTenpTableName->setEnabled(true);        
        ui->groupBox->setEnabled(true);
    }else if(btn->objectName() == "radioCreateTempTable"){
        ui->txtTenpTableName->setEnabled(true);
        ui->groupBox->setEnabled(false);
    }else if(btn->objectName() == "radioDeleteTempTable"){
        ui->txtTenpTableName->setEnabled(true);
        ui->groupBox->setEnabled(false);
    }else if(btn->objectName() == "radioCreateTable"){
        ui->txtTenpTableName->setEnabled(true);
        lblBottom->setEnabled(true);
        lblTop->setEnabled(true);
        treeView->setEnabled(true);
        treeViewIndexes->setEnabled(true);
        m_toolBarField->setEnabled(true);
        m_toolBarIndexs->setEnabled(true);
        ui->groupBox->setEnabled(false);
    }else if(btn->objectName() == "radioDeleteTable"){
        ui->txtTenpTableName->setEnabled(true);
        ui->groupBox->setEnabled(false);
    }

}

ITreeIbaseModel* QueryBuilderAdditionallyWidget::create_model(const QString &table_name)
{
    auto model = new ITreeIbaseModel(this);
    model->set_hierarchical_list(false);
    model->set_columns_order(QVector<QString>{"name", "data_type", "size", "query"});
    model->set_column_aliases(QMap<QString, QString>{qMakePair("name", "Поле"),
                                                     qMakePair("data_type", "Тип"),
                                                     qMakePair("size", "Размер"),
                                                     qMakePair("query", "Значение по умолчанию")});

//    model->set_read_only(false);
//    model->set_user_role_data("name", tree::user_role::WidgetRole, tree::item_editor_widget_roles::widgetTextLineRole);
//    model->set_user_role_data("data_type", tree::user_role::WidgetRole, tree::item_editor_widget_roles::widgetComboBoxRole);
//    model->set_user_role_data("size", tree::user_role::WidgetRole, tree::item_editor_widget_roles::widgetSpinBoxRole);
//    model->set_user_role_data("query", tree::user_role::WidgetRole, tree::item_editor_widget_roles::widgetVariantRole);
//    model->set_user_role_data("data_type", tree::user_role::UserRoleExt, QVariant(sqlite_types_qt));
//    model->set_enable_rows_icons(false);


    model->set_user_sql_where(json{{"package_ref", m_packade_uuid.toString(QUuid::WithoutBraces).toStdString()}});
    model->set_connection(root_tree_conf::sqlIteMemoryConnection, "", table_name);
    model->enable_database_changed();

    return model;
}

void QueryBuilderAdditionallyWidget::hide_columns(TreeViewWidget *tree)
{
    auto model = tree->get_model();
    for (int i = 0; i < model->columnCount(); ++i) {
        auto column = model->column_name(i);
        if(model->columns_order().indexOf(column) !=-1)
            treeView->setColumnHidden(i, false);
        else
            treeView->setColumnHidden(i, true);
    }

}

