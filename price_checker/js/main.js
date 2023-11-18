function openPageScanner(isViewOnly){
    var item = stackView.find(function(item) {
        return item.objectName === "pageScanner";
    })
    pageScanner.hideBalance = isViewOnly
    if(item === null){
        stackView.push(pageScanner);
    }else{
        if(stackView.currentItem !== item)
             stackView.push(pageScanner);
    }
    mainToolBar.updateToolbar(stackView.currentItem.objectName, isViewOnly);
}

function openPage(page){
    var item = stackView.find(function(item) {
        return item.objectName === page.objectName;
    })
    if(item === null)
        stackView.push(page);
    else{
        stackView.pop(item)
    }

    mainToolBar.updateToolbar(stackView.currentItem.objectName, false);

}

function setRecBarcode(page, message){
    if(!page.findToolBar){
        if(message.length > 13){
            popupMessage.addNotify("error", "Ошибочный штрихкод!")
            popupMessage.showNotify()
            return;
        }
        wsClient.get_barcode_information(message, wsBarcodeInfo, true)
    }else
        page.setFilterBarcode(message)
}

function onBarcode(value){
    if(stackView.currentItem.objectName !== "pageDocument"){
        wsClient.barcode_info_from_1c_service(value, wsBarcodeInfo)
    }else{
        if(!pageDocument.findToolBar)
            wsClient.get_barcode_information(value, wsBarcodeInfo, true)
        else
            pageDocument.setFilterBarcode(value)
    }
}

function onMessage(message){
    if(wsSettings.priceCheckerMode){
        openPageScanner(false);
    }
    if(stackView.currentItem.objectName === "pageDocument"){
        //setRecBarcode(pageDocument, message);
    }else if(stackView.currentItem.objectName === "pageMarkedItems"){
//        if(!pageMarkedItems.findToolBar){
//            pageMarkedItems.setBarcode(message)
//        }else
//            pageMarkedItems.setFilterBarcode(message)
    }else if(stackView.currentItem.objectName === "page1CDocument"){
//        if(!page1CDocument.findToolBar)
//            wsClient.get_barcode_information(message, wsBarcodeInfo, true)
//        else
//            page1CDocument.setFilterBarcode(message)
//        setRecBarcode(page1CDocument, message);
    }else{
        wsClient.barcode_info_from_1c_service(message, wsBarcodeInfo)
    }
    //console.log("barcode: " + message)
}

function onConnectionChanged( state){

    if(state){
        popupMessage.addNotify("message", "Успешное подключение к серверу");
    }else
        popupMessage.addNotify("error", "Сервер не доступен!");
    popupMessage.showNotify();

    if(optionsDlg.visible)
        optionsDlg.connectionState(state);

    if(!state)
        wsClient.startReconnect();

    pageScanner.isConnection = state

    if(!state){
        if(stackView.currentItem.objectName === "pageSessions" ||
                stackView.currentItem.objectName === "pageSessionForms"||
                stackView.currentItem.objectName === "page1CDocument"){
            stackView.currentItem.clear();
            openPage(pageStart);
            footerRow.visible = false
        }
        if(stackView.currentItem.objectName === "pageMarkedItems"){
            if(tackView.currentItem.is_document_1c){
                stackView.currentItem.clear();
                openPage(pageStart);
            }
        }
    }


}

function barcodeInfoChanged() {
    if(wsSettings.priceCheckerMode)
        openPageScanner(false);
    else{
        if(stackView.currentItem.objectName === "pageStart")
            openPageScanner(false);
    }

    if(stackView.currentItem.objectName === "pageScanner"){
        pageScanner.setBarcodeInfo(wsBarcodeInfo);
        if(wsBarcodeInfo.isLongImgLoad)
            wsClient.get_image_data(wsBarcodeInfo);
    }else if(stackView.currentItem.objectName === "pageDocument"){
        //pageDocument.setBarcode(wsBarcodeInfo);
    }else if(stackView.currentItem.objectName === "page1CDocument"){
        //page1CDocument.setBarcode(wsBarcodeInfo);
    }
}


function leftArrow() {
    stackView.pop();
    mainToolBar.updateToolbar(stackView.currentItem.objectName, false);
    if(stackView.currentItem === pageDocument){
        //wsClient.getDocumentContent(context.pageDocument.ref)
    }else if(stackView.currentItem === pageSessionForms){
        wsClient.disconnectForm(page1CDocument.session_uuid, page1CDocument.form_uuid)
        page1CDocument.clear()
        footerRow.visible = false;
    }
}

function openDocumentsPage(){
    if(stackView.currentItem !== pageDocuments){
        var item = stackView.find(function(item) {
            return item.objectName === "pageDocuments";
        })
        if(item === null)
            stackView.push(pageDocuments);
        else{
            stackView.pop(item)
        }
    }else
        stackView.pop()

    mainToolBar.updateToolbar(stackView.currentItem.objectName, false);

    if(stackView.currentItem.objectName === "pageDocuments"){
        //wsClient.getDocuments();
    }
}

function updateWorkplaceView(org, suborg, stok, price){
    pageStart.organization = org;
    pageStart.subdivision = suborg;
    pageStart.warehouse = stok;
    pageStart.price = price;
    pageScanner.organization = org;
    pageScanner.subdivision = suborg;
    pageScanner.warehouse = stok;
    pageScanner.price = price;
}

