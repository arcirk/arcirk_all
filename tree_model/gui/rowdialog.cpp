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
#include "treeitemtextedit.h"
#include "treeitemvariant.h"

using namespace arcirk::tree_widget;
using namespace arcirk::tree::widgets;

RowDialog::RowDialog(const json& data, User_Data& user_data, QWidget *parent,
                     const QMap<QString, QString>& aliases,
                     const QList<QString>& invisible,
                     const QList<QString>& order,
                     const QString& parentSynonim,
                     const QMap<QString, tree_editor_inner_role>& inner_roles) :
    QDialog(parent),
    ui(new Ui::RowDialog),
    m_user_data(user_data)
{
    ui->setupUi(this);

    m_data = data;
    //m_user_data = user_data;
    m_aliases = aliases;
    m_parentSynonim = parentSynonim;
    m_parentRef = "";
    is_new_element = false;
    m_inner_roles = inner_roles;

    is_group = m_data.value("is_group", 0) == 1;
    std::string ref = m_data.value("ref", "");
    is_new_element = !ref.empty();

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

QMap<QString, tree_editor_inner_role> RowDialog::inner_roles() const
{
    return m_inner_roles;
}

void RowDialog::accept()
{
    bool cancel = false;
    QGridLayout * layout = ui->gridLayout;
    if(m_data.find("parent") != m_data.end()){
        m_data["parent"] = m_parentRef.toStdString();
    }
    for (int i = 0; i < m_data.size(); ++i) {
        auto w =  layout->itemAtPosition(i,1)->widget();
        if(w){
            auto cls = w->property("class").toString();
            if(cls == "QLineEdit" || cls == "QTextEdit"){
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
        auto role = widgetRole(itr);
        if(m_inner_roles.find(itr) == m_inner_roles.end())
           m_inner_roles.insert(itr, tree_editor_inner_role::widgetInnerRoleINVALID);
        if(visible){
           auto useRole = widgetUseRole(itr);
           if(useRole != forFolderAndItem){
                if(is_group && useRole == forItem)
                    visible = false;
                else if(!is_group && useRole == forFolder)
                    visible = false;
           }
        }
        if(role != item_editor_widget_roles::widgetINVALID){
            auto control = createEditor(itr, role, value);
            addWidget(itr, row, control, visible);
        }else{
            auto control = createControl(itr, value);
            addWidget(itr, row, control, visible);
        }

        row++;
    }
    auto itr = m_widgets.find("ref");
    if(itr != m_widgets.end()){
        foreach (auto w, itr.value()) {
            if(w)
                w->setVisible(false);
        }
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
    }else if(role == item_editor_widget_roles::widgetVariantRole){
        return createVariantBox(key, value);
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
    auto control = new TreeItemTextLine(0,0, this);
    control->setText(val);
    if(key == "ref" || key == "parent"){
        control->setDefaultValue(NIL_STRING_UUID);
        if(key == "parent"){
           control->isClearButton(true);
           control->isReadOnly(true);
           m_parentRef = val;
           control->setValue(m_parentRef);
           control->setSynonim(m_parentSynonim);
           control->enableClearBottom(!is_new_element);
        }
    }

    control->setExtValue(widgetExtRole(key));
    control->setProperty("class", "QLineEdit");
    control->setObjectName(key);
    control->isBorder(true);
    control->setSpacing(1);
    control->setAutoMarkIncomplete(widgetNotNull(key));
    control->setRole(widgetInnerRole(key));

    auto lbl = new QLabel(this);
    lbl->setText(fieldAlias(key) + ":");
    lbl->setObjectName(QString(key) + "_LBL");

    connect(control, &TreeItemTextLine::textChanged, this, &RowDialog::onControlDataChanged);
    connect(control, &TreeItemTextLine::valueChanged, this, &RowDialog::onValueChanged);

    return QList<QWidget*>{control, lbl};
}

QList<QWidget *> RowDialog::createComboBox(const QString &key, const json &value)
{
    auto control = new QComboBox(this);
    auto ext = widgetExtRole(key);

    //if(param != m_user_data.end()){
        QStringList lst;
        //auto ext = param->second;
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
    //}

    control->setProperty("class", "QComboBox");
    control->setObjectName(key);

    connect(control, &QComboBox::currentTextChanged, this, &RowDialog::onControlDataChanged);

    auto lbl = new QLabel(this);
    lbl->setText(fieldAlias(key) + ":");
    lbl->setObjectName(QString(key) + "_LBL");

    return QList<QWidget*>{control, lbl};
}

QList<QWidget *> RowDialog::createVariantBox(const QString &key, const json &value)
{
    auto control = new TreeItemVariant(0,0, this);
    auto  itr = m_inner_roles.find(key);
    if(itr != m_inner_roles.end()){
        control->setRole(itr.value());
    }
    control->setData(value);

//    try {
//        auto ba = value.get<ByteArray>();
//        if(ba.size() > 0)
//            control->setRawData(&ba);
//    } catch (const std::exception& e) {
//        qCritical() << e.what();
//    }

//    if(control->role() == widgetFilePath ||
//        control->role() == widgetDirectoryPath ||
//        control->role() == widgetText){
//        if(value.is_string()){
//            control->setText(value.get<std::string>().c_str());
//        }
//    }else if(control->role() == widgetInteger){
//        if(value.is_number()){
//            control->setData(value.get<int>());
//        }
//    }else if(control->role() == widgetByteArray){
//        if(value.is_array() && value.size() > 0){
//            auto ba = value.get<ByteArray>();
//            control->setRawData(&bt);

////            auto itr = m_user_data.find(tree::RepresentationRole);
////            if(itr != m_user_data.end()){
////                auto it = itr.value().find(key);
////                if(it != itr.value().end()){
////                    control->setText(it.value().toString());
////                }else{
////                    control->setText("<бинарные данные>");
////                }
////                auto bt = value.get<ByteArray>();
////                control->setRawData(&bt);
////            }else
////                control->setText("<бинарные данные>");
//        }
//    }else if(control->role() == widgetArray){
//        if(value.is_array() && value.size() > 0){
//            control->setData(QVariant(arcirk::tree::to_string_list(value)));
//        }
//    }

    control->setProperty("class", "VariatBox");
    control->setObjectName(key);
    control->enableLabelFrame(true);

    auto lbl = new QLabel(this);
    lbl->setText(fieldAlias(key) + ":");
    lbl->setObjectName(QString(key) + "_LBL");

    connect(control, &TreeItemVariant::itemValueChanged, this, &RowDialog::onVariantValueChanged);
    connect(control, &TreeItemVariant::innerRoleChanged, this, &RowDialog::onInnerRoleChanged);
    connect(control, &TreeItemVariant::itemTypeClear, this, &RowDialog::onItemTypeClear);

    control->reset();

    return QList<QWidget*>{control, lbl};
}

item_editor_widget_roles RowDialog::widgetRole(const QString &key)
{
    auto itr = m_user_data.find(tree::user_role::WidgetRole);
    if(itr != m_user_data.end()){
        auto it = itr.value().find(key);
        if(it != itr.value().end()){
            if(it.value().isValid())
                return (item_editor_widget_roles)it.value().toInt();
        }
    }

    return item_editor_widget_roles::widgetINVALID;
}

bool RowDialog::widgetNotNull(const QString &key)
{
    auto itr = m_user_data.find(tree::user_role::NotNullRole);
    if(itr != m_user_data.end()){
        auto it = itr.value().find(key);
        if(it != itr.value().end()){
            return it.value().toBool();
        }
    }

    return false;
}

QVariant RowDialog::widgetExtRole(const QString &key)
{
    auto itr = m_user_data.find(tree::user_role::UserRoleExt);
    if(itr != m_user_data.end()){
        auto it = itr.value().find(key);
        if(it != itr.value().end()){
            return it.value();
        }
    }

    return QVariant();
}

tree_editor_inner_role RowDialog::widgetInnerRole(const QString &key)
{
    auto itr = m_user_data.find(tree::user_role::WidgetInnerRole);
    if(itr != m_user_data.end()){
        auto it = itr.value().find(key);
        if(it != itr.value().end()){
            if(it.value().isValid())
                return (tree_editor_inner_role)it.value().toInt();
        }
    }

    return tree_editor_inner_role::widgetText;
}

attribute_use RowDialog::widgetUseRole(const QString &key)
{
    auto itr = m_user_data.find(tree::user_role::UseRole);
    if(itr != m_user_data.end()){
        auto it = itr.value().find(key);
        if(it != itr.value().end()){
            if(it.value().isValid())
                return (attribute_use)it.value().toInt();
        }
    }

    return attribute_use::forFolderAndItem;
}

void RowDialog::addWidget(const QString& key, int row, QList<QWidget *> control, bool visible)
{

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

    m_widgets.insert(key, control);
}

void RowDialog::set_user_data(user_role role, const QString &column, const QVariant &value)
{
    auto itr = m_user_data.find(role);
    if(itr != m_user_data.end()){
        auto it = itr.value().find(column);
        if(it != itr.value().end()){
            itr.value()[column] = value;
        }
    }
}

QList<QWidget *> RowDialog::createTextEdit(const QString &key, const json &value)
{
    QString val;
    if(value.is_string())
        val = value.get<std::string>().c_str();
    auto control = new TreeItemTextEdit(0,0, this);
    control->setText(val);
    if(key == "ref" || key == "parent"){
        control->setDefaultValue(NIL_STRING_UUID);
        if(key == "parent"){
            control->isClearButton(true);
            control->isReadOnly(true);
            m_parentRef = val;
            control->setValue(m_parentRef);
            control->setSynonim(m_parentSynonim);
            control->enableClearBottom(!is_new_element);
        }
    }

    control->setExtValue(widgetExtRole(key));
    control->setProperty("class", "QTextEdit");
    control->setObjectName(key);
    control->isBorder(true);
    control->setSpacing(1);
    control->setAutoMarkIncomplete(widgetNotNull(key));
    control->setRole(widgetInnerRole(key));

    auto lbl = new QLabel(this);
    lbl->setText(fieldAlias(key) + ":");
    lbl->setObjectName(QString(key) + "_LBL");

    connect(control, &TreeItemTextEdit::textChanged, this, &RowDialog::onTextControlDataChanged);
    connect(control, &TreeItemTextEdit::valueChanged, this, &RowDialog::onValueChanged);

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
            if(obj_name != "parent"){
                auto ctrl = qobject_cast<TreeItemTextLine*>(w);
                if(ctrl!=0 && m_data.find(obj_name.toStdString()) != m_data.end()){
                    m_data[obj_name.toStdString()] = ctrl->text().toStdString();
                }
            }else
                m_data["parent"] = m_parentRef.toStdString();
        }else if(cls == "QTextEdit"){
            if(obj_name != "parent"){
                auto ctrl = qobject_cast<TreeItemTextEdit*>(w);
                if(ctrl!=0 && m_data.find(obj_name.toStdString()) != m_data.end()){
                    m_data[obj_name.toStdString()] = ctrl->text().toStdString();
                }
            }else
                m_data["parent"] = m_parentRef.toStdString();
        }else if(cls == "QComboBox"){
            auto ctrl = qobject_cast<QComboBox*>(w);
            if(ctrl!=0 && m_data.find(obj_name.toStdString()) != m_data.end()){
                m_data[obj_name.toStdString()] = ctrl->currentText().toStdString();
            }
        }
    }
}

void RowDialog::onValueChanged(const QVariant &value)
{
    auto w = sender();
    if(w){
        auto obj_name = w->objectName();
        if(obj_name == "parent")
            m_parentRef = value.toString();
    }
}

void RowDialog::onTextControlDataChanged()
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
            if(obj_name != "parent"){
                auto ctrl = qobject_cast<TreeItemTextLine*>(w);
                if(ctrl!=0 && m_data.find(obj_name.toStdString()) != m_data.end()){
                    m_data[obj_name.toStdString()] = ctrl->text().toStdString();
                }
            }else
                m_data["parent"] = m_parentRef.toStdString();
        }else if(cls == "QTextEdit"){
            if(obj_name != "parent"){
                auto ctrl = qobject_cast<TreeItemTextEdit*>(w);
                if(ctrl!=0 && m_data.find(obj_name.toStdString()) != m_data.end()){
                    m_data[obj_name.toStdString()] = ctrl->text().toStdString();
                }
            }else
                m_data["parent"] = m_parentRef.toStdString();
        }else if(cls == "QComboBox"){
            auto ctrl = qobject_cast<QComboBox*>(w);
            if(ctrl!=0 && m_data.find(obj_name.toStdString()) != m_data.end()){
                m_data[obj_name.toStdString()] = ctrl->currentText().toStdString();
            }
        }
    }
}

void RowDialog::onVariantValueChanged(int row, int column, const QVariant& /*value*/)
{
    auto w = sender();
    if(w){
        auto obj_name = w->objectName();
        auto ctrl = qobject_cast<TreeItemVariant*>(w);
        if(ctrl!=0 && m_data.find(obj_name.toStdString()) != m_data.end()){
            set_user_data(tree::RepresentationRole, obj_name, ctrl->text());
            auto raw = ctrl->data().toByteArray();
            auto ba = ByteArray(sizeof(arcirk::synchronize::variant_p));
            std::copy(raw.begin(), raw.end(), ba.begin());
            m_data[obj_name.toStdString()] = ba;

//            if(ctrl->role() == widgetFilePath ||
//                ctrl->role() == widgetDirectoryPath ||
//                ctrl->role() == widgetText){
//                m_data[obj_name.toStdString()] = value.toString().toStdString();
//                set_user_data(tree::RepresentationRole, obj_name, ctrl->text());
//            }else if(ctrl->role() == widgetColor){

//            }else if(ctrl->role() == widgetByteArray){
////                ByteArray ba{};
////                if(ctrl->rawData()->size() > 0){
////                    ba = ByteArray(ctrl->rawData()->size());
////                    std::copy(ctrl->rawData()->begin(), ctrl->rawData()->end(), ba.begin());
////                    m_data[obj_name.toStdString()] = ba;
////                }
//                set_user_data(tree::RepresentationRole, obj_name, ctrl->text());
//            }else if(ctrl->role() == widgetInteger){
//               // m_data[obj_name.toStdString()] = value.toInt();
//            }else if(ctrl->role() == widgetArray){
////                json m_value = json::array();
////                auto v = value.toStringList();
////                foreach (auto itr, v) {
////                    m_value += itr.toStdString();
////                }
////                m_data[obj_name.toStdString()] = m_value;
//            }else
//                m_data[obj_name.toStdString()] = "";

        }

    }
}

void RowDialog::onInnerRoleChanged(int row, int column, const tree_editor_inner_role &value)
{
    auto w = sender();
    if(w){
        auto obj_name = w->objectName();
        if(m_inner_roles.find(obj_name) != m_inner_roles.end())
            m_inner_roles[obj_name] = value;
    }
}

void RowDialog::onItemTypeClear(int row, int column)
{
    auto w = sender();
    if(w){
        auto obj_name = w->objectName();
        if(m_inner_roles.find(obj_name) != m_inner_roles.end()){
            m_inner_roles[obj_name] = tree::widgetInnerRoleINVALID;
            m_data[obj_name.toStdString()] = {};
        }
    }
}

#endif
