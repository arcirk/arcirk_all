#ifndef TEXTEDITDIALOG_H
#define TEXTEDITDIALOG_H

#ifndef IS_OS_ANDROID

#include <QDialog>

namespace Ui {
class TextEditDialog;
}

class TextEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextEditDialog(QWidget *parent = nullptr);
    ~TextEditDialog();

    void setText(const QString& text);
    QString text() const;

private:
    Ui::TextEditDialog *ui;
};

#endif
#endif // TEXTEDITDIALOG_H
