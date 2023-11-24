#ifndef ROWDIALOG_H
#define ROWDIALOG_H

#ifndef IS_OS_ANDROID
#include <QDialog>
#include "global/arcirk_qt.hpp"

using namespace arcirk::tree;

typedef QMap<std::string, QPair<QVariant, QVariant>> User_Data;

namespace Ui {
class RowDialog;
}
namespace arcirk::tree_widget {
    class RowDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit RowDialog(const json& data, const User_Data& user_data = {},
                           QWidget *parent = nullptr, const QMap<QString, QString>& aliases = {},
                           const QList<QString>& invisible = {}, const QList<QString>& order = {},
                           const QList<QString>& not_null = {},
                           const QString& parentSynonim = "");
        ~RowDialog();

        json data();
        void setIcon(const QIcon& ico){setWindowIcon(ico);};

        void accept() override;

    private:
        Ui::RowDialog *ui;
        json m_data;
        User_Data m_user_data;
        QMap<QString, QString> m_aliases;
        QList<QString> m_disable{"_id", "ref", "parent", "is_group"};
        QList<QString> m_not_null;
        QMap<QString, QList<QWidget*>> m_widgets;
        bool is_group;
        QString m_parentSynonim;
        QString m_parentRef;
        bool is_new_element;

        void createControls(const QList<QString>& invisible = {}, const QList<QString>& order = {});
        QList<QWidget*> createControl(const QString& key, const json& value);
        QList<QWidget*> createEditor(const QString& key, item_editor_widget_roles role, const json& value);
        QString fieldAlias(const QString& field);

        QList<QWidget*> createCheckBox(const QString& key, const json& value);
        QList<QWidget*> createSpinBox(const QString& key, const json& value);
        QList<QWidget*> createLineEdit(const QString& key, const json& value);
        QList<QWidget*> createTextEdit(const QString& key, const json& value);
        QList<QWidget*> createComboBox(const QString& key, const json& value);

        void addWidget(const QString& key, int row, QList<QWidget*> control, bool visible);

    private slots:
        void onControlDataChanged(const QVariant& value = false);
        void onValueChanged(const QVariant& value);
    signals:
        void onError(const QString& what, const QString& desc);

    };
}
#endif
#endif // ROWDIALOG_H
