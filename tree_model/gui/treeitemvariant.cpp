#ifndef IS_OS_ANDROID
#include "treeitemvariant.h"
//#include "qbitarray.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include "gui/araydialog.h"
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>

using namespace arcirk::tree::widgets;
using namespace arcirk::tree_widget;

TreeItemVariant::TreeItemVariant(int row, int column, QWidget *parent)
    : TreeItemWidget{row, column, parent}
{

    Q_INIT_RESOURCE(resurce);

    m_raw_data = {};

    auto hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(0,0,0,0);
    hbox->setSpacing(0);

    m_label = new QLabel(this);
    m_label->setObjectName("libel");
    hbox->addWidget(m_label);
    m_text = new QLineEdit(this);
    m_text->setObjectName("lineEdit");
    hbox->addWidget(m_text);
    m_text->setVisible(false);
    m_integer = new QSpinBox(this);
    m_integer->setObjectName("numberEdit");
    hbox->addWidget(m_integer);
    m_integer->setMaximum(9999);
    m_integer->setAlignment(Qt::AlignRight);
    m_integer->setVisible(false);

    m_sel_type= new QToolButton(this);
    m_sel_type->setObjectName("selectType");
    m_sel_type->setIcon(QIcon(":/img/select_type.png"));
    m_sel_type->setPopupMode(QToolButton::InstantPopup);
    auto mnu = new QMenu(m_sel_type);
    auto action = new QAction("Текст", this);
    action->setObjectName(enum_synonym(tree_editor_inner_role::widgetText).c_str());
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    mnu->addAction(action);
    action = new QAction("Число", this);
    action->setObjectName(enum_synonym(tree_editor_inner_role::widgetInteger).c_str());
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    mnu->addAction(action);
    action = new QAction("Список значений", this);
    action->setObjectName(enum_synonym(tree_editor_inner_role::widgetArray).c_str());
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    mnu->addAction(action);
    action = new QAction("Файл", this);
    action->setObjectName(enum_synonym(tree_editor_inner_role::widgetByteArray).c_str());
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    mnu->addAction(action);
    m_sel_type->setMenu(mnu);
    hbox->addWidget(m_sel_type);

    m_selelect = new QToolButton(this);
    m_selelect->setText("...");
    m_selelect->setObjectName("selectButton");
    hbox->addWidget(m_selelect);
    m_selelect->setVisible(false);
    m_save = new QToolButton(this);
    m_save->setIcon(QIcon(":/img/save.png"));
    m_save->setObjectName("saveButton");
    hbox->addWidget(m_save);
    m_save->setVisible(false);
    m_erase = new QToolButton(this);
    m_erase->setObjectName("eraseButton");
    m_erase->setIcon(QIcon(":/img/delete.png"));
    hbox->addWidget(m_erase);
    m_erase->setVisible(false);
    setLayout(hbox);

    connect(m_selelect, &QToolButton::clicked, this, &TreeItemVariant::onSelectClicked);
    connect(m_save, &QToolButton::clicked, this, &TreeItemVariant::onSaveClicked);
    connect(m_erase, &QToolButton::clicked, this, &TreeItemVariant::onEraseClicked);
    connect(m_integer, &QSpinBox::valueChanged, this, &TreeItemVariant::onSpinChanged);
    connect(m_text, &QLineEdit::textChanged, this, &TreeItemVariant::onTextChanged);

    setCurrentState(enum_synonym(tree_editor_inner_role::widgetNullType).c_str());
}

bool TreeItemVariant::isChecked()
{
    return false;
}

void TreeItemVariant::setChecked(bool value)
{
    Q_UNUSED(value);
}

void TreeItemVariant::setText(const QString &text)
{
    if(m_role == widgetArray || m_role == widgetByteArray){
        m_label->setText(text);
    }else
        m_text->setText(text);
}

QString TreeItemVariant::text() const
{
    QString value = "";
//    json state = m_current_state.toString().toStdString();
//    auto inner_state = state.get<tree_editor_inner_role>();
    if(m_role == widgetText){
        value = m_text->text();
    }else if(m_role == widgetArray){
        value = m_label->text();
    }else if(m_role == widgetByteArray){
        value = m_label->text();
    }else if(m_role == widgetInteger){
        value = QString::number(m_integer->value());
    }
    return value;
}

void TreeItemVariant::setCurrentState(const QVariant &state)
{
    //qDebug() << __FUNCTION__;
    m_current_state = state;
    reset_state(m_current_state.toString());
}

void TreeItemVariant::setData(const QVariant &data)
{
    //qDebug() << __FUNCTION__;
    m_current_value = data;
    if(m_current_value.typeId() == QMetaType::QStringList){
        m_list = m_current_value.toStringList();
        setText(m_list.join(", "));
    }
}

QVariant TreeItemVariant::data()
{
    //qDebug() << __FUNCTION__ << m_current_value;
    return m_current_value;
}

ByteArray *TreeItemVariant::rawData()
{
    return &m_raw_data;
}

void TreeItemVariant::setRawData(ByteArray *data)
{
    m_raw_data = ByteArray(data->size());
    std::copy(data->begin(), data->end(), m_raw_data.begin());
}

void TreeItemVariant::enableLabelFrame(bool value)
{
    if(m_label==0)
        return;

    if(value){
        m_label->setStyleSheet("QLabel {"
                      "border-style: solid;"
                      "border-width: 1px;"
                      "border-color: grey; "
                      "}");
    }else
        m_label->setStyleSheet("");
}

void TreeItemVariant::reset()
{
    if(m_role != widgetInnerRoleINVALID){
        reset_state(enum_synonym(m_role).c_str());
    }

}

void TreeItemVariant::reset_state(const QString &state)
{
    json state_ = state.toStdString();
    auto inner_state = state_.get<tree_editor_inner_role>();

    if(inner_state == widgetText){
        m_label->setVisible(false);
        m_text->setVisible(true);
        m_sel_type->setVisible(false);
        m_selelect->setVisible(true);
        m_erase->setVisible(true);
        m_integer->setVisible(false);
        m_save->setVisible(false);
    }else if(inner_state == widgetArray){
        m_label->setVisible(true);
        m_text->setVisible(false);
        m_sel_type->setVisible(false);
        m_selelect->setVisible(true);
        m_erase->setVisible(true);
        m_integer->setVisible(false);
        m_save->setVisible(false);
        m_list = m_current_value.toStringList();
    }else if(inner_state == widgetByteArray){
        m_label->setVisible(true);
        m_text->setVisible(false);
        m_sel_type->setVisible(false);
        m_selelect->setVisible(true);
        m_erase->setVisible(true);
        m_integer->setVisible(false);
        m_save->setVisible(true);
    }else if(inner_state == widgetInteger){
        m_label->setVisible(false);
        m_text->setVisible(false);
        m_sel_type->setVisible(false);
        m_selelect->setVisible(false);
        m_erase->setVisible(true);
        m_integer->setVisible(true);
        m_save->setVisible(false);
        m_integer->setValue(m_current_value.toInt());
    }else{
        m_label->setVisible(true);
        m_text->setVisible(false);
        m_sel_type->setVisible(true);
        m_selelect->setVisible(false);
        m_erase->setVisible(false);
        m_integer->setVisible(false);
        m_save->setVisible(false);
        m_label->setText("");
    }

    m_current_state = state;
    if(inner_state != widgetInnerRoleINVALID){
        //setRole(inner_state);
        m_role = inner_state;
        emit innerRoleChanged(row(), column(), m_role);
    }else{
        //setRole(widgetText);
        m_role = widgetText;
        emit innerRoleChanged(row(), column(), m_role);
    }
    //qDebug() << __FUNCTION__ << state;
}

void TreeItemVariant::onMenuItemClicked()
{
    auto mnu = qobject_cast<QAction*>(sender());
    if(mnu){
        reset_state(mnu->objectName());
    }
}

void TreeItemVariant::onSelectClicked()
{

    if(m_role == widgetArray){
        auto dlg = ArrayDialog(arcirk::tree_model::to_json(m_list), this);
        if(dlg.exec()){
            auto result = dlg.result();
            m_list = arcirk::tree::to_string_list(result);
            m_current_value = QVariant(m_list);
            m_label->setText(m_list.join(","));
            emit itemValueChanged(row(), column(), m_current_value);
        }
    }else if(m_role == widgetByteArray){
        auto result = QFileDialog::getOpenFileName(this, "Выбор файла");
        if(!result.isEmpty()){
            //QFile f(result);
//            if(f.open(QIODevice::ReadOnly)){
//                m_current_value = f.readAll();
//                f.close();
//                QFileInfo info(result);
//                m_label->setText(info.fileName());
//                emit itemValueChanged(row(), column(), m_current_value);
//            }
//            ByteArray ba{};
            m_raw_data.clear();
            arcirk::read_file(result.toStdString(), m_raw_data);
            QFileInfo info(result);
            m_label->setText(info.fileName());
            emit itemValueChanged(row(), column(), m_current_value);
        }
    }
}

void TreeItemVariant::onSaveClicked()
{
    if(m_role == widgetByteArray && m_raw_data.size() > 0){
        auto result = QFileDialog::getSaveFileName(this, "Сохранить как...", text());
        if(!result.isEmpty()){
            arcirk::write_file(result.toStdString(), m_raw_data);
        }
    }
}

void TreeItemVariant::onEraseClicked()
{
    m_label->setVisible(true);
    m_text->setVisible(false);
    m_sel_type->setVisible(true);
    m_selelect->setVisible(false);
    m_erase->setVisible(false);
    m_integer->setVisible(false);
    setRole(tree_editor_inner_role::widgetNullType);
    setCurrentState(enum_synonym(tree_editor_inner_role::widgetNullType).c_str());
    emit itemTypeClear(row(), column());
}

void TreeItemVariant::onSpinChanged(int value)
{
    m_current_value = value;
    emit itemValueChanged(row(), column(), m_current_value);
}

void TreeItemVariant::onTextChanged(const QString &value)
{
    m_current_value = value;
    emit itemValueChanged(row(), column(), m_current_value);
}

#endif
