#include "querybuilderinnersaliaseswidget.h"
#include "ui_querybuilderinnersaliaseswidget.h"
#include <QRect>

using namespace arcirk::query_builder_ui;

QueryBuilderInnersAliasesWidget::QueryBuilderInnersAliasesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QueryBuilderInnersAliasesWidget)
{
    ui->setupUi(this);

    QRect rcParent = parent->geometry();
    QList<int> currentSizes = ui->splitter->sizes();
    currentSizes[0] = rcParent.width() / 100 * 30;
    currentSizes[1] = rcParent.width() / 100 * 70;
    ui->splitter->setSizes(currentSizes);
}

QueryBuilderInnersAliasesWidget::~QueryBuilderInnersAliasesWidget()
{
    delete ui;
}
