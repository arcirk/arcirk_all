#ifndef TREESORTMODEL_H
#define TREESORTMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>
namespace arcirk::tree_model {

    enum type_of_comparison{
        Equals = 0,
        Not_Equals,
        More,
        More_Or_Equal,
        Less_Or_Equal,
        Less,
        On_List,
        Not_In_List,
        Contains
    };

class TreeSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
    //        Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    //        Q_PROPERTY(int sortRole READ sortRole WRITE setSortRole NOTIFY sortRoleChanged)
public:
    explicit TreeSortModel(QObject *parent = nullptr);

    void reset();

    void addFilter(const QString& column_name, const QVariant& value, type_of_comparison compare = type_of_comparison::Equals);
    Q_INVOKABLE void setFilter(const QString& filterText, int compare = 0);

    void setSourceModel(QAbstractItemModel *sourceModel) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const override;

    QString object_name() const {return "TreeSortModel";};

    Q_INVOKABLE QString dump(const int& row) const;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const  override;

signals:
    void modelChanged();
    //        void filterChanged();
    //        void sortRoleChanged();

private:

    QMap<QString, QPair<type_of_comparison, QVariant>> m_filter;
    bool isCompareValid(type_of_comparison compare, const QVariant& value, const QVariant& source) const;
};

}
#endif // TREESORTMODEL_H
