#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>
#include <QWidget>

class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    LineEdit(QWidget *parent = nullptr);
//protected:
//    void changeEvent(QEvent *e);
private slots:
    void onValidateText(const QString& text);
};

#endif // LINEEDIT_H
