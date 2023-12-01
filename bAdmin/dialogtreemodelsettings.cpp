#include "dialogtreemodelsettings.h"
#include "ui_dialogtreemodelsettings.h"

DialogTreeModelSettings::DialogTreeModelSettings(std::shared_ptr<root_tree_conf>& conf, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTreeModelSettings)
{
    ui->setupUi(this);
    conf_ = conf;

    ui->chHierarchical_list->setChecked(conf_->hierarchical_list());
}

DialogTreeModelSettings::~DialogTreeModelSettings()
{
    delete ui;
}
