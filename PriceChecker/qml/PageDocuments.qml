import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

import QmlTableModel 1.2
import "qrc:/js/PageDocumentsJs.js" as PageDocumentsJs

Page {

    id: pageDocs
    padding: 10

    property int fontPixelSizeGrey: screenWidth > 1000 ? 20 : 8
    property int imageMaximumHeight: screenWidth > 1000 ? 400 : 250

    property string currentDocument: ""

    signal selectedDocument(int row, var object)

    property QmlTableModel wsDocuments: QmlTableModel{
        currentObjectName: "Documents"
    }

    function setModel(text){
        PageDocumentsJs.setDcumentsModel(text)
    }

    function newNumber(){
        return PageDocumentsJs.new_number()
    }

    DoQueryBox{
        id: queryBox
        visible: false

        onAccept: {
            wsClient.deleteDocument(queryBox.uuid, queryBox.version)
            wsClient.getDocuments()
        }
    }

    DialogDocumentInfo{
        id: docInfo
        visible: false

        onAccept:function(rowObject){
            let source = "";
            if(rowObject !== undefined){
                try{
                    source = JSON.stringify(rowObject)//wsDocuments.getObjectToString(modelIndex.row)
                }catch(e){
                    source = "";
                }
            }
            wsClient.documentUpdate(docInfo.docNumber, docInfo.docDate, docInfo.docComent, source)
        }

    }

    Column{
        anchors.fill: parent
        spacing: 10
        //anchors.margins: 10

        ListViewControl {
            id: listView
            model: wsDocuments
            objectName: "documents"
            delegateMenu: ListModel{
                id: contextMenu
                    ListElement{
                        text:"Открыть"
                        objectName: "mnuOpen"
                    }
                    ListElement{
                        text:"Удалить"
                        objectName: "mnuDelete"
                    }
            }

            onSelectedRow: function(row, object){
                pageDocs.selectedDocument(row, object)
            }

            onRemoveRow: function(modelindex){

            }

            onMenuTriggered: function(row, command, object, comment){
                if(command === "mnuOpen"){
                    docInfo.docNumber = object.number
                    docInfo.docDate = wsClient.documentDate(object.date)
                    docInfo.docComent = comment
                    docInfo.rowObject = object;
                    docInfo.visible = true
                }else if(command === "mnuDelete"){
                    queryBox.text = "Удалить выбранный документ?"
                    queryBox.uuid = object.ref
                    queryBox.version = Number(object.version); //model.version
                    queryBox.visible = true
                }
            }
        }

    }
}
