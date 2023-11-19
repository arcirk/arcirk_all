function setModelSource(value){
    wsDocumentTable.jsonText = value;
}

function setBarcode(bInfo){
    wsClient.documentContentUpdate(bInfo, wsDocumentTable, rowObject.ref)
    pageDoc.currentBarcode = bInfo.barcode;
}


function setDcumentContentModel(value){
    wsDocumentTable.jsonText = value;
    wsDocumentTable.reset();
}
