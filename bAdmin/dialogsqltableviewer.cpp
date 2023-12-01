#include "dialogsqltableviewer.h"
#include "ui_dialogsqltableviewer.h"
#include "tree_model.h"
#include "dialogtreemodelsettings.h"
#include <QStringListModel>

using namespace arcirk::tree_model;

DialogSqlTableViewer::DialogSqlTableViewer(database::tables table, WebSocketClient* client, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSqlTableViewer)
{
    ui->setupUi(this);

    m_table = table;
    m_client = client;

    m_start_id = 0;
    m_max_rows = 100;

    auto model = new TreeItemModel(this);
    model->set_hierarchical_list(false);

    ui->treeView->setModel(model);
    ui->treeView->setIndentation(5);

    const QIcon icon = QIcon::fromTheme("format-indent-less", QIcon(":/img/startTask.png"));
    QPixmap pixmap(icon.pixmap(16));
    QTransform  rm;
    rm.rotate(180);
    pixmap = pixmap.transformed(rm);
    ui->btnMovePrevious->setIcon(pixmap);
    const QIcon icon1 = QIcon::fromTheme("format-indent-less", QIcon(":/img/move_end.png"));
    QPixmap pixmap1(icon1.pixmap(16));
    pixmap1 = pixmap1.transformed(rm);
    ui->btnMoveFirst->setIcon(pixmap1);

    reload_model();

    row_count = model->rowCount();

    QStringList lst{"Таблица", "Иерархический сиписок"};
    ui->cmbState->setModel(new QStringListModel(lst, this));

    setWindowTitle(QString("Таблица: %1").arg(arcirk::enum_synonym(m_table).c_str()));
    auto buttons = ui->buttonBox->buttons();
    foreach (auto btn, buttons) {
        if(btn->text() == "Close")
            btn->setText("Закрыть");
    }


    update_navigation_widget();
}

DialogSqlTableViewer::~DialogSqlTableViewer()
{
    delete ui;
}

void DialogSqlTableViewer::reload_model()
{

//    auto model = (TreeItemModel*)ui->treeView->model();
//    model->set_limit_sql_rows(m_max_rows);
////    model->set_start_id(m_start_id);
//    model->set_hierarchical_list(false);
//    if(m_table == database::tables::tbMessages)
//        model->set_default_parent(0);
//    //model->fetchRoot(arcirk::enum_synonym(m_table).c_str());

}

void DialogSqlTableViewer::update_navigation_widget()
{
    int count = std::min(m_start_id + m_max_rows, row_count);
    ui->lblRecords->setText(QString("Записей: %1 - %2 из %3").arg(QString::number(m_start_id),QString::number(count),QString::number(row_count)));
    bool to_back = m_start_id > 0;
    bool to_next = count < row_count;
    ui->btnMovePrevious->setEnabled(to_back);
    ui->btnMoveFirst->setEnabled(to_back);
    ui->btnMoveNext->setEnabled(to_next);
    ui->btnMoveLast->setEnabled(to_next);

}

void DialogSqlTableViewer::on_btnTreeViewSettings_clicked()
{
//    auto model = (TreeItemModel*)ui->treeView->model();
//    auto dlg = DialogTreeModelSettings(model->conf(), this);
//    dlg.exec();
}


void DialogSqlTableViewer::on_btnMoveNext_clicked()
{
//    auto model = (TreeItemModel*)ui->treeView->model();
//    m_start_id += std::min(100, row_count - m_start_id);
//    model->clear();
//    model->set_limit_sql_rows(m_max_rows, m_start_id);
//    //model->fetchRoot(arcirk::enum_synonym(m_table).c_str());
//    update_navigation_widget();
}


void DialogSqlTableViewer::on_btnMoveLast_clicked(bool checked)
{
//    auto model = (TreeItemModel*)ui->treeView->model();
//    m_start_id = std::max(0, row_count - 100);
//    model->clear();
//    model->set_limit_sql_rows(m_max_rows, m_start_id);
//    //model->fetchRoot(arcirk::enum_synonym(m_table).c_str());
//    update_navigation_widget();
}


void DialogSqlTableViewer::on_btnMovePrevious_clicked()
{
//    auto model = (TreeItemModel*)ui->treeView->model();
//    m_start_id = std::max(0, m_start_id - 100);
//    model->clear();
//    model->set_limit_sql_rows(m_max_rows, m_start_id);
//    //model->fetchRoot(arcirk::enum_synonym(m_table).c_str());
//    update_navigation_widget();
}


void DialogSqlTableViewer::on_btnMoveFirst_clicked()
{
//    auto model = (TreeItemModel*)ui->treeView->model();
//    m_start_id = 0;
//    model->clear();
//    model->set_limit_sql_rows(m_max_rows, m_start_id);
//   // model->fetchRoot(arcirk::enum_synonym(m_table).c_str());
//    update_navigation_widget();
}

