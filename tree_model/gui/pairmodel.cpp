#include "pairmodel.h"

PairModel::PairModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_content = {};
}

int PairModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_content.count();
}

QModelIndex PairModel::findValue(const QVariant &value)
{
    QModelIndexList matches = match(
        index(0,0),
        Qt::UserRole,
        value,
        -1,
        Qt::MatchRecursive);
    foreach (const QModelIndex &match, matches){
        return match;
    }
    return QModelIndex();
}

QVariant PairModel::data(const QModelIndex &index, int role) const
{
    const DataPair& data = m_content.at( index.row() );
    QVariant value;

    switch ( role )
    {
    case Qt::DisplayRole:
    {
        value = data.first;
    }
    break;

    case Qt::UserRole:
    {
        value = data.second;
    }
    break;

    default:
        break;
    }

    return value;
}
