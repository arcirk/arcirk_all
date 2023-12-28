QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    maindialog.cpp

HEADERS += \
    maindialog.h

FORMS += \
    maindialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../libcron/release/ -llibcron
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../libcron/debug/ -llibcron
else:unix: LIBS += -L$$OUT_PWD/../../libcron/ -llibcron

INCLUDEPATH += $$PWD/../../libcron
DEPENDPATH += $$PWD/../../libcron
INCLUDEPATH += $$PWD/../../libcron/include
INCLUDEPATH += $$PWD/../../libcron/externals/date/include
INCLUDEPATH += $$PWD/../../../tree_model
INCLUDEPATH += $$PWD/../../../common/arcirk

Boost_USE_STATIC_LIBS = ON

INCLUDEPATH += $(BOOST_INCLDUE)

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../libcron/release/liblibcron.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../libcron/debug/liblibcron.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../libcron/release/libcron.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../libcron/debug/libcron.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../libcron/liblibcron.a

win:DEFINES += _WINDOWS
