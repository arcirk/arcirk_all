import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

Popup {
    id: documentInfoDialog
    anchors.centerIn: parent
    //width: screenWidth > 1000 ? screenWidth / 2 : (screenHeight < 640 ? screenWidth - (screenWidth*0.35) : screenWidth - 20)
    width:  screenWidth > 1000 ? parent.width / 2 : parent.width - 20
    //height: 200
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    property var rowObject: undefined

    property alias docNumber: txtNumber.text
    property alias docDate: txtDate.text
    property alias docComent: txtComment.text
    property string uuid: ""
    property string docSourceComment: ""
    property bool isNew: false


    signal accept(var index);


    GridLayout {
        columns: 2
        rows: 4
        anchors.fill: parent
        Text {
            id: lblNumber
            font.pixelSize: 20
            text: qsTr("Номер:")
            color: "white"
        }
        RowLayout{
            TextField{
                font.pixelSize: 20
                id: txtNumber
                Layout.fillWidth: true
                Material.accent: Material.Blue
                //Layout.columnSpan: 2
                onAccepted: {
                    txtNumber.focus = false
                }
            }
            ToolButton {
                icon.source: "qrc:/img/arrow_back.svg"
                id: btnGenerateNumber
                onClicked: {
                    if(documentInfoDialog.rowObject !== undefined)
                        txtNumber.text = wsClient.documentGenerateNewNumber(Number(rowObject._id))
                    else{
                        txtNumber.text = wsClient.documentGenerateNewNumber(1)
                    }
                }
            }
        }


        Text {
            id: lblDate
            font.pixelSize: 20
            text: qsTr("Дата:")
            color: "white"
        }
        Label{
            text: ""
        }

        RowLayout{
            Layout.columnSpan: 2
            Text{
                font.pixelSize: 20
                id: txtDate
                Layout.fillWidth: true
                Material.accent: Material.Blue
                text: Qt.formatDateTime(new Date(), "dd.MM.yyyy hh:mm:ss")
                color: "white"
//                onAccepted: {
//                    txtDate.focus = false
//                }
            }
            ToolButton {
                icon.source: "qrc:/img/calendar.svg"
                id: btnDatePicker
                onClicked: {
                    var locale = Qt.locale()
                    calendar.selectedDate = Date.fromLocaleString(locale, txtDate.text, "dd.MM.yyyy hh:mm:ss")
                    calendar.visible = true
                }
            }

        }

        Text {
            id: lblComment
            font.pixelSize: 20
            text: qsTr("Комментарий:")
            color: "white"
        }
        Label{
            text: ""
        }

        TextField{
            font.pixelSize: 20
            id: txtComment
            Layout.fillWidth: true
            Material.accent: Material.Blue
            Layout.columnSpan: 2
            wrapMode: Text.WordWrap
            color: "white"
            onAccepted: {
                txtComment.focus = false
            }
        }

        RowLayout{
            Layout.alignment: Qt.AlignRight
            Layout.columnSpan: 2
            Button {
                text: "OK"
                onClicked: {
                    documentInfoDialog.accept(documentInfoDialog.rowObject);
                    documentInfoDialog.visible = false;
                }
            }
            Button {
                text: "Отмена"
                onClicked: {
                    documentInfoDialog.visible = false;
                }
            }
        }
    }
    Popup {
        id: calendar
        visible: false
        anchors.centerIn: parent
        property alias selectedDate: datePicker.selectedDate
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        DatePicker{
            id: datePicker
            onSelectDate: {
                calendar.visible = false;
                txtDate.text = Qt.formatDateTime(datePicker.selectedDate, "dd.MM.yyyy hh:mm:ss")
            }
        }
    }
}
