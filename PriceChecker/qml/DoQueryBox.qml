import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

Popup {

    id: dialog
    anchors.centerIn: parent
    width:  screenWidth > 1000 ? parent.width / 2 : parent.width - 20
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    property var rowObject: undefined
    property int fontPixelSize: screenWidth > 1000 ? 20 : 16

    property int version: 0
    property string uuid: ""
    property int row: 0
    property alias text: lblQuery.text
    property string theme: "Dark"
    property string command: "empty"
    property var param: ({})

    signal accept()
    signal cancel()

    GridLayout{
        id: grid
        columns: 2
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: 10
        columnSpacing: fontPixelSize
        Text{
            id: lblQuery
            font.pixelSize: fontPixelSize
            color: dialog.theme === "Light" ? "#424242" : "#efebe9"
            clip: true
            Layout.columnSpan: 2
            Layout.fillWidth: true
            //height: lblQuery.implicitHeight
            wrapMode: Text.WordWrap
        }

        RowLayout{
            Layout.alignment: Qt.AlignRight
            Layout.columnSpan: 2 // screenWidth > 1000 ? 1 : 2
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
                text: "Отмена"

                onClicked: {
                    dialog.visible = false
                    dialog.cancel()
                }
            }
        }
    }
}
