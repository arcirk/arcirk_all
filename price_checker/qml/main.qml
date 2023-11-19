import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

import WebSocketClient 1.0
import Settings 1.0
import BarcodeParser 1.0
import BarcodeInfo 1.0

import "qrc:/js/main.js" as MainJs

ApplicationWindow {
    width: 480 //EDA50K
    height: 800

    visible: true
    title: qsTr("Прайс - чекер")

    Material.theme: Theme === "Light" ? Material.Light : Material.Dark

    Component.onCompleted: {
        wsClient.open(wsSettings)
        attachFocus.focus = wsSettings.keyboardInputMode
    }

    onClosing: {
        if(stackView.currentItem.objectName !== "pageStart")
        {
            close.accepted = false
            stackView.pop()
            mainToolBar.updateToolbar(stackView.currentItem.objectName, true)
        }
    }

    property BarcodeParser wsBarcodeParser: BarcodeParser{
        onBarcode: function(value){
            MainJs.onBarcode(value)
        }
    }

    property BarcodeInfo wsBarcodeInfo: BarcodeInfo {
        id: barcodeInfo;

        onBarcodeInfoChanged: {
            MainJs.barcodeInfoChanged(barcodeInfo.isUpdate)
        }

        onImageSourceChanged: {
            pageScanner.imageSourceChanged(wsBarcodeInfo);
        }

        onClearData: {
            pageScanner.setBarcodeInfo(wsBarcodeInfo);
        }
    }
    property Settings wsSettings: Settings{
        deviceId: machineUniqueId
        onUpdateWorkplaceView: function(org, suborg, stok, price){
            MainJs.updateWorkplaceView(org, suborg, stok, price)
        }

        onOptionsChanged: {
            optionsDlg.updateData()
        }
    }

    DialogDocumentInfo{
        id: docInfo
        visible: false

        onVisibleChanged: {
            attachFocus.focus = !docInfo.visible;
        }

        onAccept: function(rowObject){
            let j = ""
            if(rowObject !== undefined)
                j = JSON.stringify(rowObject);
            wsClient.documentUpdate(docInfo.docNumber, docInfo.docDate, docInfo.docComent, j)
        }
    }

    OptionsDialog {
        id: optionsDlg
        visible: false

        onWebSocketConnect: {
            if(!wsClient.isStarted()){
                wsClient.open(wsSettings);
            }
        }

        onVisibleChanged: {
            attachFocus.focus = !optionsDlg.visible;
        }

        onAppModeChanged: {
            if(stackView.currentItem != pageStart)
                stackView.pop()
            mainToolBar.updateToolbar("pageStart")
        }
    }

    property WebSocketClient wsClient: WebSocketClient{
        id: webSocketClient

        onDisplayError: function(what, err){
            popupMessage.addNotify("error", err);
            popupMessage.showNotify();
            if(!wsClient.isStarted())
                wsClient.checkConnection();
        }

        onNotify: function(message){
            popupMessage.addNotify("message", message);
            popupMessage.showNotify();
        }

        onConnectionChanged: function(state){
            mainToolBar.setConnectionState(state)
            MainJs.onConnectionChanged(state)
            optionsDlg.connectionState(state)
        }

        onConnectionSuccess: {
            wsClient.init_data_options();
        }

        onReadDocuments: function(jsonModel){
            if(stackView.currentItem == pageDocuments)
                stackView.currentItem.setModel(jsonModel);
        }
        onReadDocumentTable: function(jsonModel){
            MainJs.openPage(pageDocument)
            if(stackView.currentItem == pageDocument)
                stackView.currentItem.setModel(jsonModel);
        }

        onUpdateBarcode: function(barcode){
            if(stackView.currentItem == pageDocument){
                wsBarcodeInfo.isUpdate = true
                wsClient.get_barcode_information(barcode, wsBarcodeInfo)
            }
        }

        onReadDocumentMarkedTable: function(jsonModel){
            if(stackView.currentItem == pageMarkedItems)
                stackView.currentItem.setModel(jsonModel);
        }
    }

    EnterBarcodeDialog {
        id: enterBarcodeDlg
        visible: false

        onEnterBarcodeFromKeyboard: function(message){
            MainJs.onMessage(message)
        }

        onVisibleChanged: {
            attachFocus.focus = !enterBarcodeDlg.visible;
        }
    }

    Item {
        //При использовании клавиатурного ввода на сканере
        id: attachFocus
        focus: wsSettings.keyboardInputMode
        Keys.onPressed: (event)=>{
            if(!attachFocus.focus)
                return;
            wsBarcodeParser.addSumbol(event.key, event.text);
        }
    }
///////////////////////////////////////////////////////////////////////////////////////////////////
    StackView{
        id: stackView
        anchors.fill: parent

        initialItem: PageStart{
            id: pageStart
            objectName: "pageStart"

            onSelectCommand: function(name){
                if(name === "price"){
                    MainJs.openPageScanner(false);
                    mainToolBar.updateToolbar("pageScanner", false)
                }else if(name === "docs"){
                    MainJs.openPage(pageDocuments)
                    mainToolBar.updateToolbar("pageDocuments", true)
                    if(stackView.currentItem == pageDocuments){
                        wsClient.getDocuments();
                    }
                }else if(name === "sessions"){
                    MainJs.openPage(pageSessions)
                    if(stackView.currentItem == pageSessions){
                        //wsClient.get1CSessions();
                    }
                }
            }
        }
    }
    PageScanner{
        objectName: "pageScanner"
        id: pageScanner
        visible: false

        Component.onCompleted: {
            if(wsSettings.keyboardInputMode)
                attachFocus.focus = true
        }

    }
    PageDocuments{
        objectName: "pageDocuments"
        id: pageDocuments
        visible: false
//        onGetContent: function(ref){
//            pageDocument.ref = ref;
//            wsClient.getDocumentContent(ref)
//            updateToolbarButton(false);
//        }
        onSelectedDocument: function(row, object){
            pageDocument.rowObject = object
            pageDocument.row = row
            wsClient.getDocumentContent(object.ref)
            mainToolBar.updateToolbar("pageDocument", true)
        }
    }

    PageDocument{
        objectName: "pageDocument"
        id: pageDocument
        visible: false
        onViewBarcodeInfo: function(barcode){
            if(barcode.trim() !== ''){
                 MainJs.openPageScanner(true)
                 MainJs.onMessage(barcode)
            }
        }
        onOpenMarkedTable: function(docRef, itemRef, itemText, quantity){
            pageMarkedItems.docRef = docRef
            pageMarkedItems.itemRef = itemRef
            pageMarkedItems.parentText = itemText
            pageMarkedItems.parentQuantity = quantity
            MainJs.openPage(pageMarkedItems)
            wsClient.getDocumentMarkedContent(itemRef)

        }
    }

    PageMarkedItems{
        objectName: "pageMarkedItems"
        id: pageMarkedItems
        visible: false

        onQuantityChanged: function(quantity, parentQuantity){
            if(pageMarkedItems.is_document_1c){
                page1CDocument.rowChanged(pageMarkedItems.currentBarcode, quantity, parentQuantity)
            }
        }
    }

    PageSessions{
        objectName: "pageSessions"
        id: pageSessions
        visible: false

//        onGetForms: function(session_uuid){
//            openPage(pageSessionForms)
//            wsClient.getSessionForms(session_uuid)
//        }
    }
    PageSessionForms{
        objectName: "pageSessionForms"
        id: pageSessionForms
        visible: false

//        onGetContent: function(uuid_form, session_uuid){
//            page1CDocument.session_uuid = session_uuid;
//            page1CDocument.form_uuid = uuid_form;
//            openPage(page1CDocument)
//            wsClient.getDocument1CContent(session_uuid, uuid_form, "Товары")
//            footerRow.text = "Выгрузка документа ..."
//            footerRow.visible = true
//        }
    }

    Page1CDocument{
        objectName: "page1CDocument"
        id: page1CDocument
        visible: false
//        onEndLoading: {
//            footerRow.visible = false
//        }
//        onViewBarcodeInfo: function(barcode){
//            if(barcode.trim() !== ''){
//                 openPageScanner(true);
//                 wsClient.get_barcode_information(barcode, wsBarcodeInfo, false, true)
//            }
//        }
//        onOpenMarkedTable: function(session, form, uuid, itemText, quantity, barcode){
//            pageMarkedItems.docRef = form
//            pageMarkedItems.itemRef = uuid
//            pageMarkedItems.parentText = itemText
//            pageMarkedItems.parentQuantity = quantity
//            pageMarkedItems.session_uuid = session
//            pageMarkedItems.is_document_1c = true
//            pageMarkedItems.currentBarcode = barcode
//            pageMarkedItems.form_uuid = form;
//            wsClient.getDocument1CContent(session, form, "ШтрихкодыУпаковок", uuid, barcode)
//            updateToolbarButton(false)
//        }
    }


    //////////////////////////////////////////////////////////////////////////////
    menuBar: MainToolBar {
        id: mainToolBar

        onOpenOptionsDialog: {
            optionsDlg.visible = true
        }

        onOpenDocumentsPage: {
            MainJs.openDocumentsPage()
            mainToolBar.updateToolbar(false)
            if(stackView.currentItem == pageDocuments){
                wsClient.getDocuments();
            }
        }

        onLeftArrow: {
            MainJs.leftArrow()
        }

        onOpenEnterBarcode: {
            enterBarcodeDlg.br = ""
            enterBarcodeDlg.visible = true
        }

        onConnectClick: {
            if(!wsClient.isStarted()){
                wsClient.open(wsSettings);
            }else{
                wsClient.close();
            }
            attachFocus.focus = true;
        }

        onFindClicked:function(checked) {
            if(stackView.currentItem == pageDocument){
                pageDocument.findToolBar = checked;
                attachFocus.focus = !checked;
            }else if(stackView.currentItem == pageMarkedItems){
                pageMarkedItems.findToolBar = checked;
                attachFocus.focus = !checked;
            }else if(stackView.currentItem == page1CDocument){
                page1CDocument.findToolBar = checked;
                attachFocus.focus = !checked;
            }
        }

        onNewDocument: {
            docInfo.docDate = Qt.formatDateTime(new Date(), "dd.MM.yyyy hh:mm:ss")
            docInfo.docNumber = pageDocuments.newNumber();
            docInfo.isNew = true
            docInfo.visible = true
        }

        onRefresh: {
            if(stackView.currentItem == pageDocument){
                wsClient.verifyDocument(pageDocument.documentRef())
            }
        }
    }

    footer: Row {
        height: 24
        visible: false
        id: footerRow
        spacing: 10
        property alias text: txtLoadComm.text
       AnimatedImage{
           height: 24
           width: 24
           source: "qrc:/img/animation_loaded.gif"
       }
       Text{
           id: txtLoadComm
           height: 24
           color: Theme === "Light" ? "#424242" : "#efebe9"
           verticalAlignment: Qt.AlignVCenter
           padding: 0
           text: "Скачивание файла обновления ..."
       }
    }

    Row{
        id: poupText
        anchors.top: stackView.bottom
        width: stackView.width
        Layout.fillWidth: true
        PopupMessage{
            id: popupMessage
            parent: poupText
            anchors.centerIn: parent
        }
    }

    Connections {
        target: qtAndroidService
        function onMessageFromService(message) {
            MainJs.onMessage(message)
        }
    }
}
