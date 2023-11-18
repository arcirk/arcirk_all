#ifndef DIALOGSETTABLEALIAS_H
#define DIALOGSETTABLEALIAS_H

#include <QDialog>

namespace Ui {
class DialogSetTableAlias;
}

class DialogSetTableAlias : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSetTableAlias(std::string& name, std::string& alias, QWidget *parent = nullptr);
    ~DialogSetTableAlias();

    void accept() override;

private:
    Ui::DialogSetTableAlias *ui;
    std::string& m_name;
    std::string& m_alias;
};

#endif // DIALOGSETTABLEALIAS_H
