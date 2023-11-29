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
    property var rowObject: ListItemDelegateJs.getCurrentObject()
    property ListModel menu: undefined
    property var menuProperty: undefined
    property bool checkable: false
    property bool isSelected: false
    property bool isSelectedItem: false
    property bool quantityVisible: false
    property int row: 0
    property var menuItemsData: undefined

    signal popupMenuTriggered(int row, string name, var object, string comment)
    signal itemClicked(int row, var object)
    //signal errorMessage(string message)

    Material.elevation: {
        if(isSelected)
            1
        else
            7
    }

    radius: 3

    function setMenu(menu){
        control.menu = menu
    }

    Material.background:{

        if(control.checked && !control.chldrenList){
            if(!control.isSelectedItem){
                if(Theme === "Dark")
                    "#424242"
                else
                    "#efebe9"
            }else{
                "red"
            }
        }else
            if(!control.isSelectedItem)
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
                        visible: {
                          let v = control.menuProperty;
                          if(v === undefined)
                              return true
                          else{
                              let obj = JSON.parse(v)
                              let prop = obj[model.objectName]
                              if(prop !== undefined){
                                  let val = prop['visible']
                                  if(val !== undefined){
                                        let data_ = rowObject[val.name]
                                        if(data_ === val.value){
                                          return true
                                        }else
                                          return false
                                  }else
                                      return true
                              }else
                                  return true
                          }
                        }
                        height: visible ? implicitHeight : 0

                        onTriggered: {
                         control.popupMenuTriggered(control.row, model.objectName, control.rowObject, ListItemDelegateJs.getComment())
                        }
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
                        if(control.objectName === "document_table"){
                            let nomenclature = rowObject.nomenclature
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
                text: {
                    let v = ListItemDelegateJs.getSynonim()
                    if(v !== undefined)
                        return v
                    else
                        return ""
                }
                font.bold: control.isSelected
                width: rowDetalis.width - quantity.implicitWidth - rowImages.implicitWidth
                font.pixelSize: control.fontPixelSize
                color: Theme === "Light" ? "#424242" : "#efebe9"
                leftPadding: imageQr.visible ? 5 : 0
                topPadding: 5
                bottomPadding: 5
                anchors.verticalCenter: parent.verticalCenter
                MouseArea{
                    id:mouseArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor;

                    onClicked: {
                        control.itemClicked(control.row, rowObject)
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
                        if (control.menu !== undefined)
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
                color: Theme === "Light" ? "#424242" : "#d69545"
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
                    control.itemClicked(control.row, rowObject)
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
                        if (control.menu !== undefined)
                            contextMenu.popup()
                    }
            }
        }
    }

}
