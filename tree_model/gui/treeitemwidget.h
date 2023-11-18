#ifndef TREEITEMWIDGET_H
#define TREEITEMWIDGET_H

#ifndef IS_OS_ANDROID

#include <QWidget>

namespace Ui {
class TreeItemWidget;
}

namespace arcirk::tree::widgets {
    class TreeItemWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit TreeItemWidget(int row, int column, QWidget *parent = nullptr);
        ~TreeItemWidget();

        virtual void setText(const QString&) = 0;
        virtual QString text() const = 0;
        virtual int row() const;
        virtual int column() const;
        virtual bool isChecked() = 0;
        virtual void setChecked(bool value) = 0;
        virtual QVariant currentState() const = 0;
        virtual void setCurrentState(const QVariant& state) = 0;

    private:
        Ui::TreeItemWidget *ui;
        int m_row;
        int m_column;
    };
}

#endif
#endif // TREEITEMWIDGET_H
