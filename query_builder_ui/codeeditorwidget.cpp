#include "codeeditorwidget.h"

#include <QPainter>
#include <QTextBlock>
#include <QMimeData>
#include <QApplication>
#include <QDrag>
#include <QPoint>
#include "query_builder.hpp"
#include "global/arcirk_qt.hpp"
#include <QSqlQuery>

using namespace arcirk::query_builder_ui;
using namespace arcirk::database::builder;

CodeEditorWidget::CodeEditorWidget(QWidget *parent) : QPlainTextEdit(parent)
{

    auto font = QFont("Source Code Pro", 10);
    this->setFont(font);
    currentLine = 0;

    setLineWrapMode(QPlainTextEdit::NoWrap);

    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditorWidget::blockCountChanged, this, &CodeEditorWidget::updateLineNumberAreaWidth);
    connect(this, &CodeEditorWidget::updateRequest, this, &CodeEditorWidget::updateLineNumberArea);
    connect(this, &CodeEditorWidget::cursorPositionChanged, this, &CodeEditorWidget::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CodeEditorWidget::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return std::max(space, 30);
}

void CodeEditorWidget::save(const QSqlDatabase &db)
{
    if(!db.isOpen())
        return;

    if(m_parent.isNull())
        return;

    auto query = query_builder();
    //auto item = query_builder_querias();

    QString q = query.use(json{"data"}).update("qbData", false).where(json{{"ref", tree::quuid_to_string(m_ref).toStdString()}}).prepare().c_str();
    //qDebug() << q;
    QSqlQuery rc(q, db);
    if(!this->toPlainText().isEmpty()){
        auto text = arcirk::string_to_byte_array(this->toPlainText().toUtf8().toBase64().toStdString());
        QByteArray* q_data = new QByteArray(reinterpret_cast<const char*>(text.data()), text.size());
        rc.bindValue(0, *q_data);
    }else{
        rc.bindValue(0, QByteArray());
    }

    if( !rc.exec() )
        qCritical() << "Ошибка сохранения blob!" << rc.lastError();

}

void CodeEditorWidget::read(const QSqlDatabase &db)
{
    if(!db.isOpen())
        return;

    if(m_parent.isNull())
        return;

    auto query = query_builder();
    QSqlQuery rc(query.select(json::array({"ref", "parent", "data"})).from("qbData").where(json{{"parent", tree::quuid_to_string(m_parent).toStdString()}}).prepare().c_str(), db);
    rc.exec();
    bool is_empty = true;
    QByteArray m_data;
    while (rc.next()) {
        m_ref = tree::to_qt_uuid(rc.value(0).toString().toStdString());
        m_parent = tree::to_qt_uuid(rc.value(1).toString().toStdString());
        m_data = rc.value(2).toByteArray();
        is_empty = false;
    }

    if(is_empty){
        query.clear();
        auto item = query_builder_querias();
        item.ref = boost::to_string(uuids::random_uuid());
        item.parent = tree::quuid_to_string(m_parent).toStdString();
        query.use(pre::json::to_json(item));
        rc.exec(query.insert("qbData").prepare().c_str());
    }

    if(!m_data.isEmpty()){
        this->setPlainText(QByteArray::fromBase64(m_data));
    }
}

void CodeEditorWidget::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditorWidget::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditorWidget::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditorWidget::dropEvent(QDropEvent *event)
{
    if(isReadOnly())
        return;

    QString txt;

    if (event->mimeData()->hasText()){
 //       auto pos = event->position();
        txt = event->mimeData()->text();// event->mimeData()->data("text/plain").toStdString().c_str();

        insertPlainText(txt);
        setFocus();

//        QTextCursor t =  this->textCursor();//cursorForPosition(event->position());
//        t.movePosition(QTextCursor::End);
//        this->setTextCursor(t);
    }

    event->acceptProposedAction();
}

void CodeEditorWidget::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    int c_line = 0;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
        //QTextBlock block = firstVisibleBlock();
        c_line = textCursor().block().blockNumber();
    }

    setExtraSelections(extraSelections);
    if(c_line !=currentLine){
        currentLine = c_line;
        lineNumberArea->repaint();
    }
}

void CodeEditorWidget::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::gray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            if(currentLine != blockNumber){
                painter.setPen(Qt::white);
            }else{
                //painter.save();
                auto fnt =this->font();
                fnt.setBold(true);
                painter.setFont(fnt);
                painter.setPen(Qt::yellow);
                //painter.restore();
            }

            painter.drawText(0, top, lineNumberArea->width() - 3, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
