#ifndef QUERYBUILDERINNERSALIASESWIDGET_H
#define QUERYBUILDERINNERSALIASESWIDGET_H

#include <QWidget>

namespace Ui {
class QueryBuilderInnersAliasesWidget;
}
namespace arcirk::query_builder_ui {
    class QueryBuilderInnersAliasesWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryBuilderInnersAliasesWidget(QWidget *parent = nullptr);
        ~QueryBuilderInnersAliasesWidget();

    private:
        Ui::QueryBuilderInnersAliasesWidget *ui;
    };
}
#endif // QUERYBUILDERINNERSALIASESWIDGET_H
