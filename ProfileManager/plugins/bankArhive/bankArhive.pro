QT -= gui
QT += core
QT += network

TEMPLATE = lib
DEFINES += BANKARHIVE_LIBRARY
CONFIG += plugin

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bankarhive.cpp

HEADERS += \
    bankarhive.h \
    bankarhiveinterface.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    bsl.qrc

DISTFILES += \
    org_from_pay.bsl
