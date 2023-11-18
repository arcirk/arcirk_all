#ifndef IS_OS_ANDROID
#include "rowdialog.h"
#include "ui_rowdialog.h"
#include <QTextEdit>
#include <QSpinBox>
#include <QLabel>
#include <QComboBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include "treeitemtextline.h"

using namespace arcirk::tree_widget;
using namespace arcirk::tree::widgets;

RowDialog::RowDialog(const json& data, const User_Data& user_data, QWidget *parent,
                     const QMap<QString, QString>& aliases,
                     const QList<QString>& invisible,
                     const QList<QString>& order,
                     const QList<QString>& not_null) :
    QDialog(parent),
    ui(new Ui::RowDialog)
{
    ui->setupUi(this);

    m_data = data;
    m_user_data = user_data;
    m_aliases = aliases;
    m_not_null = not_null;

    is_group = m_data.value("is_group", 0) == 1;
    std::string ref = m_data.value("ref", "");
    if(is_group){
        if(ref.empty())
            setWindowTitle("Новая группа");
        else
            setWindowTitle(m_data.value("name", "Группа").c_str());
    }else{
        if(ref.empty())
            setWindowTitle("Новый элемент");
        else
            setWindowTitle(m_data.value("name", "Элемент").c_str());
    }

    if(ref.empty()){
        m_data["ref"] = generate_uuid().toStdString();
    }

    createControls(invisible, order);

    auto btn = ui->buttonBox->button(QDialogButtonBox::Cancel);
    if(btn)
        btn->setText("Отмена");
}

RowDialog::~RowDialog()
{
    delete ui;
}

json RowDialog::data()
{
    return m_data;
}

void RowDialog::accept()
{
    bool cancel = false;
    QGridLayout * layout = ui->gridLayout;
    for (int i = 0; i < m_data.size(); ++i) {
        auto w =  layout->itemAtPosition(i,1)->widget();
        if(w){
            auto cls = w->property("class").toString();
            if(cls == "QLineEdit"){
                auto ln = (TreeItemTextLine*)w;
                if(ln->autoMarkIncomplete() && ln->text().isEmpty()){
                    cancel = true;
                    //emit onError("Ошибка заполнения", QString("Не заполнено поле %1").arg(ln->objectName()));
                    QMessageBox::critical(this, "Ошибка заполнения", QString("Не заполнено поле %1").arg(fieldAlias(ln->objectName())));
                }

            }
        }
    }

    if(!cancel)
        return QDialog::accept();
}

void RowDialog::createControls(const QList<QString>& invisible, const QList<QString>& order)
{
    auto items = m_data.items();
    QList<QString> m_columns;
    foreach (auto itr, order) {
        if(m_data.find(itr.toStdString()) != m_data.end())
            m_columns.append(itr);
    }

    for (auto itr = items.begin(); itr != items.end(); ++itr) {
        if(order.indexOf(itr.key().c_str()) ==-1)
           m_columns.append(itr.key().c_str());
    }

    int row = 0;

    foreach (auto itr, m_columns) {
        bool visible = invisible.indexOf(itr) ==-1;
        auto value = m_data[itr.toStdString()];
        auto param = m_user_data.find(itr.toStdString());
        item_editor_widget_roles role = item_editor_widget_roles::widgetINVALID;
        if(param != m_user_data.end())
            role = (item_editor_widget_roles)param->first.toInt();

        if(role != item_editor_widget_roles::widgetINVALID){
            auto control = createEditor(itr, role, value);
            addWidget(itr, row, control, visible);
        }else{
            auto control = createControl(itr, value);
            addWidget(itr, row, control, visible);
        }

        row++;
    }

}

QList<QWidget *> RowDialog::createControl(const QString &key, const json &value)
{
    if(key == "is_group"){
        return createCheckBox(key, value);
    }else{
        if(value.is_boolean()){
           return createCheckBox(key, value);
        }else if(value.is_number()){
           return createSpinBox(key, value);
        }else {
           return createLineEdit(key, value);
        }
    }
}

QList<QWidget*> RowDialog::createEditor(const QString& key, item_editor_widget_roles role, const json& value)
{
    if(role == item_editor_widget_roles::widgetCheckBoxRole){
        return createCheckBox(key, value);
    }else if(role == item_editor_widget_roles::widgetComboBoxRole){
        return createComboBox(key, value);
    }else if(role == item_editor_widget_roles::widgetCompareRole){
        return QList<QWidget*>{};
    }else if(role == item_editor_widget_roles::widgetLabelRole){
        return createLineEdit(key, value);
    }else if(role == item_editor_widget_roles::widgetSpinBoxRole){
        return createSpinBox(key, value);
    }else if(role == item_editor_widget_roles::widgetTextEditRole){
        return createTextEdit(key, value);
    }else if(role == item_editor_widget_roles::widgetTextLineRole){
        return createLineEdit(key, value);
    }

    return QList<QWidget*>{};
}

QString RowDialog::fieldAlias(const QString& field){

    if(m_aliases.find(field) != m_aliases.end())
        return m_aliases[field];
    else
        return field;

}

QList<QWidget*> RowDialog::createCheckBox(const QString &key, const json &value)
{
    bool val = false;

    if(value.is_boolean()){
        val = value.get<bool>();
    }else if(value.is_number()){
        val = value.get<int>() > 0;
    }

    auto control = new QCheckBox(this);
    control->setChecked(val);
    control->setText(fieldAlias(key));
    control->setProperty("class", "QCheckBox");
    control->setObjectName(key);
    connect(control, &QCheckBox::clicked, this, &RowDialog::onControlDataChanged);

    return QList<QWidget*>{control};
}

QList<QWidget*> RowDialog::createSpinBox(const QString &key, const json &value)
{
    int val = 0;
    if(value.is_number()){
        val = value.get<int>();
    }
    auto control = new QSpinBox(this);
    control->setMaximum(100000);
    control->setValue(val);
    control->setAlignment(Qt::AlignRight);
    control->setProperty("class", "QSpinBox");
    control->setObjectName(key);

    auto lbl = new QLabel(this);
    lbl->setText(fieldAlias(key) + ":");
    lbl->setObjectName(QString(key) + "_LBL");

    connect(control, &QSpinBox::valueChanged, this, &RowDialog::onControlDataChanged);

    return QList<QWidget*>{control, lbl};
}

QList<QWidget *> RowDialog::createLineEdit(const QString &key, const json &value)
{
    QString val;
    if(value.is_string())
        val = value.get<std::string>().c_str();
    auto control = new TreeItemTextLine(0,0, this); QLineEdit(this);
    control->setText(val);
    if(key == "ref" || key == "parent"){
        control->setDefaultValue(NIL_STRING_UUID);
        if(key == "parent"){
           control->isClearButton(true);
           control->isReadOnly(true);
        }
    }
    control->setProperty("class", "QLineEdit");
    control->setObjectName(key);
    control->isBorder(true);
    control->setSpacing(1);
    if(m_not_null.indexOf(key) !=-1)
        control->setAutoMarkIncomplete(true);

    auto lbl = new QLabel(this);
    lbl->setText(fieldAlias(key) + ":");
    lbl->setObjectName(QString(key) + "_LBL");

    connect(control, &TreeItemTextLine::textChanged, this, &RowDialog::onControlDataChanged);

    return QList<QWidget*>{control, lbl};
}

QList<QWidget *> RowDialog::createComboBox(const QString &key, const json &value)
{
    auto control = new QComboBox(this);
    auto param = m_user_data.find(key.toStdString());

    if(param != m_user_data.end()){
        QStringList lst;
        auto ext = param->second;
        if(ext.isValid()){
            lst = ext.toStringList();
            control->addItems(lst);
            if(value.is_string()){
                int current = lst.indexOf(value.get<std::string>().c_str());
                if (current > -1) {
                    control->setCurrentIndex(current);
                }
            }
        }
    }

    control->setProperty("class", "QComboBox");
    control->setObjectName(key);

    connect(control, &QComboBox::currentTextChanged, this, &RowDialog::onControlDataChanged);

    auto lbl = new QLabel(this);
    lbl->setText(fieldAlias(key) + ":");
    lbl->setObjectName(QString(key) + "_LBL");

    return QList<QWidget*>{control, lbl};
}

void RowDialog::addWidget(const QString& key, int row, QList<QWidget *> control, bool visible)
{
//    if(!visible)
//        return;

    QGridLayout * layout = ui->gridLayout;
    bool enabled = true;
    if(m_disable.indexOf(key)!=-1)
        enabled = false;

    if(control.size() == 2){
        control[1]->setEnabled(enabled);
        control[1]->setVisible(visible);
        layout->addWidget(control[1], row, 0);
    }

    if(key != "parent"){
        control[0]->setEnabled(enabled);
    }
    control[0]->setVisible(visible);
    layout->addWidget(control[0], row, 1);

}

QList<QWidget *> RowDialog::createTextEdit(const QString &key, const json &value)
{
    QString val;
    if(value.is_string())
        val = value.get<std::string>().c_str();
    auto control = new QTextEdit(this);
    control->setText(val);
    control->setProperty("class", "QTextEdit");
    control->setObjectName(key);

    auto lbl = new QLabel(this);
    lbl->setText(fieldAlias(key) + ":");
    lbl->setObjectName(QString(key) + "_LBL");

    //connect(control, &QTextEdit::textChanged, this, &RowDialog::onControlDataChanged);

    return QList<QWidget*>{control, lbl};
}


void RowDialog::onControlDataChanged(const QVariant& value)
{
    auto w = sender();
    if(w){
        auto obj_name = w->objectName();
        QString cls = w->property("class").toString();
        if(cls == "QCheckBox"){
            auto ctrl = qobject_cast<QCheckBox*>(w);
            if(ctrl!=0 && m_data.find(obj_name.toStdString()) != m_data.end()){
                if(m_data[obj_name.toStdString()].is_boolean())
                    m_data[obj_name.toStdString()] = ctrl->isChecked();
                else
                    m_data[obj_name.toStdString()] = ctrl->isChecked() ? 1:0;
            }
        }else if(cls == "QSpinBox"){
            auto ctrl = qobject_cast<QSpinBox*>(w);
            if(ctrl!=0 && m_data.find(obj_name.toStdString()) != m_data.end()){
                m_data[obj_name.toStdString()] = ctrl->value();
            }
        }else if(cls == "QLineEdit"){
            auto ctrl = qobject_cast<TreeItemTextLine*>(w);
            if(ctrl!=0 && m_data.find(obj_name.toStdString()) != m_data.end()){
                m_data[obj_name.toStdString()] = ctrl->text().toStdString();
            }
        }else if(cls == "QComboBox"){
            auto ctrl = qobject_cast<QComboBox*>(w);
            if(ctrl!=0 && m_data.find(obj_name.toStdString()) != m_data.end()){
                m_data[obj_name.toStdString()] = ctrl->currentText().toStdString();
            }
        }
    }
}
#endif
