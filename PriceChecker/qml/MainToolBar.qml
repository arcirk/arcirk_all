import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

import "qrc:/js/MainToolBarJs.js" as MainToolBarJs

ToolBar {

    id: control
    width: parent.width
    Material.background: Theme === "Light" ? "#efebe9" : "#424242"

    function setConnectionState(state){
        MainToolBarJs.updateIcons(state)
    }

    function updateToolbar(objectName, state){
        MainToolBarJs.updateToolbarButton(objectName, state)
    }

    signal openOptionsDialog();
    signal openDocumentsPage();
    signal leftArrow();
    signal openEnterBarcode();
    signal connectClick();
    signal findClicked(bool checked);
    signal newDocument();
    signal refresh();

    RowLayout{
        ToolButton{
            id: btnArrowleft
            icon.source: "qrc:/img/arrow-left.svg"
            visible: false
            onClicked: {
                control.leftArrow()
            }
        }
        ToolButton{
            id: btnScan
            icon.source: "qrc:/img/qr16.png"
            visible: true
            onClicked: {
                control.openEnterBarcode()
            }
        }
        ToolButton{
            id: btnDocuments
            icon.source: "qrc:/img/file-document-multiple.svg"
            visible: true
            onClicked: {
                control.openDocumentsPage()
            }

        }

        ToolButton{
            id: btnDocumentNew
            icon.source: "qrc:/img/documetAdd.svg"
            visible: false
            onClicked: {
                control.newDocument()
            }
        }
        ToolButton{
            id: btnFind
            icon.source: "qrc:/img/text-search.svg"
            visible: false
            checkable: true
            Material.accent: Material.Blue
            onClicked: {
                control.findClicked(btnFind.checked)
            }
        }
        Label{
            id: lblTitle
            Layout.fillWidth: true
            text:  ""
        }
        width: parent.width
        Row{
            ToolButton{
                id: btnSync
                icon.source: "qrc:/img/cloud_sync.svg"

                onClicked: {
                    wsClient.synchronizeBase();
                    attachFocus.focus = true;
                    control.refresh()
                }

            }
            ToolButton{
                id: btnConnectionState
                icon.source: "qrc:/img/lan_check_offline.png"

                onClicked: {
                    control.connectClick()
                }

            }
            ToolButton{
                id: btnOptions
                icon.source: "qrc:/img/setting.png"

                onClicked: menu.open()

                Menu {
                    id: menu
                    y: btnOptions.height

                    MenuItem {
                        text: "Основные настройки"
                        onTriggered: {
                            control.openOptionsDialog()
                        }
                    }
                    MenuItem {
                        text: "Провирить обновления"
                        onTriggered: {
                            wsClient.checkUpdate()
                        }
                    }
                    MenuItem {
                        text: "Открыть страницу загрузки файлов"
                        onTriggered: {
                            Qt.openUrlExternally(wsSettings.httpUrl)
                        }
                    }
                }
            }
        }


    }
}
