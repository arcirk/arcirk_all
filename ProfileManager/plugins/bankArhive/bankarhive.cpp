#include "bankarhive.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QDirIterator>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>

BankArhive::~BankArhive()
{

}

void BankArhive::setParam(const QString &param)
{
    auto doc = QJsonDocument::fromJson(param.toUtf8());
    if(!doc.isEmpty()){
        m_param = doc.object();
        m_task_param.destantion = m_param.value("destantion").toString();
        m_task_param.host = m_param.value("host").toString();
        m_task_param.token = m_param.value("token").toString();
    }

    is_valid = !m_task_param.destantion.isEmpty() && !m_task_param.host.isEmpty() && !m_task_param.token.isEmpty();
}

bool BankArhive::isValid()
{
    return is_valid;
}

bool BankArhive::accept()
{
    if(!is_valid)
        return false;

    auto dt = param_t(readData());
    if(dt.files.size() > 0){
        int i = 0;
        QMap<QString, QByteArray> m_result;
        foreach (auto itr, dt.files) {
            QString name = dt.names[i];
            auto m = file_details();
            m.parse(name);
            if(!m.pay.isEmpty()){
                QFile script(":/org_from_pay.bsl");
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

bool BankArhive::prepare()
{
    return false;
}

QByteArray BankArhive::param() const
{
    auto object = QJsonObject();
    object.insert("host", m_task_param.host);
    object.insert("token", m_task_param.token);
    object.insert("destantion", m_task_param.destantion);
    object.insert("destantion_bnk", m_task_param.destantion_bnk);
    return QJsonDocument(object).toJson();
}

QByteArray BankArhive::readData() const
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

QJsonObject BankArhive::http_get(const QString &command, const QString &param)
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

