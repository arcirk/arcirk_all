#ifndef DIALOGTREEMODELSETTINGS_H
#define DIALOGTREEMODELSETTINGS_H

#include <QDialog>
#include "tree_model.h"
#include "global/arcirk_qt.hpp"

using namespace arcirk::tree_model;

namespace Ui {
class DialogTreeModelSettings;
}

class DialogTreeModelSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTreeModelSettings(std::shared_ptr<root_tree_conf>& conf, QWidget *parent = nullptr);
    ~DialogTreeModelSettings();

private:
    Ui::DialogTreeModelSettings *ui;
    std::shared_ptr<root_tree_conf> conf_;
};

#endif // DIALOGTREEMODELSETTINGS_H
