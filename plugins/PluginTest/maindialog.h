#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QAbstractButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainDialog; }
QT_END_NAMESPACE

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

private slots:
    void onBtnCreateClicked();
    void onSelectPlugin();
    void onTextChanged(const QString& value);
    void onDialogBoxClicked(QAbstractButton * button);
    void onBtnParamClicked();

    void on_btnSelectPdf_clicked();

    void on_btnReadPdf_clicked();

    void on_btnHash_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainDialog *ui;
};
#endif // MAINDIALOG_H
