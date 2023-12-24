QT += core gui sql svg
QT += network
QT += widgets

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
include(../common/arcirk/arcirk.pri)

SOURCES += \
    gui/araydialog.cpp \
    gui/comparewidget.cpp \
    gui/pairmodel.cpp \
    gui/rowdialog.cpp \
    gui/selectgroupdialog.cpp \
    gui/selectitemdialog.cpp \
    gui/tabletoolbar.cpp \
    gui/texteditdialog.cpp \
    gui/treeitemcheckbox.cpp \
    gui/treeitemcombobox.cpp \
    gui/treeitemdelegate.cpp \
    gui/treeitemnumber.cpp \
    gui/treeitemtextedit.cpp \
    gui/treeitemtextline.cpp \
    gui/treeitemvariant.cpp \
    gui/treeitemwidget.cpp \
    gui/treeviewwidget.cpp \
    sort/treesortmodel.cpp \
    tree_model.cpp \
    treeitem.cpp

HEADERS += \
    global/arcirk_qt.hpp \
    gui/araydialog.h \
    gui/comparewidget.h \
    gui/pairmodel.h \
    gui/rowdialog.h \
    gui/selectgroupdialog.h \
    gui/selectitemdialog.h \
    gui/tabletoolbar.h \
    gui/texteditdialog.h \
    gui/treeitemcheckbox.h \
    gui/treeitemcombobox.h \
    gui/treeitemdelegate.h \
    gui/treeitemnumber.h \
    gui/treeitemtextedit.h \
    gui/treeitemtextline.h \
    gui/treeitemvariant.h \
    gui/treeitemwidget.h \
    gui/treeviewwidget.h \
    iface/iface.hpp \
    sort/treesortmodel.h \
    tree_model.h \
    treeitem.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

windows:DEFINES += _CRT_SECURE_NO_WARNINGS
windows:DEFINES += _WINDOWS

DEFINES +=  QT_STATICLIB_PRO

Boost_USE_STATIC_LIBS = ON
windows:LIBS += -lbcrypt

INCLUDEPATH += $(BOOST_INCLDUE)

win32 {
    QMAKE_TARGET_PRODUCT = "arcirk Tree Model"
    QMAKE_TARGET_DESCRIPTION = "arcirk Tree Model objects"
}

VERSION = 1.1.1
QMAKE_TARGET_COPYRIGHT = (c)(arcirk) Arcady Borisoglebsky

FORMS += \
    gui/araydialog.ui \
    gui/comparewidget.ui \
    gui/rowdialog.ui \
    gui/selectgroupdialog.ui \
    gui/selectitemdialog.ui \
    gui/tabletoolbar.ui \
    gui/texteditdialog.ui \
    gui/treeitemwidget.ui

RESOURCES += \
    res/resurce.qrc
