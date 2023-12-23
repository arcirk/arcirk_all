QT += core gui sql svg
QT += websockets network
QT += core5compat

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20
#CONFIG += c++17
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../common/arcirk/arcirk.pri)
include(../sql/query_builder/query_builder.pri)
include(../ws/websocket/websocket.pri)
include(../tree_model/tree_model.pri)

SOURCES += \
    certuser.cpp \
    commandline.cpp \
    commandlineparser.cpp \
    cryptcertificate.cpp \
    cryptcontainer.cpp \
    dialoginfo.cpp \
    dialogmplitem.cpp \
    dialogmstsc.cpp \
    dialogremoteusers.cpp \
    dialogselectauth.cpp \
    dialogselectintree.cpp \
    dialogtask.cpp \
    main.cpp \
    dialogmain.cpp \
    taskparamdialog.cpp \
    winreg/WinReg.cpp

HEADERS += \
    certuser.h \
    commandline.h \
    commandlineparser.h \
    cryptcertificate.h \
    cryptcontainer.h \
    dialoginfo.h \
    dialogmain.h \
    dialogmplitem.h \
    dialogmstsc.h \
    dialogremoteusers.h \
    dialogselectauth.h \
    dialogselectintree.h \
    dialogtask.h \
    shared_struct.hpp \
    task.hpp \
    taskparamdialog.h \
    thread_pool.hpp \
    winreg/WinReg.hpp

FORMS += \
    dialoginfo.ui \
    dialogmain.ui \
    dialogmplitem.ui \
    dialogmstsc.ui \
    dialogremoteusers.ui \
    dialogselectauth.ui \
    dialogselectintree.ui \
    dialogtask.ui \
    taskparamdialog.ui


include(singleapplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

windows:DEFINES += _CRT_SECURE_NO_WARNINGS
windows:DEFINES += _WINDOWS
DEFINES += BOOST_LOCALE

Boost_USE_STATIC_LIBS = ON
#windows:LIBS += -lbcrypt

INCLUDEPATH += $(BOOST_INCLDUE)
INCLUDEPATH += $$PWD/../plugins/virtual/facelib

CONFIG(debug, debug|release) {
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/debug/lib -lboost_locale-vc140-mt-gd -lfmtd
#    -lcryptopp
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/debug/bin
} else {
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/lib -lboost_locale-vc140-mt -lfmt
#    -lcryptopp
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/bin
}

DEFINES += IS_USE_QT_LIB

win32 {
    QMAKE_TARGET_PRODUCT = "Manager Profile (arcirk)"
    QMAKE_TARGET_DESCRIPTION = "Client arcirk WebSocket Server"
}

VERSION = 1.1.0
QMAKE_TARGET_COPYRIGHT = (c)(arcirk) Arcady Borisoglebsky

RESOURCES += \
    pm_resurses.qrc
