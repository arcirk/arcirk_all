function new_number(){
    let m = wsDocuments.max("_id");
    if(m > 0)
        return wsClient.documentGenerateNewNumber(m + 1);
    else
        return wsClient.documentGenerateNewNumber(1);
}

function setDcumentsModel(value){
    wsDocuments.jsonText = value;
    wsDocuments.reset();
    console.log("setDcumentsModel")
}
