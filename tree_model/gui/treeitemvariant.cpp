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
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <alpaca/alpaca.h>

using namespace arcirk::tree::widgets;
using namespace arcirk::tree_widget;

TreeItemVariant::TreeItemVariant(int row, int column, QWidget *parent)
    : TreeItemWidget{row, column, parent}
{

    Q_INIT_RESOURCE(resurce);

    m_raw = arcirk::synchronize::variant_p_();
    m_raw.type = (int)arcirk::synchronize::vNull;
    m_role = arcirk::tree::tree_editor_inner_role::widgetNullType;
    m_raw.role = m_role;
    m_label_frame = false;
    m_current_widget = nullptr;

    m_hbox = new QHBoxLayout(this);
    m_hbox->setContentsMargins(0,0,0,0);
    m_hbox->setSpacing(0);

    m_current_menu = new QMenu(this);
    auto action = new QAction("Текст", this);
    action->setObjectName(enum_synonym(tree_editor_inner_role::widgetText).c_str());
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    m_current_menu->addAction(action);
    action = new QAction("Число", this);
    action->setObjectName(enum_synonym(tree_editor_inner_role::widgetInteger).c_str());
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    m_current_menu->addAction(action);
    action = new QAction("Список значений", this);
    action->setObjectName(enum_synonym(tree_editor_inner_role::widgetArray).c_str());
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    m_current_menu->addAction(action);
    action = new QAction("Файл", this);
    action->setObjectName(enum_synonym(tree_editor_inner_role::widgetByteArray).c_str());
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    m_current_menu->addAction(action);
    action = new QAction("Путь к каталогу", this);
    action->setObjectName(enum_synonym(tree_editor_inner_role::widgetDirectoryPath).c_str());
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    m_current_menu->addAction(action);
    action = new QAction("Путь к файлу", this);
    action->setObjectName(enum_synonym(tree_editor_inner_role::widgetFilePath).c_str());
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    m_current_menu->addAction(action);

    setLayout(m_hbox);

    createEditor();
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
    update();
}

QString TreeItemVariant::text() const
{
    return m_raw.representation.c_str();
}

void TreeItemVariant::setCurrentState(const QVariant &/*state*/)
{

}

void TreeItemVariant::setRole(tree_editor_inner_role role)
{
    m_raw = arcirk::synchronize::variant_p_();
    m_raw.role = role;
    m_role = role;
    setControlType();
    createEditor();
    emit innerRoleChanged(row(), column(), m_role);
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

    std::error_code ec;
    auto ba = data.get<ByteArray>();
    m_raw = alpaca::deserialize<variant_p>(ba, ec);
    if (!ec) {
        // use object
        m_role = (tree_editor_inner_role)m_raw.role;
        createEditor();
        update();
    }

}

QVariant TreeItemVariant::data()
{
    ByteArray bytes;
    alpaca::serialize(m_raw, bytes);
    return QByteArray(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

void TreeItemVariant::enableLabelFrame(bool value)
{
    m_label_frame = value;
    auto m_label = qobject_cast<QLabel*>(m_current_widget);
    if(!m_label)
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

void TreeItemVariant::update()
{
    using namespace arcirk::synchronize;

    if(m_raw.data.size() == 0 || !m_current_widget)
        return;

    auto type = (arcirk::synchronize::variant_type)m_raw.type;
    auto role = (tree_editor_inner_role)m_raw.role;

    if(type == vJsonDump){
        auto sval = arcirk::byte_array_to_string(m_raw.data);
        if(!json::accept(sval))
            return;

        auto jval = json::parse(sval);
        if(role == widgetText){
            auto m_text = qobject_cast<QLineEdit*>(m_current_widget);
            if(m_text){
                if(jval.is_string()){
                    m_text->setText(jval.get<std::string>().c_str());
                    m_raw.representation = m_text->text().toStdString();
                }else{
                    m_text->setText("");
                    m_raw.representation = "" ;
                }
            }
        }else if(role == widgetMultiText || role == widgetDirectoryPath || role == widgetFilePath){
            auto m_edit = qobject_cast<QTextEdit*>(m_current_widget);
            if(m_edit){
                if(jval.is_string()){
                    m_edit->setPlainText(jval.get<std::string>().c_str());
                    m_raw.representation = m_edit->toPlainText().toStdString();
                }else{
                    m_edit->setPlainText("");
                    m_raw.representation = "" ;
                }
            }
        }else if(role == widgetArray){
            if(jval.is_array()){
                auto list = arcirk::tree_model::to_variant(jval);
                if(list.isValid()){
                    auto val = list.toStringList().join(", ");
                    auto m_label = qobject_cast<QLabel*>(m_current_widget);
                    if(m_label){
                        m_label->setText(val);
                        m_raw.representation = val.toStdString();
                    }
                }
            }
        }else if(role == widgetInteger){
            int val = 0;
            if(jval.is_number()){
                val = jval.get<int>();
            }
            auto m_integer = qobject_cast<QSpinBox*>(m_current_widget);
            if(m_integer)
                m_integer->setValue(val);
            m_raw.representation = QString::number(m_integer->value()).toStdString();
        }
    }else if(type == vBinary){
        if(role == widgetByteArray){
            auto m_label = qobject_cast<QLabel*>(m_current_widget);
            if(m_raw.representation.empty()){
                if(m_label)
                    m_label->setText("<Бинарные данные>");
            }else
                m_label->setText(m_raw.representation.c_str());
        }
    }

}

void TreeItemVariant::generateRaw(const std::string& rep, ByteArray* data)
{
    m_raw.representation = rep;
    m_raw.data = ByteArray(data->size());
    std::copy(data->begin(), data->end(), m_raw.data.begin());
}

void TreeItemVariant::createEditor()
{
    clearLayout(m_hbox);

    if(m_role == tree_editor_inner_role::widgetInnerRoleINVALID || m_role == tree_editor_inner_role::widgetNullType){
        m_current_widget = createEditorNull();
    }else if(m_role == tree_editor_inner_role::widgetArray || m_role == tree_editor_inner_role::widgetByteArray){
        m_current_widget = createEditorLabel(m_role == tree_editor_inner_role::widgetByteArray);
    }else if(m_role == tree_editor_inner_role::widgetText){
        m_current_widget = createEditorTextLine();
    }else if(m_role == tree_editor_inner_role::widgetMultiText || m_role == tree_editor_inner_role::widgetDirectoryPath || m_role == tree_editor_inner_role::widgetFilePath){
        m_current_widget = createEditorTextBox();
    }else if(m_role == tree_editor_inner_role::widgetInteger){
        m_current_widget = createEditorNumber();
    }

}

QWidget *TreeItemVariant::createEditorNull()
{
    auto widget = new QLabel(this);
    widget->setObjectName("label");
    m_hbox->addWidget(widget);
    auto m_sel_type= new QToolButton(this);
    m_sel_type->setObjectName("selectType");
    m_sel_type->setIcon(QIcon(":/img/select_type.png"));
    m_sel_type->setPopupMode(QToolButton::InstantPopup);
    m_sel_type->setMenu(m_current_menu);
    m_hbox->addWidget(m_sel_type);
    return widget;
}

QWidget *TreeItemVariant::createEditorLabel(bool save)
{
    auto widget = new QLabel(this);
    widget->setObjectName("label");
    m_hbox->addWidget(widget);
    auto m_selelect = new QToolButton(this);
    m_selelect->setText("...");
    m_selelect->setObjectName("selectButton");
    m_selelect->setToolTip("Выбрать");
    m_hbox->addWidget(m_selelect);
    if(save){
        auto m_save = new QToolButton(this);
        m_save->setIcon(QIcon(":/img/save.png"));
        m_save->setObjectName("saveButton");
        m_save->setToolTip("Сохранить как ...");
        m_hbox->addWidget(m_save);
        connect(m_save, &QToolButton::clicked, this, &TreeItemVariant::onSaveClicked);
    }
    auto m_erase = new QToolButton(this);
    m_erase->setObjectName("eraseButton");
    m_erase->setIcon(QIcon(":/img/erase_type.png"));
    m_erase->setToolTip("Удалить тип");
    m_hbox->addWidget(m_erase);
    connect(m_selelect, &QToolButton::clicked, this, &TreeItemVariant::onSelectClicked);
    connect(m_erase, &QToolButton::clicked, this, &TreeItemVariant::onEraseClicked);
    return widget;
}

QWidget *TreeItemVariant::createEditorTextLine()
{
    auto widget = new QLineEdit(this);
    widget->setObjectName("LineEdit");
    m_hbox->addWidget(widget);
    auto m_selelect = new QToolButton(this);
    m_selelect->setText("...");
    m_selelect->setObjectName("selectButton");
    m_selelect->setToolTip("Выбрать");
    m_hbox->addWidget(m_selelect);
    auto m_erase = new QToolButton(this);
    m_erase->setObjectName("eraseButton");
    m_erase->setIcon(QIcon(":/img/erase_type.png"));
    m_erase->setToolTip("Удалить тип");
    m_hbox->addWidget(m_erase);
    connect(m_selelect, &QToolButton::clicked, this, &TreeItemVariant::onSelectClicked);
    connect(m_erase, &QToolButton::clicked, this, &TreeItemVariant::onEraseClicked);
    connect(widget, &QLineEdit::textChanged, this, &TreeItemVariant::onTextChanged);
    return widget;
}

QWidget *TreeItemVariant::createEditorTextBox()
{
    auto widget = new QTextEdit(this);
    widget->setObjectName("TextEdit");
    m_hbox->addWidget(widget);
    auto m_selelect = new QToolButton(this);
    m_selelect->setText("...");
    m_selelect->setObjectName("selectButton");
    m_selelect->setToolTip("Выбрать");
    m_hbox->addWidget(m_selelect);
    auto m_erase = new QToolButton(this);
    m_erase->setObjectName("eraseButton");
    m_erase->setIcon(QIcon(":/img/erase_type.png"));
    m_erase->setToolTip("Удалить тип");
    m_hbox->addWidget(m_erase);
    m_hbox->setAlignment(Qt::AlignmentFlag::AlignTop);
    connect(m_selelect, &QToolButton::clicked, this, &TreeItemVariant::onSelectClicked);
    connect(m_erase, &QToolButton::clicked, this, &TreeItemVariant::onEraseClicked);
    connect(widget, &QTextEdit::textChanged, this, &TreeItemVariant::onTextEditChanged);
    return widget;
}

QWidget *TreeItemVariant::createEditorNumber()
{
    auto m_integer = new QSpinBox(this);
    m_integer->setObjectName("SpinBox");
    m_integer->setMaximum(9999);
    m_integer->setAlignment(Qt::AlignRight);
    m_hbox->addWidget(m_integer);
    auto m_erase = new QToolButton(this);
    m_erase->setObjectName("eraseButton");
    m_erase->setIcon(QIcon(":/img/erase_type.png"));
    m_erase->setToolTip("Удалить тип");
    m_hbox->addWidget(m_erase);
    connect(m_erase, &QToolButton::clicked, this, &TreeItemVariant::onEraseClicked);
    connect(m_integer, &QSpinBox::valueChanged, this, &TreeItemVariant::onSpinChanged);
    return m_integer;
}

void TreeItemVariant::setControlType()
{
    m_raw.role = m_role;
    if(m_role == widgetInnerRoleINVALID || m_role == widgetNullType){
        m_raw.type = synchronize::vNull;
    }else if(m_role == widgetByteArray){
        m_raw.type = synchronize::vBinary;
    }else
        m_raw.type = synchronize::vJsonDump;
}



void TreeItemVariant::onMenuItemClicked()
{
    auto mnu = qobject_cast<QAction*>(sender());
    if(mnu){
        json r = mnu->objectName().toStdString();
        auto role = r.get<tree_editor_inner_role>();
        if(m_role != role){
            m_role = role;
            setControlType();
            createEditor();
            update();
            enableLabelFrame(m_label_frame);
        }
    }
}

void TreeItemVariant::onSelectClicked()
{

    Q_ASSERT(m_current_widget!=0);

    if(m_role == widgetArray){
        QStringList m_list{};
        auto sval = arcirk::byte_array_to_string(m_raw.data);
        if(json::accept(sval)){
            auto jval = json::parse(sval);
            if(jval.is_array()){
                m_list = arcirk::tree::to_string_list(jval);
            }
        }
        auto dlg = ArrayDialog(arcirk::tree_model::to_json(m_list), this);
        if(dlg.exec()){
            auto result = dlg.result();
            m_list = arcirk::tree::to_string_list(result);
            m_current_value = QVariant(m_list);
            auto m_label = qobject_cast<QLabel*>(m_current_widget);
            if(m_label){
                m_label->setText(m_list.join(","));
                auto str = arcirk::string_to_byte_array(result.dump());
                generateRaw(m_label->text().toStdString(), &str);
                emit itemValueChanged(row(), column(), m_current_value);
            }
        }
    }else if(m_role == widgetByteArray){
        auto result = QFileDialog::getOpenFileName(this, "Выбор файла");
        if(!result.isEmpty()){
            auto m_label = qobject_cast<QLabel*>(m_current_widget);
            if(m_label){
                QFileInfo info(result);
                m_label->setText(info.fileName());
                ByteArray bt{};
                arcirk::read_file(result.toStdString(), bt);
                generateRaw(info.fileName().toStdString(), &bt);
                emit itemValueChanged(row(), column(), m_current_value);
            }
        }
    }else if(m_role == widgetDirectoryPath){
        auto result = QFileDialog::getExistingDirectory(this, "Выбор директории");
        if(!result.isEmpty()){
            auto m_edit = qobject_cast<QTextEdit*>(m_current_widget);
            if(m_edit){
                m_edit->setText(result);
                m_current_value = result;
                emit itemValueChanged(row(), column(), m_current_value);
            }
        }
    }else if(m_role == widgetFilePath){
        auto result = QFileDialog::getOpenFileName(this, "Выбор файла");
        if(!result.isEmpty()){
            auto m_edit = qobject_cast<QTextEdit*>(m_current_widget);
            if(m_edit){
                m_edit->setText(result);
                m_current_value = result;
                emit itemValueChanged(row(), column(), m_current_value);
            }
        }
    }
}

void TreeItemVariant::onSaveClicked()
{
    if(m_role == widgetByteArray && m_raw.data.size() > 0){
        auto result = QFileDialog::getSaveFileName(this, "Сохранить как...", m_raw.representation.c_str());
        if(!result.isEmpty()){
            arcirk::write_file(result.toStdString(), m_raw.data);
        }
    }
}

void TreeItemVariant::onEraseClicked()
{
    setRole(tree_editor_inner_role::widgetNullType);
    enableLabelFrame(m_label_frame);
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

void TreeItemVariant::onTextEditChanged()
{
    auto control = qobject_cast<QTextEdit*>(sender());
    if(control){
        m_current_value = control->toPlainText();
        json v = m_current_value.toString().toStdString();
        auto bt = arcirk::string_to_byte_array(v.dump());
        generateRaw(m_current_value.toString().toStdString(), &bt);
        emit itemValueChanged(row(), column(), m_current_value);
    }

}

#endif
