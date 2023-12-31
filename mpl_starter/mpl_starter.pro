QT -= gui

CONFIG += c++17
#console
CONFIG -= app_bundle
CONFIG -= console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = mpl_starter.ico

DISTFILES += \
    mpl_starter.ico

win32 {
    QMAKE_TARGET_PRODUCT = "Profile manager starter"
    QMAKE_TARGET_DESCRIPTION = "Starter manager profiles"
}

VERSION = 1.0.0
QMAKE_TARGET_COPYRIGHT = (c)(arcirk) Arcady Borisoglebsky
