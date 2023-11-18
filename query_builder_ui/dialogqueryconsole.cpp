#include "dialogqueryconsole.h"
#include "ui_dialogqueryconsole.h"
#include <QMenu>
#include <QTextEdit>
//#include "dialogquerybuilder.h"
#include "QueryBuilderPackageDialog.h"

using namespace arcirk::query_builder_ui;

DialogQueryConsole::DialogQueryConsole(WebSocketClient* client, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogQueryConsole)
{
    ui->setupUi(this);

    m_client = client;

    connect(ui->txtCodeEdit, &QTextEdit::customContextMenuRequested, [&](const QPoint& pos) {
        QMenu menu;
        menu.addAction(tr("Конструктор запросов"), ui->txtCodeEdit, [&]() {
            emit openQueryBuilder(ui->txtCodeEdit->toPlainText());
        });

        menu.exec(ui->txtCodeEdit->viewport()->mapToGlobal(pos));
    });
    connect(this, &DialogQueryConsole::openQueryBuilder, this, &DialogQueryConsole::onOpenQueryBuilder);

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    ui->txtCodeEdit->setFont(font);

    //highlighter = new SyntaxHighlighter(ui->txtCodeEdit->document());
    highlighter = new QSourceHighliter(ui->txtCodeEdit->document());
    highlighter->setCurrentLanguage(QSourceHighliter::CodeSQL);

    auto splitter = ui->splitterMain;
    QRect rcParent = parent->geometry();
    QList<int> currentSizes = splitter->sizes();
    currentSizes[0] = rcParent.width() / 100 * 30;
    currentSizes[1] = rcParent.width() / 100 * 70;
    splitter->setSizes(currentSizes);
    splitter = ui->splitter;
    rcParent = parent->geometry();
    currentSizes = splitter->sizes();
    currentSizes[0] = rcParent.width() / 100 * 70;
    currentSizes[1] = rcParent.width() / 100 * 30;
    splitter->setSizes(currentSizes);
}

DialogQueryConsole::DialogQueryConsole(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogQueryConsole)
{
    ui->setupUi(this);

    m_client = nullptr;

    connect(ui->txtCodeEdit, &QTextEdit::customContextMenuRequested, [&](const QPoint& pos) {
        QMenu menu;
        menu.addAction(tr("Конструктор запросов"), ui->txtCodeEdit, [&]() {
            emit openQueryBuilder(ui->txtCodeEdit->toPlainText());
        });

        menu.exec(ui->txtCodeEdit->viewport()->mapToGlobal(pos));
    });
    connect(this, &DialogQueryConsole::openQueryBuilder, this, &DialogQueryConsole::onOpenQueryBuilder);

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    ui->txtCodeEdit->setFont(font);

    //highlighter = new SyntaxHighlighter(ui->txtCodeEdit->document());
    highlighter = new QSourceHighliter(ui->txtCodeEdit->document());
    highlighter->setCurrentLanguage(QSourceHighliter::CodeSQL);

    auto splitter = ui->splitterMain;
    QRect rcParent = this->geometry();
    QList<int> currentSizes = splitter->sizes();
    currentSizes[0] = rcParent.width() / 100 * 30;
    currentSizes[1] = rcParent.width() / 100 * 70;
    splitter->setSizes(currentSizes);
    splitter = ui->splitter;
    rcParent = this->geometry();
    currentSizes = splitter->sizes();
    currentSizes[0] = rcParent.width() / 100 * 70;
    currentSizes[1] = rcParent.width() / 100 * 30;
    splitter->setSizes(currentSizes);
}


DialogQueryConsole::~DialogQueryConsole()
{
    delete ui;
}

void DialogQueryConsole::onOpenQueryBuilder(const QString& text)
{
    auto dlg = QueryBuilderPackageDialog(m_client, this);
    if(dlg.exec()){
        ui->txtCodeEdit->setPlainText(dlg.result());
    }
}
