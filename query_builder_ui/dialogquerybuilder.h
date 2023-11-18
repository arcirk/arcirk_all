#ifndef DIALOGQUERYBUILDER_H
#define DIALOGQUERYBUILDER_H

#include <QDialog>
//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"
#include "query_builder.hpp"
#include "databasestructurewidget.h"
#include "querybuildertableswidget.h"
#include "querybuilderfieldswidget.h"
#include "websocketclient.h"
#include <QTabWidget>
#include "querybuildertableinnerswidget.h"
#include "querybuildergroupbyfieldslistwidget.h"
#include "querybuildergroupbyfunwidget.h"
#include "querybuilderadditionallywidget.h"
#include "querybuilderinnersaliaseswidget.h"
#include "querybuildersortwidget.h"
#include "querybuildertotalbyhierarchy.h"
#include "querybuilderwherewidget.h"

namespace Ui {
class DialogQueryBuilder;
}
namespace arcirk::query_builder_ui {
    enum builder_form_tabs{
        TAB_GENERAL = 0,
        TAB_FIELDS = 1,
        TAB_INNERS = 2,
        TAB_GROUP_BY = 3,
        TAB_WHERE = 4,
        TAB_ALIASES = 5,
        TAB_ORDER = 6,
        TAB_TOTAL = 7
    };

    class DialogQueryBuilder : public QDialog
    {
        Q_OBJECT

    public:
        explicit DialogQueryBuilder(WebSocketClient* client, const query_builder_packet& packade, QWidget *parent = nullptr);
        ~DialogQueryBuilder();

        void accept() override;

        void setQueryName(const QString& name){m_query_name = name;};
        QString queryName() const{return m_query_name;};

        void set_data(const query_builder_packet& data);

        //void set_query_package_uuid(const QUuid& value){m_uuid = value;};

    private:
        Ui::DialogQueryBuilder *ui;
        DatabaseStructureWidget* m_structure;
        QueryBuilderTablesWidget* m_query_tables;
        QueryBuilderFieldsWidget* m_query_fields;
        QueryBuilderTableInnersWidget* m_table_inners;
        QueryBuilderGroupByFieldsListWidget * m_group_by_fields;
        QueryBuilderGroupByFunWidget * m_group_by_function;
        QueryBuilderAdditionallyWidget * m_addionall_widget;
        QueryBuilderInnersAliasesWidget * m_inners_aliases_widget;
        QueryBuilderSortWidget * m_sort_widget;
        QueryBuilderTotalByHierarchy * m_total_widget;
        QueryBuilderWhereWidget * m_where_widget;
        WebSocketClient* m_client;
        QTabWidget * m_tab;
        QUuid m_packade_uuid;
        QString m_query_name;
        query_builder_packet m_packade;

        void reset_filer();
        QVector<ibase_object_structure> for_arbitrary_table(bool selected_fields = true);

    public slots:
        void onChangeTableList(int count);
        void onTabBarClicked(int index);
        void onGroupByFieldsDropEvent(const json& object);
        void onGroupByGroupsDropEvent(const json& object);
        void onGroupByFunctionsDropEvent(const json& object);
        void onOpenArbitraryFunctionsDialog(bool selected_fields, const QString& current_function = "", QWidget* parent = nullptr, const QString& ref = "");
        void onSelectedQueryType(sql_global_query_type index);

    signals:
        void changeTablesList();
        void setFieldsFilter(const QStringList &lst);
    private slots:
        void onButtonBoxAccepted();
        void onButtonBoxRejected();
        void onButtonGenerateQuery();
    };
}
#endif // DIALOGQUERYBUILDER_H
