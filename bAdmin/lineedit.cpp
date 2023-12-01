#include "lineedit.h"
#include <QRegularExpressionValidator>

LineEdit::LineEdit(QWidget *parent) : QLineEdit(parent)
{
    connect(this, &QLineEdit::textEdited, this, &LineEdit::onValidateText);
}

void LineEdit::onValidateText(const QString &text)
{
    QRegularExpression rx("[A-Za-z\\d]+");
    QValidator * validator = new QRegularExpressionValidator(rx, this);
    //const QValidator *validator = sender_->validator();
    if (validator) {
        int pos;
        QString s = text;
        QString style;
        if (validator->validate(s, pos) != QValidator::Acceptable){
            style = "color: red";
        }else
            style = "";
        setStyleSheet(style);
        delete validator;
    }
}
