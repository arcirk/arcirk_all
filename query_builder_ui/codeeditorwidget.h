#ifndef CODEEDITORWIDGET_H
#define CODEEDITORWIDGET_H

#include <QPlainTextEdit>

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

    protected:
        void resizeEvent(QResizeEvent *event) override;
        void dropEvent(QDropEvent *event) override;

    private slots:
        void updateLineNumberAreaWidth(int newBlockCount);
        void highlightCurrentLine();
        void updateLineNumberArea(const QRect &rect, int dy);

    private:
        QWidget *lineNumberArea;
        int currentLine;
        QPoint m_dragStart;
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
