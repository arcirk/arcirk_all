#include "taskparamdialog.h"
#include "ui_taskparamdialog.h"
//#include "shared_struct_qt.hpp"
#include <QToolButton>
#include "shared_struct.hpp"

TaskParamDialog::TaskParamDialog(const json& param, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaskParamDialog)
{
    ui->setupUi(this);

    auto model = new ITreeParam(this);
    model->set_hierarchical_list(false);
//    auto columns = json::array({"name", "value"});
//    auto rows = json::array();
//    for (auto itr = param.items().begin(); itr != param.items().end(); ++itr) {
//        auto row = json::object();
//        row["name"] = itr.key();
//        row["value"] = itr.value();
//        rows += row;
//    }
//    auto table = json::object();
//    table["columns"] = columns;
//    table["rows"] = rows;
    model->set_read_only(false);
    //model->set_table(table);
    model->create_controls();

    //model->set_user_role_data("name", arcirk::tree::user_role::ReadOnlyRole, true);
    model->set_user_role_data("value", arcirk::tree::user_role::WidgetRole, arcirk::tree::item_editor_widget_roles::widgetVariantRole);
    model->set_user_role_data("name", arcirk::tree::user_role::WidgetRole, arcirk::tree::item_editor_widget_roles::widgetTextLineRole);

    model->set_enable_rows_icons(false);
    model->set_column_aliases(QMap<QString, QString>{qMakePair("name", "Наименование"), qMakePair("value", "Значение")});
//    int i = 0;
//    for (auto itr = param.items().begin(); itr != param.items().end(); ++itr) {
//        if(itr.value().is_string()){
//            auto index = model->index(i, 1, QModelIndex());
//            model->set_widget(index, item_editor_widget_roles::widgetTextLineRole);
//        }
//        i++;
//    }

//    model->set_items_read_only("name", true);
    m_tree = new TreeViewWidget(this);
    m_tree->enable_sort(false);
    m_tree->setModel(model);
    m_tree->hideColumn(model->column_index("ref"));
    m_tree->hideColumn(model->column_index("parent"));


//    auto delegate = new TreeItemDelegate(true, m_tree);
//    m_tree->setItemDelegate(delegate);
//    m_tree->setEditTriggers(QAbstractItemView::AllEditTriggers);
//    m_tree->setIndentation(0);
//    m_tree->setModel(model);

    ui->verticalLayout->addWidget(m_tree);

    connect(ui->btnAdd, &QToolButton::clicked, this, &TaskParamDialog::onInsertParam);

}

TaskParamDialog::~TaskParamDialog()
{
    delete ui;
}

void TaskParamDialog::accept()
{
//    auto model = (TreeItemModel*)m_tree->model();
//    auto arr = model->to_array(QModelIndex());
//    m_param = json::object();
//    for (auto it = arr.begin(); it != arr.end(); ++it) {
//        auto obj = *it;
//        m_param[obj["name"].get<std::string>()] = obj["value"];
//    }
    return QDialog::accept();
}

json TaskParamDialog::dialog_result() const
{
    //return m_param;
    return {};
}

void TaskParamDialog::read_param(const json& param)
{
//    if(!param.is_object())
//        return;

//    auto model = (ITreeParam*)m_tree->get_model();
//    auto items = param.items();
//    for (auto itr = items.begin(); itr != items.end(); ++itr) {
//        //auto object = arcirk::secure_serialization<param_structure>(itr);
//    }
}

void TaskParamDialog::onInsertParam()
{
    auto model = (ITreeParam*)m_tree->get_model();
    model->add_struct(param_structure());
}
