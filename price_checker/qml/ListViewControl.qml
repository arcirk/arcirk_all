import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12


ListView {
    id: control
    leftMargin: 10
    rightMargin: 10
    anchors.fill: parent
    displayMarginBeginning: 40
    displayMarginEnd: 40
    spacing: 12

    property string objectName: ""

    function setSourceModel(modelObject){
        control.model = modelObject
    }

    signal selectedRow(var modelindex)
    signal removeRow(var modelindex)

    delegate: Column {
        spacing: 6
        Row {
            id: messageRow
            spacing: 6
            Layout.fillWidth: true
            ListItemDelegate{
                id: delegate
                objectName: control.objectName
                width: listView.width - messageRow.spacing - 12
                rowObject: JSON.parse(control.model.dump(model) )

//                onMenuTriggered: function(command){
//                    console.debug("onMenuTriggered: " + command)
//                    if(command === "mnuOpen"){
//                        docInfo.docNumber = wsDocuments.get(model.row, "number") //model.number
//                        docInfo.docDate = wsClient.documentDate(wsDocuments.get(model.row, "date")) //model.date
//                        docInfo.docComent = delegate.getComment()
//                        //docInfo.docSourceComment = delegate.getSourceComment();
//                        docInfo.modelIndex = model;
//                        docInfo.visible = true
//                    }else if(command === "mnuDelete"){
//                        queryBox.text = "Удалить выбранный документ?"
//                        queryBox.uuid = wsDocuments.get(model.row, "ref") //model.ref
//                        queryBox.version = Number(wsDocuments.get(model.row, "version")); //model.version
//                        queryBox.visible = true
//                    }
//                }
//                onClicked: function(row){
//                    console.log("onSelectItem " + row)
//                    listView.selectedRow(wsDocuments.index(model.row,0))
//                }
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
