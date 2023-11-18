#ifndef QUERYBUILDERTOTALBYHIERARCHY_H
#define QUERYBUILDERTOTALBYHIERARCHY_H

#include <QWidget>
//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"
#include "tree_model.h"
#include "gui/treeviewwidget.h"

using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;

namespace Ui {
class QueryBuilderTotalByHierarchy;
}
namespace arcirk::query_builder_ui {
    class QueryBuilderTotalByHierarchy : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryBuilderTotalByHierarchy(QWidget *parent = nullptr);
        ~QueryBuilderTotalByHierarchy();

    private:
        Ui::QueryBuilderTotalByHierarchy *ui;
        TreeViewWidget* m_tree_fields;
        TreeViewWidget* m_tree_groups;
        TreeViewWidget* m_tree_agr;
    };
}
#endif // QUERYBUILDERTOTALBYHIERARCHY_H
