#ifndef CODEEDITORWIDGET_H
#define CODEEDITORWIDGET_H

#include <QPlainTextEdit>
#include <QUuid>
#include <QSqlDatabase>
#include "global/arcirk_qt.hpp"

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE

namespace arcirk::query_builder_ui {

    class LineNumberArea;


    class CodeEditorWidget : public QPlainTextEdit
    {
        Q_OBJECT

    public:

        CodeEditorWidget(QWidget *parent = nullptr);

        void lineNumberAreaPaintEvent(QPaintEvent *event);
        int lineNumberAreaWidth();

        void setParent(const QUuid& ref){
            m_parent = ref;
            if(m_parent.isNull())
                m_ref = QUuid();
        }
        void setDatabaseRef(const QString& ref, const QSqlDatabase &db);

        void save(const QSqlDatabase& db);
        void read(const QSqlDatabase& db, const QString& dbRefDefault);

        QUuid ref() const{return m_ref;};
        QUuid parent_ref() const{return m_parent;}
        QString databaseRef(){return m_database_ref;}

    protected:
        void resizeEvent(QResizeEvent *event) override;
        void dropEvent(QDropEvent *event) override;
        void focusOutEvent(QFocusEvent *event) override;

    private slots:
        void updateLineNumberAreaWidth(int newBlockCount);
        void highlightCurrentLine();
        void updateLineNumberArea(const QRect &rect, int dy);

    private:
        QWidget *lineNumberArea;
        int currentLine;
        QPoint m_dragStart;
        QUuid m_parent;
        QUuid m_ref;
        QString m_database_ref;

    signals:
        void focusOut();
        void readData(const query_builder_querias& data);
    };

    class LineNumberArea : public QWidget
    {
    public:
        LineNumberArea(CodeEditorWidget *editor) : QWidget(editor), codeEditor(editor)
        {}

        QSize sizeHint() const override
        {
            return QSize(codeEditor->lineNumberAreaWidth(), 0);
        }

    protected:
        void paintEvent(QPaintEvent *event) override
        {
            codeEditor->lineNumberAreaPaintEvent(event);
        }

    private:
        CodeEditorWidget *codeEditor;
    };

}


#endif // CODEEDITORWIDGET_H
