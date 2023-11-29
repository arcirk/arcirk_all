import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

Page {

    id: pageStart
    padding: 10

    property alias organization: org.text
    property alias subdivision: suborg.text
    property alias warehouse: stock.text
    property alias price: priceText.text

    property int fontPixelSizeGrey: screenWidth > 1000 ? 20 : 12
    property int fontPixelSize: screenWidth > 1000 ? 20 : 16

    Material.background: "white"

    signal selectCommand(string name)

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

    Column{
        width: parent.width
        anchors.centerIn: parent
        visible: !wsSettings.priceCheckerMode
        spacing: 10
        RoundPane{
            id: control2
            radius: 3
            Material.elevation: 7
            width: parent.width / 3
            height: parent.width / 3
            anchors.horizontalCenter: parent.horizontalCenter
            Column{
                anchors.centerIn: parent
                width: parent.width
                Image {
                    //id: name
                    source: "qrc:/img/scan.png"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    id: txtPriceChecker
                    text: "<html><head></head>
                            <body>
                            <div style=\"text-align: center;\">Прайс - чекер</div></body></html>"
                    font.bold: true
                    font.pixelSize: fontPixelSize
                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
            }
            MouseArea{
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor;

                onClicked: {
                    console.log("onClicked");

                }
                onEntered: {
                       control2.Material.elevation = 1
                    console.log("onEntered");
                }
                onExited: {
                    control2.Material.elevation = 7
                    pageStart.selectCommand("price")

                }
            }
        }
        RoundPane{
            id: control1
            radius: 4
            Material.elevation: 7
            width: parent.width / 3
            height: parent.width / 3
            anchors.horizontalCenter: parent.horizontalCenter

            Column{
                anchors.centerIn: parent
                width: parent.width
                Image {
                    //id: name
                    source: "qrc:/img/documentation.png"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    id: txtToBarcodes
                    text: "<html><head></head>
                            <body>
                            <div style=\"text-align: center;\">Подбор</div>
                            <div style=\"text-align: center;\">штрихкодов</div></body></html>"
                    font.bold: true
                    font.pixelSize: fontPixelSize
                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
            }
            MouseArea{
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor;

                onClicked: {
                    console.log("onClicked");
                }
                onEntered: {
                    control1.Material.elevation = 1
                    console.log("onEntered");
                }
                onExited: {
                    control1.Material.elevation = 7
                    pageStart.selectCommand("docs")

                }
            }
        }
        RoundPane{
            id: control
            radius: 3
            Material.elevation: 7
            width: parent.width / 3
            height: parent.width / 3
            visible: false
            anchors.horizontalCenter: parent.horizontalCenter
            Column{
                anchors.centerIn: parent
                width: parent.width
                Image {
                    //id: name
                    source: "qrc:/img/to1C.png"
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: false
                }
                Text {
                    id: txtTo1cDocument
                    text: "<html><head></head>
                            <body>
                            <div style=\"text-align: center;\">Подбор</div>
                            <div style=\"text-align: center;\">в документ</div></body></html>"
                    font.bold: true
                    font.pixelSize: fontPixelSize
                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap
                    width: parent.width
                    visible: false
                }
            }

            MouseArea{
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor;

                onClicked: {
                    console.log("onClicked");
                }
                onEntered: {
                    control.Material.elevation = 1
                    console.log("onEntered");
                }
                onExited: {
                    control.Material.elevation = 7
                    pageStart.selectCommand("sessions")
                }

            }

        }
    }

}
