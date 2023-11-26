#ifndef TABLETOOLBAR_H
#define TABLETOOLBAR_H

#ifndef IS_OS_ANDROID

#include <QWidget>

namespace Ui {
class TableToolBar;
}
namespace arcirk::tree_widget {
    class TableToolBar : public QWidget
    {
        Q_OBJECT

    public:
        explicit TableToolBar(QWidget *parent = nullptr);
        ~TableToolBar();

        void setHierarchyState(bool state);

        void setButtonEnabled(const QString& name, bool value);
        void setButtonVisible(const QString& name, bool value);
        void setSeparatorVisible(int index, bool value);
        QToolButton *botton(const QString& name);
        void addBotton(const QString& name, const QIcon& ico, bool checkable = false, int position = -1);

    private:
        Ui::TableToolBar *ui;
        QMap<QString,QToolButton*> m_bottons;

    public slots:
        void onHierarchyState(bool state);
        void onTreeEnabled(bool value);

    private slots:
        void onButtonClicked();

    signals:
        void itemClicked(const QString& buttonName);
    };
}

#endif
#endif // TABLETOOLBAR_H
