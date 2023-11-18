#ifndef QUERYINNERCOMPAREWIDGET_H
#define QUERYINNERCOMPAREWIDGET_H

#include <QWidget>
//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"

using json = nlohmann::json;

namespace Ui {
class QueryInnerCompareWidget;
}

namespace arcirk::tree_model {
    class QueryInnerCompareWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryInnerCompareWidget(QWidget *parent = nullptr);
        explicit QueryInnerCompareWidget(int row, int col, QWidget *parent = nullptr);
        ~QueryInnerCompareWidget();

        void setActivePage(int index);

        void setModelIndex(int row, int col);

        void setLeftData(const QStringList& value, const QString& table_bame = "Table1");
        void setRightData(const QStringList& value, const QString& table_bame = "Table2");

        QString text() const;
        void setText(const QString& text);

        void setTableModel(const json& model);

        void set_rcmb_read_only(bool value);

    private slots:
        void onCmbLeftCompareCurrentIndexChanged(int index);
        void onCmbRightCompareCurrentIndexChanged(int index);
        void onCuurrentTextChanged(const QString &arg1);
        void onOpenArbitraryExpressionDialog();

    private:
        Ui::QueryInnerCompareWidget *ui;
        QPair<int,int> currentIndex;
        QString leftTable;
        QString rightTable;
        json table_model;

        void formControl(int row, int col);

        void update_data();
        void parse();
    signals:
        void selectionItemLeftChanged(int row, int col, const QString& value, int index);
        void selectionItemRightChanged(int row, int col, const QString& value, int index);

    };
}
#endif // QUERYINNERCOMPAREWIDGET_H
