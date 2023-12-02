#ifndef DIALOGSELECTAUTH_H
#define DIALOGSELECTAUTH_H

#include <QDialog>
#include "shared_struct.hpp"

using json = nlohmann::json;

namespace Ui {
class DialogSelectAuth;
}

class DialogSelectAuth : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelectAuth(QWidget *parent = nullptr);
    ~DialogSelectAuth();

    json get_result() const;


private slots:
    void on_toolButton_toggled(bool checked);

private:
    Ui::DialogSelectAuth *ui;
};

#endif // DIALOGSELECTAUTH_H
