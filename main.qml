import QtQuick
import QtQuick.Window
import Requester 1.0

Window {
    id: root
    width: 640
    height: 480
    visible: true
    title: qsTr("TextBrowser")

    RequesterHelper {
        id: requester
        onPayloadChanged: {
            textResponse.text = requester.payload
        }
        onSuccessChanged: {
            if (requester.success) {
                textResponse.color = "#000000"
            } else {
                textResponse.color = "#FF0000"
            }
        }

        onRequestFinished: {
            console.log("Request finished")
            mouseAreaButton.enabled = true
        }
    }

    Column {
        id: row
        anchors.fill: parent
        property int marginSize: 3
        spacing: marginSize
        anchors.margins: 2 * row.marginSize
        Row {
            id: column
            spacing: row.marginSize
            width: parent.width
            height: parent.height / 10
            Rectangle {
                id: rectangleUrl
                width: parent.width - (parent.width / 9)
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                radius: 2 * row.marginSize
                border.width: row.marginSize

                TextInput {
                    id: textInputUrl
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: parent.height / 3
                    anchors.margins: 2 * row.marginSize

                    inputMethodHints: Qt.ImhUrlCharactersOnly
                    Component.onCompleted: {
                        textInputUrl.text = "Enter url here"
                    }

                    onTextEdited: {
                        if (text.length === 0) {
                            mouseAreaButton.enabled = false
                        } else {
                            if (!requester.active) {
                                mouseAreaButton.enabled = true
                            }
                        }

                        requester.url = text
                    }
                }
            }

            Rectangle {
                id: rectangleButton
                width: parent.width - x
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                radius: 2 * row.marginSize
                border.width: row.marginSize

                MouseArea {

                    id: mouseAreaButton
                    anchors.fill: parent
                    anchors.margins: row.marginSize
                    onClicked: {
                        requester.request()
                        enabled = false
                        anim.start()
                    }
                    Text {
                        id: textButton
                        anchors.fill: parent
                        anchors.centerIn: parent.Center
                        anchors.margins: 2 * row.marginSize
                        text: qsTr("Load")
                        font.pixelSize: parent.height / 3
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        fontSizeMode: Text.Fit
                    }
                }

                SequentialAnimation {
                    id: anim

                    // Expand the button
                    PropertyAnimation {
                        target: rectangleButton
                        property: "scale"
                        to: 1.1
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }

                    // Shrink back to normal
                    PropertyAnimation {
                        target: rectangleButton
                        property: "scale"
                        to: 1.0
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }

        Rectangle {
            id: rectangleResponse

            color: "#ffffff"
            anchors.right: parent.right
            anchors.left: parent.left
            height: parent.height - y
            radius: 2 * row.marginSize
            border.width: row.marginSize

            Text {
                id: textResponse
                anchors.fill: parent
                anchors.margins: 2 * row.marginSize

                wrapMode: Text.WrapAnywhere
                text: qsTr("Get request")
                textFormat: Text.PlainText
                font.pixelSize: 12
            }
        }
    }
}
