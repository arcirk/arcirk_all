#include "qtandroidservice.h"

#ifdef IS_OS_ANDROID
#include <QJniEnvironment>
#include <QtDebug>
#include <QNetworkInterface>
#include <QFile>
#include <QUuid>

QtAndroidService *QtAndroidService::m_instance = nullptr;

static void receivedFromAndroidService(JNIEnv *env, jobject /*thiz*/, jstring value)
{
    emit QtAndroidService::instance()->messageFromService(
        env->GetStringUTFChars(value, nullptr));
}

QtAndroidService::QtAndroidService(QObject *parent) : QObject(parent)
{
    m_instance = this;

    registerNatives();
    registerBroadcastReceiver();
}

void QtAndroidService::sendToService(const QString &name)
{
    auto activity = QJniObject(QNativeInterface::QAndroidApplication::context());
    QAndroidIntent serviceIntent(activity.object(),
                                "ru/arcirk/lscanner/qtandroidservice/QtAndroidService");
    serviceIntent.putExtra("name", name.toUtf8());
    QJniObject result =activity.callObjectMethod(
                "startService",
                "(Landroid/content/Intent;)Landroid/content/ComponentName;",
                serviceIntent.handle().object());
}

void QtAndroidService::startUpdate(const QString &file_path)
{
    QJniEnvironment env;
    jclass javaClass = env.findClass("ru/arcirk/lscanner/qtandroidservice/ActivityUtils");
    QJniObject classObject(javaClass);
    QJniObject jsText = QJniObject::fromString(file_path);
    auto activity = QJniObject(QNativeInterface::QAndroidApplication::context());

    QJniObject intent = classObject.callObjectMethod("getIntent",
                                                     "(Ljava/lang/String;Landroid/content/Context;)Landroid/content/Intent;",
                                                     jsText.object<jstring>(),
                                                     activity.object());
    QtAndroidPrivate::startActivity(intent, 0);

}

QString QtAndroidService::getUuid()
{
#ifndef Q_OS_ANDROID
    return {};
#endif
    QJniEnvironment env;
    jclass javaClass = env.findClass("ru/arcirk/lscanner/qtandroidservice/ActivityUtils");
    QJniObject classObject(javaClass);
    //QJniObject jsText = QJniObject::fromString(file_path);
    auto activity = QJniObject(QNativeInterface::QAndroidApplication::context());

    QJniObject uuid = classObject.callObjectMethod("getUuid",
                                                     "(Landroid/content/Context;)Ljava/lang/String;",
                                                     activity.object());

    return uuid.toString();
}

void QtAndroidService::registerNatives()
{
    JNINativeMethod methods[] {
        {"sendToQt", "(Ljava/lang/String;)V", reinterpret_cast<void *>(receivedFromAndroidService)}};
    QJniObject javaClass("ru/arcirk/lscanner/qtandroidservice/ActivityUtils");

    QJniEnvironment env;
    jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
    env->RegisterNatives(objectClass,
                         methods,
                         sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);
}

void QtAndroidService::registerBroadcastReceiver()
{
    QJniEnvironment env;
    jclass javaClass = env.findClass("ru/arcirk/lscanner/qtandroidservice/ActivityUtils");
    QJniObject classObject(javaClass);

    auto activity = QJniObject(QNativeInterface::QAndroidApplication::context());
    classObject.callMethod<void>("registerServiceBroadcastReceiver",
                                 "(Landroid/content/Context;)V",
                                 activity.object());
}

QString QtAndroidService::androidId()
{
    // Trying to get ANDROID_ID from system
    QJniObject myID = QJniObject::fromString("android_id");
    QJniObject activity = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
    QJniObject appctx = activity.callObjectMethod("getApplicationContext","()Landroid/content/Context;");
    QJniObject contentR = appctx.callObjectMethod("getContentResolver", "()Landroid/content/ContentResolver;");

    QJniObject androidId = QJniObject::callStaticObjectMethod("android/provider/Settings$Secure","getString",
                                                                         "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;",
                                                                         contentR.object<jobject>(),
                                                                         myID.object<jstring>());

    if(androidId.isValid() && !androidId.toString().isEmpty()){
        if(androidId.toString().endsWith("9774d56d682e549c")){
            qWarning()<< Q_FUNC_INFO <<"This device has the bug with unique id"; //https://code.google.com/p/android/issues/detail?id=10603
        }else{
            qDebug()<< Q_FUNC_INFO <<"Using androidId"<<androidId.toString();
            QUuid id = QUuid::fromRfc4122(androidId.toString().toUtf8());
            return id.toString(QUuid::WithoutBraces);
        }
    }


    qDebug()<< Q_FUNC_INFO <<"Using randomUuid";
    return QUuid::createUuid().toString();
}
#else
QtAndroidService::QtAndroidService(QObject *parent) : QObject(parent)
{

}

#endif
