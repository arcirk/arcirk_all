QT += core gui sql svg
QT += websockets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../query_builder_ui/query_builder_ui.pri)

SOURCES += \
    main.cpp


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

windows:DEFINES += _CRT_SECURE_NO_WARNINGS
windows:DEFINES += _WINDOWS

Boost_USE_STATIC_LIBS = ON
windows:LIBS += -lbcrypt

INCLUDEPATH += $(BOOST_INCLDUE)

CONFIG(debug, debug|release) {
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/debug/lib -lboost_locale-vc140-mt-gd -lfmtd -lcryptopp -lsqlite3 -lboost_filesystem-vc140-mt-gd -lboost_serialization-vc140-mt-gd
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/debug/bin
} else {
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/lib -lboost_locale-vc140-mt -lfmt -lcryptopp -lsqlite3 -lboost_filesystem-vc140-mt -lboost_serialization-vc140-mt
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/bin
}
