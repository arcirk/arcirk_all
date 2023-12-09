#include "pluginpropertydialog.h"
#include "ui_pluginpropertydialog.h"
#include <QLabel>

using namespace arcirk::plugins;

PluginPropertyDialog::PluginPropertyDialog(const QJsonObject& object, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PluginPropertyDialog)
{
    ui->setupUi(this);

    m_object = object;
    createControls();
}

PluginPropertyDialog::~PluginPropertyDialog()
{
    delete ui;
}

void PluginPropertyDialog::createControls()
{
    int row = 0;
    for (auto itr = m_object.begin(); itr != m_object.end(); ++itr) {
        auto label = new QLabel(this);
        label->setText(itr.key() + ":");
        ui->gridLayout->addWidget(label, row, 0);
        if(itr.value().toVariant().typeId() == QMetaType::QString){
            auto control = new PropertyControl(ControlType::TextLine, this);
            control->setObjectName(itr.key());
            m_ctrls.append(control);
            ui->gridLayout->addWidget(control, row, 1);
        }
    }
}
