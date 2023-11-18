#include "querybuildertotalbyhierarchy.h"
#include "ui_querybuildertotalbyhierarchy.h"

using namespace arcirk::tree_model;
using namespace arcirk::query_builder_ui;

QueryBuilderTotalByHierarchy::QueryBuilderTotalByHierarchy(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QueryBuilderTotalByHierarchy)
{
    ui->setupUi(this);

    m_tree_fields = new TreeViewWidget(this, "QueryBuilderTotalByHierarchy");
    m_tree_groups = new TreeViewWidget(this, "QueryBuilderTotalByHierarchy");
    m_tree_agr = new TreeViewWidget(this, "QueryBuilderTotalByHierarchy");

    ui->horizontalLayout_7->insertWidget(0, m_tree_fields);
    ui->splitter->addWidget(m_tree_groups);
    ui->splitter->addWidget(m_tree_agr);

    set_splitter_size(ui->splitter_2, parent, 40);

}

QueryBuilderTotalByHierarchy::~QueryBuilderTotalByHierarchy()
{
    delete ui;
}
