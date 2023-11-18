#ifndef QUERYBUILDERSORTWIDGET_H
#define QUERYBUILDERSORTWIDGET_H

#include <QWidget>

namespace Ui {
class QueryBuilderSortWidget;
}
namespace arcirk::query_builder_ui {
    class QueryBuilderSortWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryBuilderSortWidget(QWidget *parent = nullptr);
        ~QueryBuilderSortWidget();

    private:
        Ui::QueryBuilderSortWidget *ui;
    };
}
#endif // QUERYBUILDERSORTWIDGET_H
