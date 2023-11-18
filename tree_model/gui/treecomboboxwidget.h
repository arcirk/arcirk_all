#ifndef TREECOMBOBOXWIDGET_H
#define TREECOMBOBOXWIDGET_H

#include <QWidget>

namespace Ui {
class TreeComboBoxWidget;
}

namespace arcirk::tree_model {

    class TreeComboBoxWidget : public QWidget
    {
        Q_OBJECT

        public:
            explicit TreeComboBoxWidget(QWidget *parent = nullptr);
            ~TreeComboBoxWidget();

            void addItems(const QStringList& list);
            void setCurrentIndex(int index);
            QString currentText() const;

        private:
            Ui::TreeComboBoxWidget *ui;

        private slots:
            void onCurrentIndexChanged(int index);


        signals:
            void currentIndexChanged(int index);
    };

}
#endif // TREECOMBOBOXWIDGET_H
