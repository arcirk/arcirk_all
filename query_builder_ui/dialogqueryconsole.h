#ifndef DIALOGQUERYCONSOLE_H
#define DIALOGQUERYCONSOLE_H

#include <QDialog>
#include "websocketclient.h"
//#include "syntaxhighlighter.h"
#include "qsourcehighliter.h"

//using namespace arcirk::utils;
using namespace source_highlite;

namespace Ui {
class DialogQueryConsole;
}
namespace arcirk::query_builder_ui {
    class DialogQueryConsole : public QDialog
    {
        Q_OBJECT

    public:
        explicit DialogQueryConsole(WebSocketClient* client, QWidget *parent = nullptr);
        explicit DialogQueryConsole(QWidget *parent = nullptr);
        ~DialogQueryConsole();

    private:
        Ui::DialogQueryConsole *ui;

        QSourceHighliter *highlighter;
        WebSocketClient* m_client;

    signals:
        void openQueryBuilder(const QString& text);

    public slots:
        void onOpenQueryBuilder(const QString& text);
    };
}
#endif // DIALOGQUERYCONSOLE_H
