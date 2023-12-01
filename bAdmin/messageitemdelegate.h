#ifndef MESSAGEITEMDELEGATE_H
#define MESSAGEITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>
#include <shared_struct.hpp>
#include "tree_model.h"

using namespace arcirk::tree_model;

class MessageItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    MessageItemDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

private:
    void paintHeader( QPainter * painter, const QStyleOptionViewItem & option, const arcirk::database::messages & object, TreeItemModel* model ) const;

private slots:

};

#endif // MESSAGEITEMDELEGATE_H
