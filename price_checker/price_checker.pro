QT += quick
QT += websockets network sql
QT += svg
QT += quickcontrols2
QT += core-private
QT += core

CONFIG += c++20
#CONFIG += c++17

#QMAKE_CXXFLAGS_DEBUG += /MTd
#QMAKE_CXXFLAGS_RELEASE += /MT

include(../common/arcirk/arcirk.pri)
include(../tree_model/tree_model.pri)
include(../ws/websocket/websocket.pri)
include(../sql/query_builder/query_builder.pri)

SOURCES += \
        main.cpp \
        src/barcode_info.cpp \
        src/barcode_parser.cpp \
        src/iwebsocketclient.cpp \
        src/notificationqueue.cpp \
        src/qtandroidservice.cpp \
        src/synch_data.cpp \
        src/wssettings.cpp

RESOURCES += main.qrc
# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml\
    android/res/xml/provider_paths.xml \
    android/src/ru/arcirk/lscanner/qtandroidservice/ActivityUtils.java \
    android/src/ru/arcirk/lscanner/qtandroidservice/DeviceUuidFactory.java \
    android/src/ru/arcirk/lscanner/qtandroidservice/QtAndroidService.java

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
#    ANDROID_PACKAGE_SOURCE_DIR = \
#        $$PWD/android
 DEFINES += IS_OS_ANDROID

}
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
windows:DEFINES += _CRT_SECURE_NO_WARNINGS
windows:DEFINES += _WINDOWS

Boost_USE_STATIC_LIBS = ON
windows:LIBS += -lbcrypt

INCLUDEPATH += $(BOOST_INCLDUE)

DEFINES += IS_USE_QT_LIB
DEFINES += QML_APPLICATION

#CONFIG(debug, debug|release) {
#    LIBS += -LC:/lib/vcpkg/installed/x64-windows-static/debug/lib -lboost_locale-vc140-mt-gd
##    LIBS += -LC:/lib/vcpkg/installed/x64-windows/debug/bin
##    QMAKE_CXXFLAGS += /MTd
#} else {
#    LIBS += -LC:/lib/vcpkg/installed/x64-windows-static/lib -lboost_locale-vc140-mt
##    LIBS += -LC:/lib/vcpkg/installed/x64-windows/bin
##    QMAKE_CXXFLAGS += /MT
#}

HEADERS += \
    src/barcode_info.hpp \
    src/barcode_parser.hpp \
    src/database_struct.hpp \
    src/iwebsocketclient.h \
    src/notificationqueue.h \
    src/qtandroidservice.h \
    src/shared_struct.hpp \
    src/synch_data.hpp \
    src/verify_database.hpp \
    src/wssettings.h


