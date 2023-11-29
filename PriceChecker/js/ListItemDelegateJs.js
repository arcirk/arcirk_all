function getSynonim(){

    if(rowObject === undefined)
        return "";

    if(objectName === "documents"){
        return "" + rowObject.second + "  " + rowObject.number + " от " + wsClient.documentDate(rowObject.date);
    }else if(control.objectName === "document_table"){
        return rowObject.barcode //control.modelIndex.barcode //representation;
    }else if(control.objectName === "document_table_marked"){
        return wsClient.parseQr(rowObject.qr_code); //control.modelIndex.qr_code
    }else if(control.currentTable === "sessions_table"){
        let info_base = control.modelIndex.info_base;
        if(info_base === undefined || info_base === ""){
            return control.modelIndex.user_name;
        }else{
            return control.modelIndex.user_name + " (" + info_base.trim() + ")";
        }
    }else if(control.currentTable === "session_forms_table"){
        if(control.modelIndex.presentation !== "")
            return control.modelIndex.presentation;
        else
            return control.modelIndex.caption;
    }else if(control.currentTable === "1c_document_table"){
        return control.modelIndex.barcode;
    }
}

function getComment(){
    if(rowObject === undefined)
        return "";
    if(objectName === "1c_document_table"){
        let val = rowObject.nomenclature;
        return val !== undefined ? val : ""
    }else if(objectName === "document_table"){
        let val = rowObject.good;
        return val !== undefined ? val : ""
    }else if(objectName === "session_forms_table"){
        return rowObject.responsible
    }else{
        try{
            let cache = rowObject.cache
            let cm = cache.trim()
            if(cm !== undefined && cm !== ""){
                try{
                    let obj = JSON.parse(cm)
                    let val = obj['comment']
                    return val !== undefined ? val.trim() : ""
                }catch(e){
                    return ""
                }

            }else
                return ""
        }catch(error){
            return "";
        }
    }
}

function getSourceComment(){
    if(rowObject === undefined)
        return "";
    let cm = rowObject.cache
    if(cm !== undefined && cm !== ""){
        let obj = JSON.parse(cm)
        return obj.trim()
    }else
        return ""
}

function setSelectedBackground(){
    if(control.checked && !control.chldrenList){
        if(!control.isSelected){
            if(Theme === "Dark")
                control.Material.background  = "#424242"
            else
                control.Material.background  = "#efebe9"
        }else{
            control.Material.background  = "#424242"
        }
    }else
        if(!control.isSelected)
            control.Material.background  = undefined
        else
            control.Material.background  = "#424242"
}

function item_is_marked(){
    if(rowObject === undefined)
        return false;
    if(control.objectName === "document_table"){
        let is_marked = Number(rowObject.is_marked)
        return is_marked !== undefined ? is_marked === 1 : false
    }
    return false;
}

function quantityText(){
    if(rowObject === undefined)
        return "";
    if(control.objectName === "document_table"){
        let quant = Number(rowObject.quantity)
        let unit = rowObject.unit
        let is_marked = Number(rowObject.is_marked)
        let l = control.quantityVisible ? "" + quant + " " + unit : ""
        if(is_marked === 1){
            let marked_text = "<html><head></head><body>"
            let q = Number(rowObject.marked_quantity)
            if(q === undefined)
                q = 0

            l = l + " / "
            if(q !== quant)
                l = l + "<font color=\"#ff8080\">" + q + " шт.</font>"
            else
                l = l + "<font color=\"#66a334\">" + q + " шт.</font>"

            return marked_text + l +"</body></html>"
        }else
            return l
    }else
        return ""
}

function getCurrentObject(){
    let obj = JSON.parse(parentModel.dump(model.row))
    return obj;
}
