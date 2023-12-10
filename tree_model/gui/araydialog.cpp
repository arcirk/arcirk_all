#include "araydialog.h"
#include "ui_araydialog.h"

using namespace arcirk::tree_widget;

json ArrayDialog::result() const
{
    return m_data;
}

void ArrayDialog::accept()
{
    m_tree->close_editor();
    auto model = m_tree->get_model();
    m_data = model->unload_column(0);
    return QDialog::accept();
}

ArrayDialog::ArrayDialog(const json& data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ArrayDialog)
{
    ui->setupUi(this);

    m_data = data;
    if(!m_data.is_array())
        m_data = json::array();

    json table = json::object();
    table["columns"] = json::array({"value"});
    auto rows = json::array();
    for (auto itr = m_data.begin(); itr != m_data.end(); ++itr) {
        rows += json{{"value", *itr}};
    }
    table["rows"] = rows;

    auto model = new TreeItemModel(json::object({{"value", ""}}), this);
    model->set_hierarchical_list(false);
    model->set_read_only(false);
    model->set_enable_rows_icons(false);
    model->enable_drag_and_drop(false);
    model->set_user_role_data("value", tree::WidgetRole, tree::widgetTextLineRole);
    model->set_column_aliases(QMap<QString, QString>{qMakePair("value", "Значение")});
    model->set_columns_order(QList<QString>{"value"});
    model->set_table(table);

    m_toolbar = new TableToolBar(this);
    m_tree = new TreeViewWidget(this);
    m_tree->setTableToolBar(m_toolbar);
    m_tree->enable_sort(false);

    m_tree->setModel(model);

    m_tree->hide_default_columns();

    ui->verticalLayout->addWidget(m_toolbar);
    ui->verticalLayout->addWidget(m_tree);

    connect(m_tree, &TreeViewWidget::toolBarItemClicked, this, &ArrayDialog::onToolBarItemClicked);
}

ArrayDialog::~ArrayDialog()
{
    delete ui;
}

void ArrayDialog::onToolBarItemClicked(const QString& buttonName)
{
    if(buttonName == "add_item"){
        auto model = m_tree->get_model();
        auto item = model->empty_data();
        model->add(item);
    }
}
