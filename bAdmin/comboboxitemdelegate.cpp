#include "comboboxitemdelegate.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>

ComboboxItemDelegate::ComboboxItemDelegate(QObject* parent)
    : QItemDelegate(parent)
{

}

QSize ComboboxItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
//    QStyle* style = qApp->style();
//    QStyleOptionViewItem opt = option;
//    QRect focusRect = style->subElementRect(QStyle::SE_ItemViewItemFocusRect, &opt);
//    focusRect.moveLeft(focusRect.left() + 1);
//    focusRect.setWidth(focusRect.width() - 10);
//    opt.rect = focusRect;
    return QItemDelegate::sizeHint(option, index);
}

void ComboboxItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
//    QStyle* style = qApp->style();
//    QStyleOptionViewItem opt = option;
//    //initStyleOption(&opt, index);
//    QRect focusRect = style->subElementRect(QStyle::SE_ItemViewItemFocusRect, &opt);
//    focusRect.moveLeft(focusRect.left() + 1);
//    focusRect.setWidth(focusRect.width() - 10);
//    opt.rect = focusRect;
    return QItemDelegate::paint(painter, option, index);
//    QString data = index.model()->data(index, Qt::DisplayRole).toString();


//    QStyleOptionViewItem myOption = option;
//    myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;


////    QString icon = ":/img/" + index.model()->data(index, Qt::DecorationRole).toString();

////    QPixmap pixmap2(icon);

//    drawDisplay(painter, myOption, myOption.rect, data);
//    drawFocus(painter, myOption, myOption.rect);

////    painter->drawPixmap(myOption.rect.x(),myOption.rect.y(),13,13, pixmap2);


}

void ComboboxItemDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const
{
//    QStyleOptionViewItem opt = option;

//    QStyle* style = QApplication::style();
//    painter->save();
//        painter->setPen(QColor(Qt::gray));
//        painter->drawRect(rect);
//    painter->restore();
}
