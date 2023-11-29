import QtQuick 2.15
import QtQuick.Controls 2.15
import NotificationQueue 1.0

Item{
    id: control

    property NotificationQueue queue: NotificationQueue{

    }

    function show(message){
        toolTip.visible = true
        var text = qsTr("<table width=\"100%\" cols=2 align=\"center\">
                         <tr><td width=\"20\" align=\"center\"><img src='qrc:/img/info16.png'/></td><td style=\"text-align:center\">" + message + "</td></tr></table>")
        toolTip.show(text)
    }
    function showError(what, err){
        toolTip.visible = true
        var text = qsTr("<table width=\"100%\ cols=2  align=\"center\>
                         <tr><td width=\"20\" align=\"center\"><img src='qrc:/img/error16.png'/></td><td style=\"text-align:center\"> " + what + ": " + err + "</td></tr></table>")
        toolTip.show(text)
    }

    function addNotify(type_, message_){
        let msg = {
            type: type_,
            message: message_
        }

        queue.add(JSON.stringify(msg))
    }

    function showNotify(){
        if(!toolTip.visible && toolTip.isStarted)
            toolTip.isStarted = false
        if(!toolTip.isStarted)
            toolTip.next()
    }

    Row{
        ToolTip {
            property bool isStarted: false

            function next(){
                if(!control.queue.isEmpty()){
                    let msg = JSON.parse(queue.get())
                    if(msg.type === "message")
                        control.show(msg.message)
                    else if(msg.type === "error")
                        control.showError("Ошибка", msg.message)
                    toolTip.isStarted = true
                }else
                    isStarted = false
            }

            id: toolTip
            parent: parent
            visible: false
            delay: 1000
            timeout: 2000
            anchors.centerIn: parent
            verticalPadding: 10
            onAboutToHide: {
                //console.debug("onAboutToHide")
                //toolTip.next()
            }
            onAboutToShow: {
                toolTip.isStarted = true
            }
            onTimeoutChanged: {
                //toolTip.next()
            }
            onDelayChanged: {
                //toolTip.next()
            }
            onVisibleChanged:{
                console.debug("onVisibleChanged " + toolTip.visible)
                if(!toolTip.visible)
                    toolTip.next()
            }
        }
    }
}


