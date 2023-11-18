
function updateIcons(connectionState){
    if(connectionState){
       btnConnectionState.icon.source = "qrc:/img/lan_check_online.png"
    }else
       btnConnectionState.icon.source = "qrc:/img/lan_check_offline.png"

    btnSync.icon.source = connectionState ? "qrc:/img/cloud_sync.svg" : "qrc:/img/sync_problem.svg"
    btnSync.enabled = connectionState
}

function updateToolbarButton(objectName, isBlockDocCommand){

    if(objectName === "pageStart"){
        btnDocumentNew.visible = false
        btnArrowleft.visible = false
        btnFind.visible = false
        btnScan.visible = true
        btnDocuments.visible = !wsSettings.priceCheckerMode
        btnSync.visible = !wsSettings.priceCheckerMode
        if(wsSettings.keyboardInputMode)
            attachFocus.focus = true
    }else if(objectName === "pageScanner"){
        btnDocumentNew.visible = false;
        btnArrowleft.visible = true
        btnFind.visible = false
        btnDocuments.visible = !wsSettings.priceCheckerMode
        btnSync.visible = !wsSettings.priceCheckerMode
        btnScan.visible = true
        if(wsSettings.keyboardInputMode)
            attachFocus.focus = true
        if(btnDocuments.visible){
            btnDocuments.visible = !isBlockDocCommand;
        }
    }else if(objectName === "pageDocuments"){
        btnDocumentNew.visible = true;
        btnArrowleft.visible = true
        btnDocuments.visible = false
        btnSync.visible = true
        btnFind.visible = false
        btnScan.visible = false
        if(wsSettings.keyboardInputMode)
            attachFocus.focus = true
    }else if(objectName === "pageDocument"){
        btnDocumentNew.visible = false;
        btnArrowleft.visible = true
        btnDocuments.visible = false
        btnSync.visible = true
        btnFind.visible = true
        btnScan.visible = true
        if(wsSettings.keyboardInputMode)
            attachFocus.focus = true
    }else if(objectName === "pageMarkedItems"){
        btnDocumentNew.visible = false;
        btnArrowleft.visible = true
        btnDocuments.visible = false
        btnSync.visible = true
        btnFind.visible = true
        btnScan.visible = true
        if(wsSettings.keyboardInputMode)
            attachFocus.focus = true
    }else if(objectName === "pageSessions"){
        btnDocumentNew.visible = false;
        btnArrowleft.visible = true
        btnDocuments.visible = false
        btnSync.visible = true
        btnFind.visible = false
        btnScan.visible = false
        if(wsSettings.keyboardInputMode)
            attachFocus.focus = true
    }else if(objectName === "pageSessionForms"){
        btnArrowleft.visible = true
        btnDocuments.visible = false
        btnSync.visible = true
        btnFind.visible = false
        btnScan.visible = false
        if(wsSettings.keyboardInputMode)
            attachFocus.focus = true
    }else if(objectName === "page1CDocument"){
        btnDocumentNew.visible = false;
        btnArrowleft.visible = true
        btnDocuments.visible = false
        btnSync.visible = true
        btnFind.visible = true
        btnScan.visible = true
        if(wsSettings.keyboardInputMode)
            attachFocus.focus = true
    }

}
