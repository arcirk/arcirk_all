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
