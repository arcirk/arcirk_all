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

    property QmlTableModel wsDocuments: QmlTableModel{
        currentObjectName: "Documents"
    }

    function setModel(text){
        PageDocumentsJs.setDcumentsModel(text)
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

        onAccept:function(modelIndex){
            let source = "";
            if(modelIndex !== undefined){
                try{
                    source = wsDocuments.getObjectToString(modelIndex.row)
                }catch(e){
                    source = "";
                }
            }
            wsClient.documentUpdate(docInfo.docNumber, docInfo.docDate, docInfo.docComent, source, docInfo.uuid)
        }

    }

    Column{
        anchors.fill: parent
        spacing: 10

        ListViewControl {
            id: listView
            model: wsDocuments

            onSelectedRow: function(modelindex){

            }

            onRemoveRow: function(modelindex){

            }
        }

    }
}
