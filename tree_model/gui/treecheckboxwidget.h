#ifndef TREECHECKBOXWIDGET_H
#define TREECHECKBOXWIDGET_H

#include <QWidget>
#include <QCheckBox>

namespace Ui {
class TreeCheckBoxWidget;
}

namespace arcirk::tree_model {
class TreeCheckBoxWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TreeCheckBoxWidget(Qt::Alignment align, QWidget *parent = nullptr);
    ~TreeCheckBoxWidget();

    void setChecked(bool value);
    bool isChecked() const;
    void setCurrentIndex(int row, int col);

private:
    Ui::TreeCheckBoxWidget *ui;
    QPair<int,int> current_index;
    QCheckBox* m_check_box;

signals:
    void stateChanged(int row, int col, bool state);
private slots:
    void onCheckBoxStateChanged(int arg1);
};
}
#endif // TREECHECKBOXWIDGET_H
