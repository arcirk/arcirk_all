function new_number(){
    let m = wsClient.dataMax();
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
