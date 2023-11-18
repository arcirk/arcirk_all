#ifndef QUERYBUILDERADDITIONALLYWIDGET_H
#define QUERYBUILDERADDITIONALLYWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QSpinBox>
#include <QRadioButton>
#include <QUuid>
#include "query_builder.hpp"
#include "gui/treeviewwidget.h"
#include "gui/tabletoolbar.h"
#include "iface/iface.hpp"
#include <QLabel>

using namespace arcirk::database::builder;
using namespace arcirk::tree_widget;

namespace Ui {
class QueryBuilderAdditionallyWidget;
}
    namespace arcirk::query_builder_ui {
    class QueryBuilderAdditionallyWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryBuilderAdditionallyWidget(QWidget *parent = nullptr, const QUuid& packade_uuid = {});
        ~QueryBuilderAdditionallyWidget();

        void save_to_database();

        void set_database_structure(ITree<ibase_object_structure>* structurte){m_structurte = structurte;};

    private slots:
        void onChkLimitToggled(bool checked);
        void onRatioClicked();
        void onToolBarFieldItemClicked(const QString& buttonName);
        void onToolBarIndexItemClicked(const QString& buttonName);

    private:
        Ui::QueryBuilderAdditionallyWidget *ui;
        QUuid m_packade_uuid;
        int m_current_type;
        QMap<int, QRadioButton*> m_radio;
        tree_widget::TreeViewWidget* treeView;
        tree_widget::TreeViewWidget* treeViewIndexes;
        TableToolBar* m_toolBarField;
        TableToolBar* m_toolBarIndexs;
        QLabel* lblTop;
        QLabel* lblBottom;
        ITree<ibase_object_structure>* m_structurte;

        void form_control(QWidget* btn);
        ITreeIbaseModel* create_model(const QString& table_name);
        void hide_columns(TreeViewWidget* tree);
        void onBtnAddField();
        void onBtnEditField();

    signals:
        void selectedQueryType(sql_global_query_type index);
    };
}
#endif // QUERYBUILDERADDITIONALLYWIDGET_H
