#include "bankstatementsplugun.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "pluginpropertydialog.h"
#include <QStandardPaths>
#include <QDir>

using namespace arcirk::plugins;

BankStatementsPlugun::~BankStatementsPlugun()
{

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
    m_param = read_param();
    auto dlg = PluginPropertyDialog(m_param, parent);
    if(dlg.exec()){
        m_param = dlg.result();
        write_param();
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
    auto app_path = QPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));


    if(!app_path.exists()){
        auto res =app_path.mkpath();
        if(!res)
            return default_param();
    }

    app_path /= PLUGIN_FILE_NAME;

    if(!app_path.exists()){
        return default_param();
    }

    QFile f(app_path.path());
    if(f.open(QIODevice::ReadOnly)){
        auto str = f.readAll().toStdString();
        if(json::accept(str)){
            return json::parse(str);
        }else
            return default_param();
        f.close();
    }else
        return default_param();

}

void BankStatementsPlugun::write_param()
{
    auto app_path = QPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!app_path.exists()){
        auto res =app_path.mkpath();
        if(!res)
            return;
    }

    app_path /= PLUGIN_FILE_NAME;
    QFile f(app_path.path());
    if(f.open(QIODevice::WriteOnly)){
        f.write(m_param.dump().c_str());
        f.close();
    }

}

