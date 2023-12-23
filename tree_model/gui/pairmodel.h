#ifndef PAIRMODEL_H
#define PAIRMODEL_H

#include <QAbstractListModel>
typedef QPair<QVariant, QVariant> DataPair;

class PairModel : public QAbstractListModel
{
    Q_OBJECT    
    QList< DataPair > m_content;

public:


    explicit PairModel(QObject *parent = nullptr);

    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;

    void setContent(const QList<DataPair>& values){
        m_content = values;
    };

    QModelIndex findValue(const QVariant& value);

private:

};

#endif // PAIRMODEL_H
