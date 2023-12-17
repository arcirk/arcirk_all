#include "bankstatementsplugun.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "pluginpropertydialog.h"
#include <QStandardPaths>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace arcirk::plugins;

BankStatementsPlugun::~BankStatementsPlugun()
{

}

void BankStatementsPlugun::setParam(const QString &param)
{
    if(json::accept(param.toStdString()))
        m_param = json::parse(param.toStdString());
}

bool BankStatementsPlugun::isValid()
{
    return is_valid;
}

bool BankStatementsPlugun::accept()
{
    m_param = read_param();
    m_task_param = task_param();
    auto vals = m_param.value("rows", json::array());
    for (auto itr = vals.begin(); itr != vals.end(); ++itr) {
        auto object = *itr;
        m_task_param.host = object.value("host", "").c_str();
        m_task_param.token = object.value("token", "").c_str();
        m_task_param.destantion = object.value("destantion", "").c_str();
        m_task_param.destantion_bnk = object.value("destantion_bnk", "").c_str();
    }

    QDir dest(m_task_param.destantion);
    if(!dest.exists()){
        m_last_error = "Каталог загруженных файлов не найден!";
        return false;
    }

    QDir dest_bnk(m_task_param.destantion_bnk);
    if(!dest.exists()){
        m_last_error = "Каталог на хосте не найден!";
        return false;
    }

    auto dt = param_t(readData());
    if(dt.files.size() > 0){
        int i = 0;
        QMap<QString, QByteArray> m_result;
        foreach (auto itr, dt.files) {
            QString name = dt.names[i];
            auto m = file_details();
            m.parse(name);
            if(!m.pay.isEmpty()){
                QFile script(":res/org_from_pay.bsl");
                if(script.open(QIODevice::ReadOnly)) {
                    //получим расшифровку номера счета
                    QJsonObject param;
                    param.insert("pay", m.pay);
                    param.insert("privileged_mode", true);
                    param.insert("script", QString(script.readAll().toBase64()));
                    auto result = http_get("Run1CScript", QJsonDocument(param).toJson());

                    if(!result.isEmpty()){
                        QString orgName = result.value("name").toString();
                        QString path = orgName + "/";
                        if(dt.creates.size() > i){
                            auto m_date = dt.creates[i];
                            if(m_date.isValid()){
                                path.append(m_date.toString("yyyy/MM/dd/"));
                            }
                            path.append(dt.generateName(i));
                            path.append(".");
                            path.append(dt.suffix[i]);
                            auto fileName = m_task_param.destantion + path;
                            QByteArray data(itr);
                            m_result.insert(fileName, data);
                        }
                    }
                }
            }
            i++;
        }
        if(m_result.size() > 0){
            for (auto itr = m_result.begin(); itr != m_result.end(); ++itr) {
                QFile f(itr.key());
                if(f.open(QIODevice::WriteOnly)){
                    f.write(itr.value());
                    f.close();
                }
            }
        }
        foreach (auto itr, dt.loc_paths) {
            QFile f(itr);
            f.remove();
        }
    }

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

QString BankStatementsPlugun::lastError() const
{
    return m_last_error;
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
QByteArray BankStatementsPlugun::readData() const
{
    //ищем скаченные файлы выписок
    QString pathSource = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    auto d = QDir(pathSource);
    if(!d.exists())
        return nullptr;

    auto lst = QStringList();
    lst << "*Выписка за*";
    lst << "*kl_to_1c*";

    QDirIterator it(pathSource, lst, QDir::NoFilter, QDirIterator::Subdirectories);
    auto p = param_t();
    while (it.hasNext()) {
        p.read(it.next());
    }

    return p.toRaw();
}

QJsonObject BankStatementsPlugun::http_get(const QString &command, const QString &param)
{
    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;

    auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
    {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if(status_code.isValid()){
            httpStatus = status_code.toInt();
            if(httpStatus != 200){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
            }else
            {
                httpData = reply->readAll();
            }
        }
        loop.quit();

    };

    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

    QUrl ws(m_task_param.host);
    QString http_query = "/api/info";
    QUrl url(ws.toString() + http_query);
    QNetworkRequest request(url);

    QString headerData = "Token " + m_task_param.token.toUtf8().toBase64();
    request.setRawHeader("Authorization", headerData.toLocal8Bit());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    auto http_param = QJsonObject();
    http_param.insert("command" , command);
    http_param.insert("param" , QByteArray(param.toUtf8()).toBase64().data());

    httpService.post(request, QJsonDocument(http_param).toJson());

    loop.exec();

    if(httpStatus != 200){
        return QJsonObject();
    }

    if(httpData.isEmpty())
        return QJsonObject();

    if(httpData == WS_RESULT_ERROR){
        return QJsonObject();
    }

    auto msg = QJsonDocument::fromJson(httpData).object();

    if(msg.isEmpty())
        return QJsonObject();

    try {
        if(msg.value("result").toString() != WS_RESULT_ERROR){
            if(msg.value("result").toString() != WS_RESULT_SUCCESS){
                QByteArray text = QByteArray::fromBase64(msg.value("result").toString().toUtf8());
                auto http_result = QJsonDocument::fromJson(text).object();
                return http_result;
            }else
                return QJsonObject();
        }else{
            //emit error(__FUNCTION__, msg.command.c_str(), msg.message.c_str());
            return QJsonObject();
        }
    } catch (const std::exception& e) {
        //emit error(__FUNCTION__, msg.command.c_str(), e.what());
        return QJsonObject();
    }

}

