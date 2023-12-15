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

    //m_raw_data = {};
    m_raw = arcirk::synchronize::variant_p();
    m_raw.type = (int)arcirk::synchronize::vNull;

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
    m_raw.representation = text.toStdString();
    if(m_role == widgetArray || m_role == widgetByteArray){
        m_label->setText(text);
    }else
        m_text->setText(text);
}

QString TreeItemVariant::text() const
{
    //QString value = "";
////    json state = m_current_state.toString().toStdString();
////    auto inner_state = state.get<tree_editor_inner_role>();
//    if(m_role == widgetText){
//        value = m_text->text();
//    }else if(m_role == widgetArray){
//        value = m_label->text();
//    }else if(m_role == widgetByteArray){
//        value = m_label->text();
//    }else if(m_role == widgetInteger){
//        value = QString::number(m_integer->value());
//    }

    return m_raw.representation.c_str();
}

void TreeItemVariant::setCurrentState(const QVariant &state)
{
    //qDebug() << __FUNCTION__;
    m_current_state = state;
    reset_state(m_current_state.toString());
}

void TreeItemVariant::setData(const QVariant &data)
{
    auto ba = data.toByteArray();
    if(ba.size() == sizeof(arcirk::synchronize::variant_p)){
        const auto r = reinterpret_cast<arcirk::synchronize::variant_p*>(ba.data());
        m_raw.representation = r->representation;
        m_raw.type = r->type;
        m_raw.data = ByteArray(r->data.size());
        std::copy(r->data.begin(), r->data.end(), m_raw.data.begin());
    }

}

void TreeItemVariant::setData(const json &data)
{
    using namespace arcirk::synchronize;
    try {
        auto ba = data.get<ByteArray>();
        auto const ptr = reinterpret_cast<variant_p*>(ba.data());
        if(ptr){
            m_raw.representation = ptr->representation;
            m_raw.type = ptr->type;
            if(ptr->data.size() > 0){
                m_raw.data = ByteArray(ptr->data.size());
                std::copy(ptr->data.begin(), ptr->data.end(), m_raw.data.begin());
            }
        }
    } catch (...) {
    }

}

QVariant TreeItemVariant::data()
{
    const auto r = reinterpret_cast<char*>(&m_raw);
    return QByteArray::fromRawData(r, sizeof(arcirk::synchronize::variant_p));
}

//ByteArray *TreeItemVariant::rawData()
//{
////    const auto ptr = reinterpret_cast<ByteArray*>(&m_raw);
////    return ptr;
//    //const auto r = reinterpret_cast<char*>(&m_raw);
//    auto raw = data().toByteArray();
//    auto ba = ByteArray(sizeof(arcirk::synchronize::variant_p));
//    std::copy(raw.begin(), raw.end(), ba.begin());
//    return &ba;

//}

void TreeItemVariant::setRawData(ByteArray *data)
{
    const auto raw = reinterpret_cast<arcirk::synchronize::variant_p*>(data->data());
    if(raw->data.size() > 0){
        qDebug() << raw->data.size();
        m_raw.data = ByteArray(raw->data.size());
        std::copy(raw->data.begin(), raw->data.end(), m_raw.data.begin());
    }

    m_raw.representation = raw->representation;
    m_raw.type = raw->type;
    m_role = (tree_editor_inner_role)m_raw.type;
    reset_state(enum_synonym(m_role).c_str());
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
        m_raw.type = (int)arcirk::synchronize::vJsonDump;
    }else if(inner_state == widgetArray){
        m_label->setVisible(true);
        m_text->setVisible(false);
        m_sel_type->setVisible(false);
        m_selelect->setVisible(true);
        m_erase->setVisible(true);
        m_integer->setVisible(false);
        m_save->setVisible(false);
        m_list = m_current_value.toStringList();
        m_raw.type = (int)arcirk::synchronize::vJsonDump;
    }else if(inner_state == widgetByteArray){
        m_label->setVisible(true);
        m_text->setVisible(false);
        m_sel_type->setVisible(false);
        m_selelect->setVisible(true);
        m_erase->setVisible(true);
        m_integer->setVisible(false);
        m_save->setVisible(true);
        m_raw.type = (int)arcirk::synchronize::vBinary;
    }else if(inner_state == widgetInteger){
        m_label->setVisible(false);
        m_text->setVisible(false);
        m_sel_type->setVisible(false);
        m_selelect->setVisible(false);
        m_erase->setVisible(true);
        m_integer->setVisible(true);
        m_save->setVisible(false);
        m_integer->setValue(m_current_value.toInt());
        m_raw.type = (int)arcirk::synchronize::vJsonDump;
    }else{
        m_label->setVisible(true);
        m_text->setVisible(false);
        m_sel_type->setVisible(true);
        m_selelect->setVisible(false);
        m_erase->setVisible(false);
        m_integer->setVisible(false);
        m_save->setVisible(false);
        m_label->setText("");
        m_raw.type = (int)arcirk::synchronize::vNull;
    }

    m_current_state = state;
    if(inner_state != widgetInnerRoleINVALID){
        m_role = inner_state;
        emit innerRoleChanged(row(), column(), m_role);
    }else{
        m_role = widgetText;
        emit innerRoleChanged(row(), column(), m_role);
    }

    m_raw.type = (int)inner_state;
}

void TreeItemVariant::generateRaw(const std::string& rep, ByteArray* data)
{
    m_raw.representation = rep;
    m_raw.data = ByteArray(data->size());
    std::copy(data->begin(), data->end(), m_raw.data.begin());
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
            auto str = arcirk::string_to_byte_array(result.dump());
            generateRaw(m_label->text().toStdString(), &str);
            emit itemValueChanged(row(), column(), m_current_value);
        }
    }else if(m_role == widgetByteArray){
        auto result = QFileDialog::getOpenFileName(this, "Выбор файла");
        if(!result.isEmpty()){
            QFileInfo info(result);
            m_label->setText(info.fileName());
            ByteArray bt{};
            arcirk::read_file(result.toStdString(), bt);
            generateRaw(info.fileName().toStdString(), &bt);
            emit itemValueChanged(row(), column(), m_current_value);
        }
    }
}

void TreeItemVariant::onSaveClicked()
{
    if(m_role == widgetByteArray && m_raw.data.size() > 0){
        auto result = QFileDialog::getSaveFileName(this, "Сохранить как...", m_raw.representation.c_str());
        if(!result.isEmpty()){
            arcirk::write_file(m_raw.representation, m_raw.data);
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
    json v = value;
    auto bt = arcirk::string_to_byte_array(v.dump());
    generateRaw(QString::number(value).toStdString(), &bt);
    emit itemValueChanged(row(), column(), m_current_value);
}

void TreeItemVariant::onTextChanged(const QString &value)
{
    m_current_value = value;
    json v = value.toStdString();
    auto bt = arcirk::string_to_byte_array(v.dump());
    generateRaw(value.toStdString(), &bt);
    emit itemValueChanged(row(), column(), m_current_value);
}

#endif
