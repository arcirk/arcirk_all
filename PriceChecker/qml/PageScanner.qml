import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

import "qrc:/js/PageScannerJs.js" as PageScannerJs

Page {

    id: pageConnanctions
    padding: 10
    property alias organization: org.text
    property alias subdivision: suborg.text
    property alias warehouse: stock.text
    property alias price: priceText.text
    property bool isConnection: false

    property int fontPixelSizeGrey: screenWidth > 1000 ? 20 : 12
    property int imageMaximumHeight: screenWidth > 1000 ? 400 : 200

    property bool hideBalance: false

    Material.background: "white"

    function imageSourceChanged(bInfo){
        PageScannerJs.changeImageSource(bInfo)
    }

    function setBarcodeInfo(bInfo){
        PageScannerJs.setBarcode(bInfo)
    }

    GridLayout{
        id: grid
        anchors.top: parent.top
        z: 1
        columns: 2

           Text{
               leftPadding: 10
               topPadding: 10
               color: "gray"
               text: "Организация:"
               font.pixelSize: fontPixelSizeGrey
           }
           Text{
               id: org
               topPadding: 10
               color: "gray"
               font.pixelSize: fontPixelSizeGrey
           }

           Text{
               leftPadding: 10
               text: "Подразделение:"
               color: "gray"
               font.pixelSize: fontPixelSizeGrey
           }
           Text{
               id: suborg
               color: "gray"
                font.pixelSize: fontPixelSizeGrey
           }
           Text{
               leftPadding: 10
               text: "Склад:"
               color: "gray"
               font.pixelSize: fontPixelSizeGrey
           }
           Text{
               color: "gray"
               id: stock
               font.pixelSize: fontPixelSizeGrey
           }
           Text{
               leftPadding: 10
               text: "Тип цен:"
               color: "gray"
               font.pixelSize: fontPixelSizeGrey
           }
           Text{
               color: "gray"
               id: priceText
               font.pixelSize: fontPixelSizeGrey
           }
    }

    Pane{
        id: pane
        anchors.centerIn: parent
        width: parent.width
        visible: false

        GridLayout{
            columns: 5
            rows: 5
            id: column
            width: pane.width - pageConnanctions.padding * 2

            RowLayout{
                Layout.columnSpan: 5
                Layout.rowSpan: 1
                Layout.row: 1
                Layout.column: 1
                Layout.alignment: Qt.AlignCenter
                Layout.maximumHeight: imageMaximumHeight
                Layout.minimumHeight: imageMaximumHeight
                visible: isConnection
                id: rowImage

                Image {
                    id: imageData
                    Layout.maximumHeight: imageMaximumHeight
                    Layout.maximumWidth: pane.width - pageConnanctions.padding * 2
                    fillMode:Image.PreserveAspectFit; clip:true
                    visible: wsSettings.showImage
                }

                AnimatedImage{
                    id: longOperation
                    source: "qrc:/img/longOperation.gif"
                    Layout.alignment: Qt.AlignCenter
                    visible: false
                }

            }


            Text {

                Layout.fillWidth: true
                Layout.columnSpan: screenWidth > 1000 ? 3 : 5
                Layout.rowSpan: 1
                Layout.row: 2
                Layout.column: screenWidth > 1000 ? 2 : 1
                id: txtBarcode
                padding: 4
                font.pixelSize: fontPixelSizeGrey + 8
                width: column.width / 2
                fontSizeMode: Text.Fit
                minimumPixelSize: 8 // minimum height of the font
                font.bold: true
                //color: "blue"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: ""
            }
            Text {
                Layout.fillWidth: true
                Layout.columnSpan: screenWidth > 1000 ? 3 : 5
                Layout.rowSpan: 1
                Layout.row: 3
                Layout.column: screenWidth > 1000 ? 2 : 1
                id: txtName
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                width: column.width / 2
                horizontalAlignment: Qt.AlignHCenter
                font.pixelSize: fontPixelSizeGrey + 18
                color: "#536AC2"
                text: ""
                Layout.maximumWidth: pane.width - pageConnanctions.padding * 2

            }


            GridLayout{
                Layout.columnSpan: 1
                Layout.rowSpan: 1
                Layout.row: 4
                Layout.column: 3
                Layout.alignment: Qt.AlignCenter
                columns: 2
                visible: isConnection
                Text {
                    text: "Цена:"
                    horizontalAlignment: Qt.AlignHCenter
                    font.pixelSize: fontPixelSizeGrey + 8
                }
                Text {
                    id: txtPrice
                    textFormat: Text.RichText
                    horizontalAlignment: Qt.AlignHCenter
                    font.pixelSize: fontPixelSizeGrey + 8
                    font.bold: true
                }
                Text {
                    id: lblBalance
                    text: "Остаток:"
                    font.pixelSize: fontPixelSizeGrey + 8
                    visible: !pageConnanctions.hideBalance
                }
                Text {
                    id: txtStockBalance
                    textFormat: Text.RichText
                    horizontalAlignment: Qt.AlignHCenter
                    font.pixelSize: fontPixelSizeGrey + 8
                    font.bold: true
                    visible: !pageConnanctions.hideBalance
                }

            }
        }

    }

}
