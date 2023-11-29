function setBarcode(bInfo){
    txtBarcode.text = bInfo.barcode;
    txtName.text = bInfo.synonym;
    txtPrice.text = bInfo.price + " " + bInfo.currency;
    txtStockBalance.text = bInfo.balance + " " + bInfo.unit;
    if(!bInfo.isLongImgLoad){
        if(wsSettings.showImage){
            imageData.source = bInfo.imageSource;
            longOperation.visible = false
        }else
            imageData.visible = false
    }else{
        imageData.visible = false
        longOperation.visible = true
    }

    pane.visible = true;
}

function changeImageSource(bInfo){
    longOperation.visible = false
    imageData.source = bInfo.imageSource;
    imageData.visible = bInfo.imageSource !== ""
    rowImage.visible = bInfo.imageSource !== "";
}

function setPaneVisible(value){
    pane.visible = value;
}
