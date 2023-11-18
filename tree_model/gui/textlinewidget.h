#ifndef TEXTLINEWIDGET_H
#define TEXTLINEWIDGET_H

#include <QWidget>

namespace Ui {
class TextLineWidget;
}

namespace arcirk::tree_model {
    class TextLineWidget : public QWidget
    {
        Q_OBJECT

        public:
            explicit TextLineWidget(int row, int column, QWidget *parent = nullptr);
            ~TextLineWidget();

            void setText(const QString& text);
            QString text() const;

            void setStyleSheet(const QString& text);

            int row() const{return m_row;};
            int column() const{return m_column;};

        private slots:
            void onLineEditEditingFinished();
            void onEditTextDialog();

        private:
            Ui::TextLineWidget *ui;
            int m_row;
            int m_column;

        signals:
            void toolButtonClicked(int row, int column, const QString& text);
    };
}
#endif // TEXTLINEWIDGET_H
