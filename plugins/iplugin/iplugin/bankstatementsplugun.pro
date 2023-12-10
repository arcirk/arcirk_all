QT += gui widgets
QT += core
QT += network

TEMPLATE = lib
DEFINES += BANKARHIVE_LIBRARY
CONFIG += plugin

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


include(../../virtual/facelib/facelib.pri)
include(../../../tree_model/tree_model.pri)
include(../../../common/arcirk/arcirk.pri)

SOURCES += \
    bankstatementsplugun.cpp \
    pluginpropertydialog.cpp

HEADERS += \
    bankstatementsplugun.h \
    pluginpropertydialog.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    plugin_res.qrc

FORMS += \
    pluginpropertydialog.ui

INCLUDEPATH += $$PWD/../../virtual/facelib

windows:DEFINES += _CRT_SECURE_NO_WARNINGS
windows:DEFINES += _WINDOWS
DEFINES += BOOST_LOCALE

Boost_USE_STATIC_LIBS = ON

INCLUDEPATH += $(BOOST_INCLDUE)
