QT += core gui sql svg widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../common/arcirk/arcirk.pri)
include(../sql/query_builder/query_builder.pri)
include(../tree_model/tree_model.pri)
include(../highliter/qsourcehighlite.pri)
include(../ws/websocket/websocket.pri)

SOURCES += \
    $$PWD/databasestructurewidget.cpp \
    $$PWD/dialogquerybuilder.cpp \
    $$PWD/dialogqueryconsole.cpp \
    $$PWD/dialogtemptable.cpp \
    $$PWD/mainwindow.cpp \
    $$PWD/querybuilderadditionallywidget.cpp \
    $$PWD/querybuilderarbitraryexpressiondialog.cpp \
    $$PWD/querybuilderfieldexdialog.cpp \
    $$PWD/querybuilderfieldswidget.cpp \
    $$PWD/querybuilderforeginkeydialog.cpp \
    $$PWD/querybuildergenerator.cpp \
    $$PWD/querybuildergroupbyfieldslistwidget.cpp \
    $$PWD/querybuildergroupbyfunwidget.cpp \
    $$PWD/querybuilderinnersaliaseswidget.cpp \
    $$PWD/querybuilderpackagedialog.cpp \
    $$PWD/querybuilderselectdialog.cpp \
    $$PWD/querybuildersortwidget.cpp \
    $$PWD/querybuildertableinnerswidget.cpp \
    $$PWD/querybuildertableswidget.cpp \
    $$PWD/querybuildertotalbyhierarchy.cpp \
    $$PWD/querybuilderwherewidget.cpp \
    $$PWD/querybuildesqlitesupportitemdialog.cpp \
    $$PWD/queryeditoraddgroupdialog.cpp \
    $$PWD/codeeditorwidget.cpp \
    $$PWD/dialogsettablealias.cpp \
    $$PWD/queryinnercomparewidget.cpp

HEADERS += \
    $$PWD/databasestructurewidget.h \
    $$PWD/dialogquerybuilder.h \
    $$PWD/dialogqueryconsole.h \
    $$PWD/dialogtemptable.h \
    $$PWD/mainwindow.h \
    $$PWD/querybuilderadditionallywidget.h \
    $$PWD/querybuilderarbitraryexpressiondialog.h \
    $$PWD/querybuilderfieldexdialog.h \
    $$PWD/querybuilderfieldswidget.h \
    $$PWD/querybuilderforeginkeydialog.h \
    $$PWD/querybuildergenerator.h \
    $$PWD/querybuildergroupbyfieldslistwidget.h \
    $$PWD/querybuildergroupbyfunwidget.h \
    $$PWD/querybuilderinnersaliaseswidget.h \
    $$PWD/querybuilderpackagedialog.h \
    $$PWD/querybuilderselectdialog.h \
    $$PWD/querybuildersortwidget.h \
    $$PWD/querybuildertableinnerswidget.h \
    $$PWD/querybuildertableswidget.h \
    $$PWD/querybuildertotalbyhierarchy.h \
    $$PWD/querybuilderwherewidget.h \
    $$PWD/querybuildesqlitesupportitemdialog.h \
    $$PWD/queryeditoraddgroupdialog.h \
    $$PWD/codeeditorwidget.h \
    $$PWD/dialogsettablealias.h \
    $$PWD/sqlite_utils.hpp \
    $$PWD/queryinnercomparewidget.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

windows:DEFINES += _CRT_SECURE_NO_WARNINGS
windows:DEFINES += _WINDOWS

DEFINES += IS_USE_QT_LIB

Boost_USE_STATIC_LIBS = ON
windows:LIBS += -lbcrypt
windows:LIBS += -lsqlite3

INCLUDEPATH += $(BOOST_INCLDUE)

FORMS += \
    $$PWD/databasestructurewidget.ui \
    $$PWD/dialogquerybuilder.ui \
    $$PWD/dialogqueryconsole.ui \
    $$PWD/dialogtemptable.ui \
    $$PWD/mainwindow.ui \
    $$PWD/querybuilderadditionallywidget.ui \
    $$PWD/querybuilderarbitraryexpressiondialog.ui \
    $$PWD/querybuilderfieldexdialog.ui \
    $$PWD/querybuilderfieldswidget.ui \
    $$PWD/querybuilderforeginkeydialog.ui \
    $$PWD/querybuildergroupbyfieldslistwidget.ui \
    $$PWD/querybuildergroupbyfunwidget.ui \
    $$PWD/querybuilderinnersaliaseswidget.ui \
    $$PWD/querybuilderpackagedialog.ui \
    $$PWD/querybuilderselectdialog.ui \
    $$PWD/querybuildersortwidget.ui \
    $$PWD/querybuildertableinnerswidget.ui \
    $$PWD/querybuildertableswidget.ui \
    $$PWD/querybuildertotalbyhierarchy.ui \
    $$PWD/querybuilderwherewidget.ui \
    $$PWD/querybuildesqlitesupportitemdialog.ui \
    $$PWD/queryeditoraddgroupdialog.ui \
    $$PWD/dialogsettablealias.ui \
    $$PWD/queryinnercomparewidget.ui

RESOURCES += \
    $$PWD/resurses.qrc


INCLUDEPATH += $$PWD
