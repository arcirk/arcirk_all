#include "bankstatementsplugun.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "pluginpropertydialog.h"
#include <QStandardPaths>
#include <QDir>

using namespace arcirk::plugins;

BankStatementsPlugun::~BankStatementsPlugun()
{
//    m_param.insert("host", "");
//    m_param.insert("token", "");
//    m_param.insert("destantion", "");
//    m_param.insert("destantion_bnk", "");

}

void BankStatementsPlugun::setParam(const QString &param)
{
    //m_param = QJsonDocument::fromJson( param.toUtf8()).object();
    if(json::accept(param.toStdString()))
        m_param = json::parse(param.toStdString());
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

bool BankStatementsPlugun::editParam(QWidget* parent)
{
    auto dlg = PluginPropertyDialog(m_param, parent);
    if(dlg.exec()){
        return true;
    }
    return false;
}

json BankStatementsPlugun::default_param() const
{
    json table = json::object();
    table["columns"] = json::array({"key","value","is_group","ref","parent"});

    auto rows = json::array();
    auto row = plugin_param();
    row.key = "host";
    rows += pre::json::to_json(row);
    row = plugin_param();
    row.key = "token";
    rows += pre::json::to_json(row);
    row = plugin_param();
    row.key = "destantion";
    rows += pre::json::to_json(row);
    row = plugin_param();
    row.key = "destantion_bnk";
    rows += pre::json::to_json(row);

    table["rows"] = rows;

    return table;
}

json BankStatementsPlugun::read_param() const
{
    auto app_path = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!app_path.exists())
        return default_param();

    app_path += QDir::separator();


}
