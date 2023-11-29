function clear(){
    wsDocumentTableMarked.clear(true);
    session_uuid = "";
    form_uuid = "";
}

function setFilterBarcode(value){
    txtFilter.text = value
}

function setModelSource(value){
    wsDocumentTableMarked.jsonText = value;
}
function setBarcode(qrCode){

    let count = wsDocumentTableMarked.rowCount();
    if(count === parentQuantity){
        popupMessage.addNotify("error", "Необходимое количество Qr кодов уже подобрано.")
        popupMessage.showNotify()
        return
    }
    if(!is_document_1c)
        wsClient.setQrCode(qrCode, docRef, itemRef, wsDocumentTableMarked);
    else{
        wsClient.setQrCode1CDocument(pageMarkedItems.session_uuid,
                                     pageMarkedItems.form_uuid,
                                     qrCode,
                                     pageMarkedItems.itemRef,
                                     pageMarkedItems.currentBarcode);
    }
}

function insertQrCode(jsonModel){
    wsDocumentTableMarked.insertRow(0, jsonModel)
}

function setDcumentContentModel(value){
    wsDocumentTableMarked.jsonText = value;
    wsDocumentTableMarked.reset();
}
