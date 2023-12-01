#include "messageitemdelegate.h"
#include <QPainter>
#include <QStyle>
#include <QApplication>
#include <QBrush>
#include <QDateTime>

using namespace arcirk::tree_model;

void MessageItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column() != 0) return;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
//    QStyle* style = QApplication::style();
//    QRect  rcDecoration = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt);
//    QRect  rcFocusRect = style->subElementRect(QStyle::SE_ItemViewItemFocusRect, &opt);
//    QRect  rcItemText = style->subElementRect(QStyle::SE_ItemViewItemText, &opt);

    auto model = (TreeItemModel*)index.model();
    auto item = arcirk::secure_serialization<arcirk::database::messages>(model->to_object(index));
    if(item.is_group == 1){

        auto date = QDateTime::fromSecsSinceEpoch(item.date);
        if(date.isValid()){
            painter->setPen(Qt::white);
            auto fnt = painter->font();
            fnt.setBold(true);
            painter->setFont(fnt);
            painter->drawText(opt.rect,  Qt::AlignVCenter | Qt::AlignLeft, date.toString("dd.MM.yyyy"));
            painter->drawLine(QLine(0, opt.rect.bottom(), opt.rect.width(), opt.rect.bottom()));
        }
    }else{
        paintHeader(painter, option, item, model);
    }

//        painter->setRenderHint(QPainter::Antialiasing);
//        painter->setBrush(QColor("#424242")); //95, 195, 255));
//        painter->setPen(Qt::NoPen);
//        painter->drawRoundedRect(opt.rect, 2.0, 2.0);
//    //rcDecoration.setWidth(rcDecoration.width() - 1);
//    //rcDecoration.setHeight(rcDecoration.height() - 1);
//    painter->setBrush(QBrush(Qt::gray));
//    //painter->fillRect(opt.rect, painter->brush());
//    painter->setPen(QColor(Qt::gray));
//    painter->drawRect(rcDecoration);
//    painter->drawRect(rcFocusRect);
//    painter->drawRect(rcItemText);
//    }
}

void MessageItemDelegate::paintHeader( QPainter * painter, const QStyleOptionViewItem & option, const arcirk::database::messages & object, TreeItemModel* model) const
{

   QPainter &p = *painter;
   p.save();
   p.setClipRect(option.rect);
   p.setPen(QColor(77, 77, 77));
   // рисуем текст
   QRect tr;
   QString name = model->property("name").toString();// object.first.c_str(),
   QString desc = "desc";//index.data(QvObjectModel::DetailRole).toString();

   QFont f = option.font;
   f.setPointSize(12);
   f.setWeight(QFont::Bold);
   QFontMetrics fm(f);
   tr = fm.boundingRect(name);
   p.setFont(f);
   p.drawText(option.rect, Qt::AlignVCenter | Qt::AlignLeft, name);

   f = option.font;
   f.setWeight(QFont::DemiBold);
   p.setFont(f);
   p.drawText(option.rect, Qt::AlignVCenter | Qt::AlignRight, desc);

   p.restore();
}

QSize MessageItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

void MessageItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}
