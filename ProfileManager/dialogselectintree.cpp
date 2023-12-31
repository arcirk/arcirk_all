#include "dialogselectintree.h"
#include "ui_dialogselectintree.h"
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QPushButton>

DialogSelectInTree::DialogSelectInTree(TreeItemModel* model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSelectInTree)
{
    ui->setupUi(this);

    treeView = new TreeViewWidget(this);
    ui->verticalLayout->addWidget(treeView);

    treeView->setModel(model);

    for (int i = 0; i < model->columnCount(QModelIndex()); ++i) {
        auto key = model->column_name(i);
        if(key != "name" && key != "size" && key != "first"){
            treeView->hideColumn(i);
        }
    }

    allow_sel_group_ = false;

    auto btn = ui->buttonBox->button(QDialogButtonBox::Cancel);
    if(btn)
        btn->setText("Отмена");

}

DialogSelectInTree::DialogSelectInTree(TreeItemModel *model, QVector<QString> hideColumns, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSelectInTree)
{
    ui->setupUi(this);

    treeView = new TreeViewWidget(this);
    ui->verticalLayout->addWidget(treeView);
    treeView->setModel(model);

    foreach (auto v, hideColumns) {
        auto i = model->column_index(v);
        if(i != -1)
            treeView->hideColumn(i);
    }

    allow_sel_group_ = false;

    auto btn = ui->buttonBox->button(QDialogButtonBox::Cancel);
    if(btn)
        btn->setText("Отмена");
}

DialogSelectInTree::~DialogSelectInTree()
{
    delete ui;
}

QString DialogSelectInTree::file_name() const
{
    return file_name_;
}

void DialogSelectInTree::accept()
{
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Элемент не выбран!");
        return;
    }

    auto model = (TreeItemModel*)treeView->get_model();
    auto in = model->column_index("path");
    if(in != -1)
        file_name_ = model->index(index.row(), in).data().toString();

    sel_object = model->to_object(index);

    if(!allow_sel_group_){
        auto is_find = sel_object.find("is_group") != sel_object.end();
        if(is_find){
            auto is_group = sel_object["is_group"].get<int>();
            if(is_group){
                QMessageBox::critical(this, "Ошибка", "Выберете элемент!");
                return;
            }
        }

    }

    QDialog::accept();
}

nlohmann::json DialogSelectInTree::selectedObject()
{
    return sel_object;
}

void DialogSelectInTree::allow_sel_group(bool value)
{
    allow_sel_group_ = value;
}

void DialogSelectInTree::set_window_text(const QString &value)
{
    setWindowTitle(value);
}

void DialogSelectInTree::on_treeView_doubleClicked(const QModelIndex &index)
{
    accept();
}


void DialogSelectInTree::on_buttonBox_accepted()
{
    accept();
}


void DialogSelectInTree::on_buttonBox_rejected()
{
    QDialog::close();
}

