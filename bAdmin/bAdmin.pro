QT += core gui sql svg
QT += websockets network
QT += core5compat

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20
#CONFIG += c++17

#QMAKE_CXXFLAGS_DEBUG += /MTd
#QMAKE_CXXFLAGS_RELEASE += /MT

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#include(../highliter/qsourcehighlite.pri)
include(../query_builder_ui/query_builder_ui.pri)
#include(../ws/websocket/websocket.pri)
#include(../tree_model/tree_model.pri)

SOURCES += \
    certuser.cpp \
    commandline.cpp \
    commandlineparser.cpp \
    cryptcertificate.cpp \
    cryptcontainer.cpp \
    dialogsetversion.cpp \
    dialogcertusercache.cpp \
    dialogedit.cpp \
    dialoginfo.cpp \
    dialogmessageslist.cpp \
    dialogmplitem.cpp \
    dialogmstsc.cpp \
    dialogselect.cpp \
    dialogselectdevice.cpp \
    dialogselectinlist.cpp \
    connectiondialog.cpp \
    dialogabout.cpp \
    dialogdevice.cpp \
    dialogimporttodatabase.cpp \
    dialogselectintree.cpp \
    dialogservers.cpp \
    dialogseversettings.cpp \
    dialogsqltableviewer.cpp \
    dialogtask.cpp \
    dialogtreemodelsettings.cpp \
    dialoguser.cpp \
    lineedit.cpp \
    main.cpp \
    mainwindow.cpp \
    messageitemdelegate.cpp \
    messageslistwidget.cpp \
    sendmessageboxwidget.cpp \
    taskparamdialog.cpp \
    texteditorwidget.cpp \
    userslistwidget.cpp \
    winreg/WinReg.cpp

HEADERS += \
    certuser.h \
    commandline.h \
    commandlineparser.h \
    cryptcertificate.h \
    cryptcontainer.h \
    dialogsetversion.h \
    dialogcertusercache.h \
    dialogedit.h \
    dialoginfo.h \
    dialogmessageslist.h \
    dialogmplitem.h \
    dialogmstsc.h \
    dialogselect.h \
    dialogselectdevice.h \
    dialogselectinlist.h \
    connectiondialog.h \
    dialogdevice.h \
    dialogabout.h \
    dialogimporttodatabase.h \
    dialogselectintree.h \
    dialogservers.h \
    dialogseversettings.h \
    dialogsqltableviewer.h \
    dialogtask.h \
    dialogtreemodelsettings.h \
    dialoguser.h \
    lineedit.h \
    mainwindow.h \
    messageitemdelegate.h \
    messageslistwidget.h \
    sendmessageboxwidget.h \
    shared_struct.hpp \
    sqlite_utils.hpp \
    taskparamdialog.h \
    texteditorwidget.h \
    userslistwidget.h \
    winreg/WinReg.hpp

FORMS += \
    dialogsetversion.ui \
    dialogcertusercache.ui \
    dialogedit.ui \
    dialoginfo.ui \
    dialogmessageslist.ui \
    dialogmplitem.ui \
    dialogmstsc.ui \
    dialogselect.ui \
    dialogselectdevice.ui \
    dialogselectinlist.ui \
    connectiondialog.ui \
    dialogdevice.ui \
    dialogabout.ui \
    dialogimporttodatabase.ui \
    dialogselectintree.ui \
    dialogservers.ui \
    dialogseversettings.ui \
    dialogsqltableviewer.ui \
    dialogtask.ui \
    dialogtreemodelsettings.ui \
    dialoguser.ui \
    mainwindow.ui \
    messageslistwidget.ui \
    sendmessageboxwidget.ui \
    taskparamdialog.ui \
    texteditorwidget.ui \
    userslistwidget.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

windows:DEFINES += _CRT_SECURE_NO_WARNINGS
windows:DEFINES += _WINDOWS

DEFINES += BADMIN_APPLICATION
DEFINES += USE_QUERY_BUILDER_LIB
DEFINES += USE_WEBSOCKET_LIB

Boost_USE_STATIC_LIBS = ON
windows:LIBS += -lbcrypt

INCLUDEPATH += $(BOOST_INCLDUE)
#LIBS += -L$(BOOST_LIB) -lboost_locale-vc140-mt

CONFIG(debug, debug|release) {
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/debug/lib -lboost_locale-vc140-mt-gd -lfmtd -lcryptopp -lsqlite3
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/debug/bin
} else {
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/lib -lboost_locale-vc140-mt -lfmt -lcryptopp -lsqlite3
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/bin
}


RESOURCES += \
    ba_resurses.qrc

DEFINES += IS_USE_QT_LIB
DEFINES += BOOST_LOCALE
#DEFINES += BOOST_FILESYSTEM

win32 {
    QMAKE_TARGET_PRODUCT = "arcirk Websocket Server Manager"
    QMAKE_TARGET_DESCRIPTION = "Managing servers"
}

VERSION = 1.1.1
QMAKE_TARGET_COPYRIGHT = (c)(arcirk) Arcady Borisoglebsky

