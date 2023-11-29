#include "barcode_info.hpp"
#include <QDebug>
#include <QDateTime>

BarcodeInfo::BarcodeInfo(QObject *parent)
: QObject{parent}
{
    barcode_inf = arcirk::client::barcode_info();
    barcode_inf.currency = "руб.";
    barcode_inf.unit = "шт.";
    m_update = false;
}

void BarcodeInfo::set_barcode_info_object(const std::string &source)
{

    barcode_inf = arcirk::secure_serialization<arcirk::client::barcode_info>(source);

    if(!barcode_inf.image_base64.empty()){
         QByteArray img = QByteArray::fromBase64(barcode_inf.image_base64.data());
         if(!img.isNull()){
             QString image("data:image/png;base64,");
             image.append(QString::fromLatin1(img.toBase64().data()));
             m_imageSource = image;
         }else
             m_imageSource = "";
    }else
        m_imageSource = "";

    emit barcodeInfoChanged();

}

void BarcodeInfo::set_barcode_info(const arcirk::client::barcode_info &info)
{
    barcode_inf = info;
    if(!barcode_inf.image_base64.empty()){
         QByteArray img = QByteArray::fromBase64(barcode_inf.image_base64.data());
         if(!img.isNull()){
             QString image("data:image/png;base64,");
             image.append(QString::fromLatin1(img.toBase64().data()));
             m_imageSource = image;
         }else
             m_imageSource = "";
    }else
        m_imageSource = "";

    emit barcodeInfoChanged();
}

void BarcodeInfo::set_image_from_base64(const std::string &response)
{
    auto info = nlohmann::json::parse(response);
    if(info.value("uuid", "") != barcode_inf.uuid)
        return;

    barcode_inf.image_base64 = info.value("image_base64", "");
    if(!barcode_inf.image_base64.empty()){
         QByteArray img = QByteArray::fromBase64(barcode_inf.image_base64.data());
         if(!img.isNull()){
             QString image("data:image/png;base64,");
             image.append(QString::fromLatin1(img.toBase64().data()));
             m_imageSource = image;
             //emit imageSourceChanged();
         }else
             m_imageSource = "";
    }else
        m_imageSource = "";

    emit imageSourceChanged();
}

void BarcodeInfo::set_image(const QByteArray &ba)
{
//    m_image.loadFromData(ba);
//    emit imageChanged(m_image);
    if(!ba.isNull()){
        QString image("data:image/png;base64,");
        image.append(QString::fromLatin1(ba.toBase64().data()));
        m_imageSource = image;
        emit imageSourceChanged();
    }else
        m_imageSource = "";
}

QString BarcodeInfo::barcode() const
{
    return QString::fromStdString(barcode_inf.barcode);
}

QString BarcodeInfo::synonym() const
{
    return QString::fromStdString(barcode_inf.synonym);
}

QString BarcodeInfo::vendor_code() const
{
    return QString::fromStdString(barcode_inf.vendor_code);
}

int BarcodeInfo::balance() const
{
    return barcode_inf.balance;
}

double BarcodeInfo::price() const
{
    return barcode_inf.price;
}

QString BarcodeInfo::image_base64() const
{
    return QString::fromStdString(barcode_inf.image_base64);
}

bool BarcodeInfo::is_qr() const
{
    return barcode_inf.is_qr;
}

QString BarcodeInfo::imageSource()
{
    return m_imageSource;
}

void BarcodeInfo::set1CRow(const QString &jsonObject)
{
    clear();

    using json = nlohmann::json;

    auto obj = json::parse(jsonObject.toStdString());

    if(obj.is_object()){
        barcode_inf.barcode = obj.value("barcode", "");
        auto quantity = obj.value("quantity", json{});
        if(quantity.is_number_integer())
            barcode_inf.quantity = quantity.get<int>();
        else if(quantity.is_string()){
            try {
                barcode_inf.quantity = std::atoi(quantity.get<std::string>().c_str());
            } catch (...) {
            }
        }
        barcode_inf.line_uuid = obj.value("line_key", "");
    }

}

void BarcodeInfo::clear(const std::string& def)
{
    m_imageSource = "";
    barcode_inf = arcirk::client::barcode_info();
    barcode_inf.barcode = def;
    emit clearData();
}
