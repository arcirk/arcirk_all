#ifndef TEXTEDITORWIDGET_H
#define TEXTEDITORWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;
QT_END_NAMESPACE

namespace Ui {
class TextEditorWidget;
}

class TextEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TextEditorWidget(QWidget *parent = nullptr);
    ~TextEditorWidget();

    void setText(const QString& text);
    QString text();

private:
    Ui::TextEditorWidget *ui;
    QTextEdit *textEdit;

    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionUnderlineColor;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
    QAction *actionIndentLess;
    QAction *actionIndentMore;
    QAction *actionToggleCheckState;
    QAction *actionUndo;
    QAction *actionRedo;
#ifndef QT_NO_CLIPBOARD
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
#endif

    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;

    void setupTextActions();

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);

    void modifyIndentation(int amount);

private slots:
    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textStyle(int styleIndex);
    void textColor();
    void underlineColor();
    void textAlign(QAction *a);
    void setChecked(bool checked);
    void indent();
    void unindent();

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();

signals:

};

#endif // TEXTEDITORWIDGET_H
