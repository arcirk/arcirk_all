import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

ListView {
    id: control
//    leftMargin: 10
//    rightMargin: 10
//    topMargin: 10
    anchors.fill: parent
    displayMarginBeginning: 40
    displayMarginEnd: 40
    spacing: 12


    property int currentRow: -1
    property bool quantityVisible: false
    //property var elide: Text.ElideRight
    property ListModel delegateMenu: undefined
    property var menuProperty: undefined

    function setSourceModel(modelObject){
        control.model = modelObject
    }

    signal selectedRow(int row, var object)
    signal removeRow(var modelindex)
    signal menuTriggered(int row, string command, var object, string comment)

    delegate: Column {
        spacing: 6
        property QtObject parentModel: control.model
        Row {
            id: messageRow
            spacing: 6
            Layout.fillWidth: true
            ListItemDelegate{
                id: delegate
                objectName: control.objectName
                width: control.width - messageRow.spacing + 5// - 12
                row: model.row
                menu: control.delegateMenu
                quantityVisible: control.quantityVisible
                isSelectedItem: control.currentRow === model.row ? true : false
                menuProperty: control.menuProperty

                onPopupMenuTriggered: function(row, command, object, comment){
                    control.menuTriggered(row, command, object, comment)
                }
                onItemClicked: function(row, object){                    
                    control.selectedRow(row, object)
                }
            }
        }
    }

//    onSelectedRow: function(index){
//        //var uuid = ""
//        //var iUuid = wsDocuments.getColumnIndex("ref")
//        let ref = model.get(index.row, "ref")
//        //if(iUuid !== -1){
//        //    uuid = wsDocuments.value(index, Qt.UserRole + iUuid)
//        //    console.log(uuid)
//            pageDocs.getContent(ref)
//        //}else
//        //    return;

////                var doc = ""
////                var iDoc = wsDocuments.getColumnIndex("document_name")
////                if(iDoc !== -1){
////                    doc = wsDocuments.value(index, Qt.UserRole + iDoc)
////                }
////                var uuid_form = ""
////                var iUuid_form = wsDocuments.getColumnIndex("uuid_form")
////                if(iUuid_form !== -1){
////                    uuid_form = wsDocuments.value(index, Qt.UserRole + iUuid_form)
////                }
////                pageDocs.selectDocument(uuid, doc, uuid_form)
//    }

    ScrollBar.vertical: ScrollBar {}
}
