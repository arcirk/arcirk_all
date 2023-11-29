#ifndef BARCODEPARSER_HPP
#define BARCODEPARSER_HPP

#include <QObject>
#include <QEventLoop>
#include <QVector>
#include <QKeySequence>

class BarcodeParser : public QObject
{
    Q_OBJECT

public:
    explicit
    BarcodeParser(QObject *parent = nullptr);

    Q_INVOKABLE void addSumbol(const int key, const QString& value);

private:
    QVector<QString> m_sumbols;

private slots:

signals:
    void barcode(const QString& value);

};

#endif
