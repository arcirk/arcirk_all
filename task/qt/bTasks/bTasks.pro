QT += widgets

TEMPLATE = lib
DEFINES += BTASKS_LIBRARY

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../../../common/arcirk/arcirk.pri)
include(../../../tree_model/tree_model.pri)

SOURCES += \
    btasks.cpp

HEADERS += \
    bTasks_global.h \
    btasks.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../libcron/out/release/ -llibcron
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../libcron/out/debug/ -llibcron
else:unix: LIBS += -L$$PWD/../../libcron/out/ -llibcron

INCLUDEPATH += $$PWD/../../libcron/include/libcron
INCLUDEPATH += $$PWD/../../libcron/..
INCLUDEPATH += $$PWD/../../libcron/externals/date/include
DEPENDPATH += $$PWD/../../libcron/include/libcron

#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../libcron/out/release/liblibcron.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../libcron/out/debug/liblibcron.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../libcron/out/release/libcron.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../libcron/out/debug/libcron.lib
#else:unix: PRE_TARGETDEPS += $$PWD/../../libcron/out/liblibcron.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/./release/ -lbTasks
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/./debug/ -lbTasks
else:unix: LIBS += -L$$OUT_PWD/./ -lbTasks

INCLUDEPATH += $$PWD/../../libcron
DEPENDPATH += $$PWD/../../libcron
