import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

Popup {
    id: enterBarcodeDialog
    anchors.centerIn: parent
    width:  screenWidth > 1000 ? parent.width / 2 : parent.width - 20
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    property alias br: txtBarcode.text

    signal enterBarcodeFromKeyboard(string value);

    GridLayout{
        id:grid
        columns: 2
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: 10
        columnSpacing: 10
        Text {
            id: txt
            font.pixelSize: 20
            text: qsTr("Введите штрихкод:")
            color: "white"
        }
        Label{
            text: ""
        }
        TextField{
            font.pixelSize: 20
            id: txtBarcode
            Layout.fillWidth: true
            Material.accent: Material.Blue
            Layout.columnSpan: 2
            onAccepted: {
                txtBarcode.focus = false
            }
        }
//        Label{
//            text: ""
//        }
//        Label{
//            text: ""
//        }
        RowLayout{
            Layout.alignment: Qt.AlignRight
            Layout.columnSpan: 2
            Button {
                text: "OK"
                onClicked: {
                    enterBarcodeDialog.enterBarcodeFromKeyboard(txtBarcode.text);
                    enterBarcodeDialog.visible = false;
                }
            }
            Button {
                text: "Отмена"
                onClicked: {
                    enterBarcodeDialog.visible = false;
                }
            }
        }


    }


}
