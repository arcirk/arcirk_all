#include "bankstatementsplugun.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "pluginpropertydialog.h"

using namespace arcirk::plugins;

BankStatementsPlugun::~BankStatementsPlugun()
{
    m_param.insert("host", "");
    m_param.insert("token", "");
    m_param.insert("destantion", "");
    m_param.insert("destantion_bnk", "");
}

void BankStatementsPlugun::setParam(const QString &param)
{
    m_param = QJsonDocument::fromJson( param.toUtf8()).object();
}

bool BankStatementsPlugun::isValid()
{
    return is_valid;
}

bool BankStatementsPlugun::accept()
{
    return true;
}

bool BankStatementsPlugun::prepare()
{
    return true;
}

QByteArray BankStatementsPlugun::param() const
{
    return QByteArray();
}

void BankStatementsPlugun::editParam(QWidget* parent)
{
    auto dlg = PluginPropertyDialog(m_param, parent);
    if(dlg.exec()){

    }
}
