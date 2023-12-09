#ifndef PROPERTYCONTROL_H
#define PROPERTYCONTROL_H

#include <QWidget>
#include <QToolButton>

enum ControlType{
    Label,
    TextLine,
    TextMultiLine,
    Spin,
    Checked,
    selectedFile,
    selectedDir
};

namespace Ui {
class PropertyControl;
}

class PropertyControl : public QWidget
{
    Q_OBJECT
public:

    explicit PropertyControl(ControlType t, QWidget *parent = nullptr);
    ~PropertyControl();


    void setData(const QVariant& value);
    QVariant data();

    void setCaption(const QString& value);

private:
    Ui::PropertyControl *ui;
    ControlType m_type;
    QVariant m_value;
    QWidget * m_currentWidget;
    QToolButton * m_selectButton;


    void createLabel();
    void createTextLine();
    void createTextBox();
    void createSpinBox();
    void createCheckBox();

};

#endif // PROPERTYCONTROL_H
