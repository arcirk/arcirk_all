#ifndef DIALOGSETVERSION_H
#define DIALOGSETVERSION_H

#include <QDialog>
#include "shared_struct.hpp"

namespace Ui {
class DialogSetVersion;
}

class DialogSetVersion : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSetVersion(const arcirk::client::version_application& ver,  QWidget *parent = nullptr);
    ~DialogSetVersion();

    arcirk::client::version_application getResult();

private:
    Ui::DialogSetVersion *ui;
};

#endif // DIALOGSETVERSION_H
