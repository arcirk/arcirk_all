#ifndef IS_OS_ANDROID
#include "tabletoolbar.h"
#include "ui_tabletoolbar.h"
#include <QToolButton>

using namespace arcirk::tree_widget;

TableToolBar::TableToolBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TableToolBar)
{
    ui->setupUi(this);

    ui->btnAdd->setProperty("itemName", "add_item");
    ui->btnAddGroup->setProperty("itemName", "add_group");
    ui->btnDelete->setProperty("itemName", "delete_item");
    ui->btnEdit->setProperty("itemName", "edit_item");
    ui->btnMoveDown->setProperty("itemName", "move_down_item");
    ui->btnMoveUp->setProperty("itemName", "move_up_item");
    ui->btnMoveTo->setProperty("itemName", "move_to_item");


    connect(ui->btnAdd, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnAddGroup, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnDelete, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnEdit, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnMoveDown, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnMoveUp, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnMoveTo, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);

    m_bottons.insert("add_item", ui->btnAdd);
    m_bottons.insert("add_group", ui->btnAddGroup);
    m_bottons.insert("delete_item", ui->btnDelete);
    m_bottons.insert("edit_item", ui->btnEdit);
    m_bottons.insert("move_down_item", ui->btnMoveDown);
    m_bottons.insert("move_up_item", ui->btnMoveUp);
    m_bottons.insert("move_to_item", ui->btnMoveTo);
}

TableToolBar::~TableToolBar()
{
    delete ui;
}

void TableToolBar::setHierarchyState(bool state)
{
    ui->btnAddGroup->setEnabled(state);
}

void TableToolBar::setButtonEnabled(const QString &name, bool value)
{
    if(name == "add_item")
        ui->btnAdd->setEnabled(value);
    else if(name == "add_group")
        ui->btnAddGroup->setEnabled(value);
    else if(name == "delete_item")
        ui->btnDelete->setEnabled(value);
    else if(name == "edit_item")
        ui->btnEdit->setEnabled(value);
    else if(name == "move_down_item")
        ui->btnMoveDown->setEnabled(value);
    else if(name == "move_up_item")
        ui->btnMoveUp->setEnabled(value);
    else if(name == "move_to_item")
        ui->btnMoveTo->setEnabled(value);
}

void TableToolBar::setButtonVisible(const QString &name, bool value)
{
    if(name == "add_item")
        ui->btnAdd->setVisible(value);
    else if(name == "add_group")
        ui->btnAddGroup->setVisible(value);
    else if(name == "delete_item")
        ui->btnDelete->setVisible(value);
    else if(name == "edit_item")
        ui->btnEdit->setVisible(value);
    else if(name == "move_down_item")
        ui->btnMoveDown->setVisible(value);
    else if(name == "move_up_item")
        ui->btnMoveUp->setVisible(value);
    else if(name == "move_to_item")
        ui->btnMoveTo->setVisible(value);
}

void TableToolBar::setSeparatorVisible(int index, bool value)
{
    if(index == 0){
        ui->separator1->setVisible(value);
    }else if(index == 1){
        ui->separator2->setVisible(value);
    }
}

QToolButton *TableToolBar::button(const QString &name)
{
    auto itr = m_bottons.find(name);
    if(itr != m_bottons.end())
        return itr.value();

    return nullptr;
}

void TableToolBar::addButton(const QString &name, const QIcon &ico, bool checkable, int position)
{
    auto btn = new QToolButton(this);
    btn->setIcon(ico);
    btn->setCheckable(checkable);
    btn->setObjectName(name);
    btn->setAutoRaise(true);
    btn->setProperty("itemName", name);
    m_bottons.insert(name, btn);
    int pos = ui->horizontalLayout->count() - 1;
    if(position != -1)
        pos = position;
    ui->horizontalLayout->insertWidget( pos, btn);
    connect(btn, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
}

void TableToolBar::insertSeparator(int position)
{
    auto label = new QLabel(this);
    label->setText("");
    label->setFrameShape(QFrame::VLine);
    label->setFrameShadow(QFrame::Raised);
    int pos = ui->horizontalLayout->count() - 1;
    if(position != -1)
        pos = position;
    ui->horizontalLayout->insertWidget( pos, label);
}

void arcirk::tree_widget::TableToolBar::onHierarchyState(bool state)
{
    ui->btnAddGroup->setEnabled(state);
}

void TableToolBar::onTreeEnabled(bool value)
{
    this->setEnabled(value);
}

void TableToolBar::onButtonClicked()
{
    auto btn = qobject_cast<QToolButton*>(sender());
    if(btn)
        emit itemClicked(btn->property("itemName").toString());
}
#endif
