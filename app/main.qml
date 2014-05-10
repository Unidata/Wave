import QtQuick 2.0

Item {
    id: root
    visible: true
    Rectangle {
        id: obj
        width: 110
        height: 50
        color: "blue"
        radius: 14
        border.color: "#000000"
        opacity: 0.4

        Text {
            id: label
            text: qsTr("This is QML text!")
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 12
        }

        MouseArea {
            id: dragger
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.XAndYAxis
        }
    }
}
