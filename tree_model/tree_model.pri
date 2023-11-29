QT += core gui sql svg
QT += network
QT += widgets

CONFIG += c++20

SOURCES += \
    $$PWD/gui/comparewidget.cpp \
    $$PWD/gui/pairmodel.cpp \
    $$PWD/gui/rowdialog.cpp \
    $$PWD/gui/selectgroupdialog.cpp \
    $$PWD/gui/selectitemdialog.cpp \
    $$PWD/gui/tabletoolbar.cpp \
    $$PWD/gui/texteditdialog.cpp \
    $$PWD/gui/treeitemcheckbox.cpp \
    $$PWD/gui/treeitemcombobox.cpp \
    $$PWD/gui/treeitemdelegate.cpp \
    $$PWD/gui/treeitemnumber.cpp \
    $$PWD/gui/treeitemtextedit.cpp \
    $$PWD/gui/treeitemtextline.cpp \
    $$PWD/gui/treeitemvariant.cpp \
    $$PWD/gui/treeitemwidget.cpp \
    $$PWD/gui/treeviewwidget.cpp \
    $$PWD/sort/treesortmodel.cpp \
    $$PWD/tree_model.cpp \
    $$PWD/treeitem.cpp

HEADERS += \
    $$PWD/global/arcirk_qt.hpp \
    $$PWD/gui/comparewidget.h \
    $$PWD/gui/pairmodel.h \
    $$PWD/gui/rowdialog.h \
    $$PWD/gui/selectgroupdialog.h \
    $$PWD/gui/selectitemdialog.h \
    $$PWD/gui/tabletoolbar.h \
    $$PWD/gui/texteditdialog.h \
    $$PWD/gui/treeitemcheckbox.h \
    $$PWD/gui/treeitemcombobox.h \
    $$PWD/gui/treeitemdelegate.h \
    $$PWD/gui/treeitemnumber.h \
    $$PWD/gui/treeitemtextedit.h \
    $$PWD/gui/treeitemtextline.h \
    $$PWD/gui/treeitemvariant.h \
    $$PWD/gui/treeitemwidget.h \
    $$PWD/gui/treeviewwidget.h \
    $$PWD/iface/iface.hpp \
    $$PWD/sort/treesortmodel.h \
    $$PWD/tree_model.h \
    $$PWD/treeitem.h


windows:DEFINES += _CRT_SECURE_NO_WARNINGS
windows:DEFINES += _WINDOWS

Boost_USE_STATIC_LIBS = ON
windows:LIBS += -lbcrypt

INCLUDEPATH += $(BOOST_INCLDUE)

INCLUDEPATH += $$PWD

FORMS += \
    $$PWD/gui/comparewidget.ui \
    $$PWD/gui/rowdialog.ui \
    $$PWD/gui/selectgroupdialog.ui \
    $$PWD/gui/selectitemdialog.ui \
    $$PWD/gui/tabletoolbar.ui \
    $$PWD/gui/texteditdialog.ui \
    $$PWD/gui/treeitemwidget.ui

RESOURCES += \
    $$PWD/res/resurce.qrc
