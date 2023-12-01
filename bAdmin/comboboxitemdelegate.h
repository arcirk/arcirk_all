#ifndef COMBOBOXITEMDELEGATE_H
#define COMBOBOXITEMDELEGATE_H

#include <QItemDelegate>
#include <QObject>

class ComboboxItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ComboboxItemDelegate(QObject* parent);

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
protected:
    virtual void drawFocus(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect) const override;
};

#endif // COMBOBOXITEMDELEGATE_H
