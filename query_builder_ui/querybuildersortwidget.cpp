#include "querybuildersortwidget.h"
#include "ui_querybuildersortwidget.h"

using namespace arcirk::query_builder_ui;

QueryBuilderSortWidget::QueryBuilderSortWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QueryBuilderSortWidget)
{
    ui->setupUi(this);

    QRect rcParent = parent->geometry();
    QList<int> currentSizes = ui->splitter->sizes();
    currentSizes[0] = rcParent.width() / 100 * 40;
    currentSizes[1] = rcParent.width() / 100 * 60;
    ui->splitter->setSizes(currentSizes);
}

QueryBuilderSortWidget::~QueryBuilderSortWidget()
{
    delete ui;
}
