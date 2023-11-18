QT -= gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES +=

HEADERS += \
    arcirk.hpp

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

windows:DEFINES += _CRT_SECURE_NO_WARNINGS
windows:DEFINES += _WINDOWS

DEFINES += BADMIN_APPLICATION

Boost_USE_STATIC_LIBS = ON
windows:LIBS += -lbcrypt

INCLUDEPATH += $(BOOST_INCLDUE)

