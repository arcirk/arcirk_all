QT += gui
QT += widgets

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../../plugins/virtual/facelib/facelib.pri)
include(../../tree_model/tree_model.pri)
include(../../common/arcirk/arcirk.pri)

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    common.hpp \
    include/libcron/Cron.h \
    include/libcron/CronClock.h \
    include/libcron/CronData.h \
    include/libcron/CronRandomization.h \
    include/libcron/CronSchedule.h \
    include/libcron/DateTime.h \
    include/libcron/Task.h \
    include/libcron/TaskQueue.h \
    include/libcron/TimeTypes.h \
    propertydialog.h \
    taskitem.h \
    tasklistswidget.h

SOURCES += \
    propertydialog.cpp \
    src/CronClock.cpp \
    src/CronData.cpp \
    src/CronRandomization.cpp \
    src/CronSchedule.cpp \
    src/Task.cpp \
    taskitem.cpp \
    tasklistswidget.cpp

INCLUDEPATH += $$PWD/../libcron/externals/date/include
INCLUDEPATH += $$PWD/../libcron/include

RESOURCES += \
    cron_res.qrc

FORMS += \
    propertydialog.ui \
    taskitem.ui \
    tasklistswidget.ui

Boost_USE_STATIC_LIBS = ON

INCLUDEPATH += $(BOOST_INCLDUE)
INCLUDEPATH += $$PWD/../../plugins/virtual/facelib

win:DEFINES += _WINDOWS
