import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

import QmlTableModel 1.2
import QmlSortModel 1.0

import "qrc:/js/PageDocumentJs.js" as PageDocumentJs

Page {

    id: pageDoc

    property string currentBarcode: ""
    property int currentQuantity: 0
    property int fontPixelSizeGrey: screenWidth > 1000 ? 20 : 8
    property int fontPixelSize: screenWidth > 1000 ? 20 : 16
    property int imageMaximumHeight: screenWidth > 1000 ? 400 : 250
    property bool findToolBar: false

    property var rowObject: undefined
    property int row: -1

    function setModel(text){
        PageDocumentJs.setDcumentContentModel(text)
    }

    function setBarcode(bInfo){
        PageDocumentJs.setBarcode(bInfo)
    }

    function updateBarcodeInfo(bInfo){
        wsClient.documentModelUpdateItem(bInfo, wsDocumentTable)
    }

    function documentRef(){
        if(rowObject !== undefined)
            return rowObject.ref;
        else
            return ""
    }

    signal viewBarcodeInfo(string br);
    signal openMarkedTable(string docRef, string itemRef, string itemText, int quantity)

    DoQueryBox{
        id: queryBox
        visible: false

        onAccept: {
            wsClient.removeRow(queryBox.rowObject.ref, wsDocumentTable)
        }
    }

    function setFilterBarcode(value){
        txtFilter.text = value
    }

    property QmlTableModel wsDocumentTable: QmlTableModel{
        onCurrentRowChanged: {
            wsDocumentTable.reset();
        }
        currentObjectName: "DocumentsTables"
    }
    property QmlSortModel wsProxyModel: QmlSortModel{
        sourceModel: wsDocumentTable
    }

    DialogEditBarcode {
        id: dlgEdit
        visible: false

        onAccept: {
            if(dlgEdit.quantity.trim() === ''){
                wsClient.displayError("Ошибка", "Не указано количество!")
                return;
            }
            if(Number(dlgEdit.quantity) === 0){
                wsClient.displayError("Ошибка", "Не указано количество!")
                return;
            }
            wsClient.documentTableSetQuantity(dlgEdit.uuid, Number(dlgEdit.quantity), wsDocumentTable)
        }
    }

    header:
        ToolBar{
        id: toolBarFind
        visible: pageDoc.findToolBar
        Material.background: Theme === "Light" ? "#efebe9" : "#424242"
        padding: 10
        Rectangle {
            color: "#efebe9"
            TextInput {
                id: txtFilter
                padding: 5
                text: ""
                font.bold: true
                font.pixelSize: 18
                Material.accent: Material.Blue
                width: parent.width
                onTextChanged: {
                    wsProxyModel.setFilter("{\"representation\":\"%1\"}".arg(txtFilter.text), 8)
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
                wsProxyModel.setFilter("{\"representation\":\"%1\"}".arg(txtFilter.text), 8)
            }
        }

    }
    Column{
        anchors.fill: parent
        spacing: 10
        anchors.margins: 10

        ListViewControl {
            id: listView
            model: wsProxyModel
            objectName: "document_table"
            quantityVisible: true
            menuProperty: JSON.stringify({mnuEditQr: {visible: {name: "is_marked", value: 1}}})
            delegateMenu: ListModel{
                id: contextMenu
                    ListElement{
                        text:"Подбор Qr кодов"
                        objectName: "mnuEditQr"
                    }
                    ListElement{
                        text:"Информация"
                        objectName: "mnuOpen"
                    }
                    ListElement{
                        text:"Удалить"
                        objectName: "mnuDelete"
                    }

            }

            currentRow: wsDocumentTable.currentRow

            onSelectedRow: function(row, object){
                wsDocumentTable.currentRow = row;
                dlgEdit.row = row
                dlgEdit.uuid = object.ref
                dlgEdit.barcode = object.barcode
                dlgEdit.quantity = object.quantity
                dlgEdit.synonim = object.good
                dlgEdit.unit = object.unit
                dlgEdit.visible = true
            }

            onMenuTriggered: function(row, command, object, comment){
                wsDocumentTable.currentRow = row;
                if(command === "mnuOpen"){
                    pageDoc.viewBarcodeInfo(object.barcode)
                }else if(command === "mnuDelete"){
                    queryBox.text = "Удалить выбранный элемент?"
                    queryBox.uuid = object.ref
                    queryBox.version = Number(object.version); //model.version
                    queryBox.visible = true
                    queryBox.rowObject = object
                }else if(command === "mnuEditQr"){
                    let item_ref = object.ref
                    let item_representation = object.representation
                    let item_quantity = Number(object.quantity)
                    pageDoc.openMarkedTable(pageDoc.ref, item_ref, item_representation, item_quantity)
                }
            }
        }

    }
}
