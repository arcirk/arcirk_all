import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

Popup {

    id: dialog
    anchors.centerIn: parent
    width:  screenWidth > 1000 ? parent.width / 2 : parent.width - 20
    //height: 200
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    property int fontPixelSize: screenWidth > 1000 ? 20 : 16

    property string uuid: ""
    property int row: 0
    property alias barcode: txtBarcode.text
    property alias quantity: txtQuantity.text
    property alias synonim: txtSynotim.text
    property alias unit: txtUnit.text
    property string theme: "Dark"

    signal accept()

    GridLayout{
        id: grid
        columns: 2

//        anchors.right: parent.right
//        anchors.left: parent.left
//        anchors.fill: dialog
//        Layout.fillWidth: true
        width: dialog.width - 30
        //anchors.margins: 10
        //margins: 10
        //Layout.margins: 10
        columnSpacing: fontPixelSize
        Text{
            font.pixelSize: fontPixelSize
            font.bold: true
            text: "Штрихкод:"
            color: dialog.theme === "Light" ? "#424242" : "#efebe9"
        }
        Text{
            id: txtBarcode
            font.pixelSize: fontPixelSize
            color: dialog.theme === "Light" ? "#424242" : "#efebe9"
            font.bold: true
            Layout.fillWidth: true
        }
        Text{
            font.pixelSize: fontPixelSize
            text: "Количество:"
            color: dialog.theme === "Light" ? "#424242" : "#efebe9"
            font.bold: true
        }
        RowLayout{
        TextField{
            id: txtQuantity
            font.pixelSize: fontPixelSize
            font.bold: true
            //text: wsSettings.url() //"ws://<domainname>"
            color: dialog.theme === "Light" ? "#424242" : "#efebe9"
            validator: IntValidator {bottom: 1; top: 10000}
            Material.accent: Material.Blue
            Layout.fillWidth: true
//            anchors.right: grid.right
//            anchors.left:
//            rightPadding: 10
            wrapMode: Text.WordWrap
            //enabled: !wsClient.isStarted();
            onEditingFinished: {
                //wsSettings.setUrl(txtServer.text);
            }
            onAccepted: {
                txtQuantity.focus = false
            }

        }
        Text{
            id: txtUnit
            font.pixelSize: fontPixelSize
            font.bold: true
            color: dialog.theme === "Light" ? "#424242" : "#efebe9"
        }
        }
        Text{
            id: txtSynotim
            Layout.columnSpan: 2
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            color: dialog.theme === "Light" ? "#424242" : "#d69545"
            width: grid.width - 20
            font.pixelSize: fontPixelSize
            Layout.fillWidth: true
        }

        RowLayout{
            Layout.alignment: Qt.AlignRight
            Layout.columnSpan: 2 // screenWidth > 1000 ? 1 : 2
            //Layout.fillWidth: true
            width: grid.width - 20
            Button{
                id: btnOK
                text: "OK" //screenWidth > 1000 ? "OK" : ""
                //icon.source:  screenWidth > 1000 ? "" : "qrc:/img/to_data.png"
                onClicked: {
                    dialog.visible = false
                    dialog.accept()
                }
            }

            Button{
                id: btnDislogClose
                text: "Закрыть"

                onClicked: {
                    dialog.visible = false
                }
            }
        }

    }

}
