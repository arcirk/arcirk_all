#ifndef TASKPARAMDIALOG_H
#define TASKPARAMDIALOG_H

#include <QDialog>
//#include "tree_model/treeitemmodel.h"

#include "gui/treeviewwidget.h"
#include "iface/iface.hpp"

using namespace arcirk::tree_widget;
using namespace arcirk::tree_model;
using json = nlohmann::json;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::tree_model), param_structure,
    (std::string, name)
    (ByteArray, value)
)

typedef ITree<arcirk::tree_model::param_structure> ITreeParam;

namespace Ui {
class TaskParamDialog;
}

class TaskParamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskParamDialog(const json& param, QWidget *parent = nullptr);
    ~TaskParamDialog();

    void accept() override;

    json dialog_result() const;

private:
    Ui::TaskParamDialog *ui;
    TreeViewWidget* m_tree;

    void read_param(const json& param);

private slots:
    void onInsertParam();

};

#endif // TASKPARAMDIALOG_H
