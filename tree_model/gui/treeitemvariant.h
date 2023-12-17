#ifndef TREEITEMVARIANT_H
#define TREEITEMVARIANT_H

#ifndef IS_OS_ANDROID
#include "treeitemwidget.h"
#include <QWidget>
#include <QHBoxLayout>

namespace arcirk::tree::widgets {
    class TreeItemVariant : public TreeItemWidget
    {
        Q_OBJECT
    public:
        explicit TreeItemVariant(int row, int column, QWidget *parent = nullptr);
        ~TreeItemVariant(){qDebug() << __FUNCTION__;};

        bool isChecked() override;
        void setChecked(bool value) override;
        void setText(const QString& text) override;
        QString text() const override;
        QVariant currentState() const override{return m_raw.role;};
        void setCurrentState(const QVariant& state) override;
        void setRole(tree_editor_inner_role role) override;

        void setData(const QVariant& data);
        void setData(const json& data);
        QVariant data();
        void enableLabelFrame(bool value);
        void update();

    private:
        bool m_label_frame;
        QWidget* m_current_widget;
        QVariant m_current_value;
        QMenu* m_current_menu;
        QHBoxLayout * m_hbox;

        arcirk::synchronize::variant_p m_raw;
        void generateRaw(const std::string& rep, ByteArray* data = {});

        void createEditor();
        QWidget *createEditorNull();
        QWidget *createEditorLabel(bool save);
        QWidget *createEditorTextLine();
        QWidget *createEditorTextBox();
        QWidget *createEditorNumber();

        void setControlType();


        void clearLayout(QLayout* layout, bool deleteWidgets = true)
        {
            while (QLayoutItem* item = layout->takeAt(0))
            {
                QWidget* widget;
                if (  (deleteWidgets)
                    && (widget = item->widget())  ) {
                    delete widget;
                }
                if (QLayout* childLayout = item->layout()) {
                    clearLayout(childLayout, deleteWidgets);
                }
                delete item;
            }
        }

    private slots:
        void onMenuItemClicked();
        void onSelectClicked();
        void onSaveClicked();
        void onEraseClicked();
        void onSpinChanged(int value);
        void onTextChanged(const QString& value);
        void onTextEditChanged();

    };
}

#endif
#endif // TREEITEMVARIANT_H
