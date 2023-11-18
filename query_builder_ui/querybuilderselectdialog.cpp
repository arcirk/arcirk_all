#include "querybuilderselectdialog.h"
#include "ui_querybuilderselectdialog.h"
#include <QMessageBox>
#include <QTreeView>

using namespace arcirk::query_builder_ui;

QueryBuilderSelectDialog::QueryBuilderSelectDialog(TreeItemModel* model, bool allowSelGroup , QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueryBuilderSelectDialog)
{
    ui->setupUi(this);

    ui->treeView->setModel(model);
    sel_group = allowSelGroup;

    for (int i = 1; i < model->columnCount(); ++i) {
        ui->treeView->hideColumn(i);
    }
    connect(ui->treeView, &QTreeView::doubleClicked, this, &QueryBuilderSelectDialog::onTreeViewSelect);
}

QueryBuilderSelectDialog::~QueryBuilderSelectDialog()
{
    delete ui;
}

void QueryBuilderSelectDialog::accept()
{
    auto index = ui->treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран эелемент!");
        return;
    }

    auto model = (TreeItemModel*)ui->treeView->model();
    object = model->to_object(index);

    return QDialog::accept();

}

json QueryBuilderSelectDialog::selectedObject() const
{
    return object;
}

void QueryBuilderSelectDialog::onTreeViewSelect(const QModelIndex &index)
{
    Q_UNUSED(index);
    accept();
}
