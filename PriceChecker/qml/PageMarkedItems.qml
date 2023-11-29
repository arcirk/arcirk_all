import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

import QmlTableModel 1.2
import QmlSortModel 1.0

import "qrc:/js/PageMarkedItemsJs.js" as PageMarkedItemsJs

Page {

    id: control

    property string docRef: ""
    property string itemRef: ""
    property string parentText: "Подбор кодов маркировки"
    property string currentBarcode: ""
    property int parentQuantity: 0
    property int currentQuantity: 0
    property int fontPixelSizeGrey: screenWidth > 1000 ? 20 : 8
    property int imageMaximumHeight: screenWidth > 1000 ? 400 : 250
    property int fontPixelSize: screenWidth > 1000 ? 20 : 16
    property bool findToolBar: false

    property var rowObject: undefined
    property string session_uuid: ""
    property string form_uuid: ""
    property bool is_document_1c: false

    signal quantityChanged(int quantity, int parentQuantity);
    signal viewBarcodeInfo(string br);
    signal errorMessahge(string message)

    function setModel(text){
        PageMarkedItemsJs.setDcumentContentModel(text)
    }
    function setFilterBarcode(value){
        PageMarkedItemsJs.setFilterBarcode(value)
    }
    function setBarcode(qrCode){
        if(qrCode.length <= 13){
            control.errorMessahge("Не верный формат!")
            return;
        }
        PageMarkedItemsJs.setBarcode(qrCode)
    }
    property QmlTableModel wsDocumentTableMarked: QmlTableModel{
        currentObjectName: "DocumentsMarkedTables"
        onCurrentRowChanged: {

        }
        onRowCountChanged: function(count){
            if(count !== control.parentQuantity)
                txtQuantity.text = "Подобрано: <font color=\"#ff8080\">" + count + "</font>/" + parentQuantity + " шт."
            else
                txtQuantity.text = "Подобрано: <font color=\"#66a334\">" + count + "</font>/" + parentQuantity + " шт."
            control.quantityChanged(count, control.parentQuantity);
        }
    }
    property QmlSortModel wsProxyMarkedModel: QmlSortModel{
        sourceModel: wsDocumentTableMarked
    }

    DoQueryBox{
        id: queryBox
        visible: false

        onAccept: {
            wsClient.removeRow(queryBox.rowObject.ref, wsDocumentTableMarked)
        }
    }

    header:
        ToolBar{
        id: toolBarFind
        Material.background: Theme === "Light" ? "#efebe9" : "#424242"
        padding: 10

        RoundPane{
            id: txtFind
            visible: !control.findToolBar

            Material.elevation: 0
            anchors.fill: parent
            Row{
//
                anchors.fill: parent
                Image{
                    id: imageQr
                    width: 16
                    height: 16
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/img/honest_sign.png"
                }

                Text{
                    text: control.parentText
                    width: parent.width - txtQuantity.implicitWidth - imageQr.width
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: control.fontPixelSize
                    font.bold: true
                    color: control.theme === "Light" ? "#424242" : "#efebe9"
                    leftPadding: 5
                    elide: Text.ElideRight
                }
                Text{
                    id: txtQuantity
                    text: "Подобрано: <font color=\"#ff0000\">" + wsDocumentTableMarked.rowCount() + " шт.</font>"
                    font.pixelSize: control.fontPixelSize
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                    color: control.theme === "Light" ? "#424242" : "#d69545" //"#efebe9"
                    textFormat: Text.RichText
                }
            }
        }
        Rectangle {
            color: "#efebe9"
            visible: control.findToolBar
            TextInput {
                id: txtFilter
                padding: 5
                text: ""
                font.bold: true
                font.pixelSize: 18
                Material.accent: Material.Blue
                width: parent.width
                onTextChanged: {
                    wsProxyMarkedModel.setFilter("{\"qr_code\":\"%1\"}".arg(txtFilter.text), 8)
                }
                onAccepted: {
                    txtFilter.focus = false
                }
            }
            anchors.fill: parent
            Component.onCompleted: {
                txtFilter.focus = true
            }
        }

        onVisibleChanged: {
            txtFilter.focus = toolBarFind.visible
            if(!toolBarFind.visible && txtFilter.text !== ""){
                txtFilter.text = ""
                wsProxyMarkedModel.setFilter("{\"qr_code\":\"%1\"}".arg(txtFilter.text), 8)
            }
        }
    }


    Column{
        anchors.fill: parent
        spacing: 10

        ListViewControl {
            id: listView
            model: wsProxyMarkedModel
            objectName: "document_table_marked"
            quantityVisible: true
            delegateMenu: ListModel{
                id: contextMenu
                    ListElement{
                        text:"Информация"
                        objectName: "mnuOpen"
                    }
                    ListElement{
                        text:"Удалить"
                        objectName: "mnuDelete"
                    }

            }

            currentRow: wsDocumentTableMarked.currentRow

            onSelectedRow: function(row, object){
                wsDocumentTableMarked.currentRow = row;
            }

            onMenuTriggered: function(row, command, object, comment){
                wsDocumentTableMarked.currentRow = row;
                if(command === "mnuOpen"){
                    control.viewBarcodeInfo(object.qr_code)
                }else if(command === "mnuDelete"){
                    queryBox.text = "Удалить выбранный элемент?"
                    queryBox.uuid = object.ref
                    queryBox.version = Number(object.version); //model.version
                    queryBox.visible = true
                    queryBox.rowObject = object
                }else if(command === "mnuEditQr"){

                }
            }
        }

    }
}
