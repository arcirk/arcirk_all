import QtQuick 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.15

import "qrc:/js/ListItemDelegateJs.js" as ListItemDelegateJs

RoundPane {
    id: control
    padding: 2

    property int fontPixelSize: screenWidth > 1000 ? 20 : 16

    property alias text: txt.text
    property alias elide: txt.elide
    property QtObject rowObject: undefined
    property QtObject menu: undefined
    property bool checkable: false
    property bool isSelected: false
    property int row: 0
    property string objectName: "documents"

    signal popupMenuTriggered(string name)
    signal itemClicked(string buttonId)

    Material.elevation: {
        if(isSelected)
            1
        else
            7
    }

    radius: 3

    function setData(dump){
        rowObject = JSON.parse(dump)
    }

    function setMenu(menu){
        control.menu = menu
    }

    Material.background:{

        if(control.checked && !control.chldrenList){
            if(!control.isSelected){
                if(Theme === "Dark")
                    "#424242"
                else
                    "#efebe9"
            }else{
                "red"
            }
        }else
            if(!control.isSelected)
                undefined
            else
                "#424242"
    }



    Column{
        id: colControl
        spacing: 0
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width
        Row{
            id:rowDetalis
            width: colControl.width
            spacing: 5
            height: txt.implicitHeight

            Menu {
                id: contextMenu

                Instantiator {
                   model: control.menu
                   MenuItem {
                      text: model.text
                   }
                   onObjectAdded: contextMenu.insertItem(index, object)
                   onObjectRemoved: contextMenu.removeItem(object)
               }
            }

            Row{
                id: rowImages
                leftPadding: 10
                anchors.verticalCenter: parent.verticalCenter
                Image {
                    id: imgErr
                    width: 16
                    height: 16
                    source: "qrc:/img/!.png"
                    visible:  {
                        if(control.listTable === "document_table"){
                            let nomenclature = wsDocumentTable.get(wsProxyModel, control.modelIndex.row, "nomenclature")
                            return control.quantityVisible ? (nomenclature === "") : false
                        }else
                            return false
                    }
                }
                Image{
                    id: imageQr
                    width: 16
                    height: 16
                    source: "qrc:/img/honest_sign.png"
                    //visible:  control.quantityVisible ? (control.modelIndex.is_marked === 1) : false
                    visible: {
                        let is_marked = ListItemDelegateJs.item_is_marked()
                        return control.quantityVisible ? is_marked : false
                    }
                }
            }
            Text {
                id: txt;
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                anchors.left: rowImages.right
                text: control.getSynonim()
                font.bold: control.isSelected
                width: rowDetalis.width - quantity.implicitWidth - rowImages.implicitWidth//control.quantityVisible ? parent.width - quantity.implicitWidth - (imgErr.visible ? imgErr.height : 0) - (imageQr.visible ? imageQr.height : 0) - 10 : parent.width
                font.pixelSize: control.fontPixelSize
                color: control.theme === "Light" ? "#424242" : "#efebe9"//"#d6cf9a" //"#9acfd6" //
                leftPadding: imageQr.visible ? 5 : 0
                topPadding: 5
                bottomPadding: 5
                anchors.verticalCenter: parent.verticalCenter
                MouseArea{
                    id:mouseArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor;

                    onClicked: {
                        control.clicked(control.row)
                    }
                    onEntered: {
                        if(!control.checkable){
                            control.Material.elevation = 1
                        }
                    }
                    onExited: {
                        if(!control.checkable)
                            control.Material.elevation = 7
                    }
                    onPressAndHold: {
                        if (!control.menu !== undefined)
                            contextMenu.popup()
                    }
                }
            }
            Text {
                id: quantity
                visible: control.quantityVisible
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: ListItemDelegateJs.quantityText()
                leftPadding: 10
                rightPadding: 5
                font.pixelSize: control.fontPixelSize
                color: Theme === "Light" ? "#424242" : "#d69545" //"#efebe9"
                font.bold: control.isSelected
                textFormat: Text.RichText

                MouseArea{
                    id:mouseAreaQuantity
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor;

                    onClicked: {
                        control.clicked(control.row)
                    }
                    onEntered: {
                        if(!control.checkable){
                            control.Material.elevation = 1
                        }
                    }
                    onExited: {
                        //setSelectedBackground();
                        if(!control.checkable)
                            control.Material.elevation = 7
                    }
                    onPressAndHold: {
                            if (!control.menuDisable)
                                contextMenu.popup()
                        }

                }
            }
        }



        Pane{
            leftPadding: 10
            rightPadding: 10
            topPadding: 0
            bottomPadding: 0
            anchors.top: rowDetalis.bottom// txt.bottom
            Rectangle {
                width: control.width - 20
                height: 1
                color: "gray"
                visible: ListItemDelegateJs.getComment() !== "" ? true : false

            }
        }
        Text {
            id: txtComment;
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            text: ListItemDelegateJs.getComment()
            width: parent.width
            font.pixelSize: control.fontPixelSize
            color: Theme === "Light" ? "#424242" : "#d69545" //"#efebe9"
            leftPadding: 10
            rightPadding: 10
            topPadding: 5
            bottomPadding: 5
            visible: ListItemDelegateJs.getComment() !== "" ? true : false

            MouseArea{
                id:mouseAreaComment
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor;

                onClicked: {
                    control.clicked(control.row)
                    console.log("onClicked");
                }
                onEntered: {
                    if(!control.checkable){
                        control.Material.elevation = 1
                    }
                    console.log("onEntered");
                }
                onExited: {
                    if(!control.checkable)
                        control.Material.elevation = 7
                    console.log("onExited");
                }
                onPressAndHold: {
                        if (control.alowMenu)
                            contextMenu.popup()
                    }
            }
        }
    }

}
