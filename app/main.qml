import QtQuick 2.0

Item {
    id: root
    visible: true
    MouseArea {
        id: mover
        anchors.fill: parent
        onPositionChanged: {
            screenPoint.x = mouseX + offsetX;
            screenPoint.y = mouseY - (screenPoint.height + offsetY);
        }

        hoverEnabled: true
        propagateComposedEvents: true
        acceptedButtons: Qt.NoButton
        property int offsetX: 10
        property int offsetY: 10
    }

    Rectangle {
        id: screenPoint
        width: 125
        height: 35
        color: "black"
        radius: 14
        border.color: "#000000"
        opacity: 0.6

        function updateText()
        {
            var pt = viewProjection.transScreenToProj(x, y);
            label.text = "(" + x + ", " + y +
                    ")\n(" + pt.x.toFixed(2) + ", " + pt.y.toFixed(2) + ")";
        }

        Connections {
            target: viewProjection
            onScreenMatrixChanged: screenPoint.updateText()
        }

        onXChanged: updateText()
        onYChanged: updateText()

        Text {
            id: label
            color: "white"
            text: "test"
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 12
        }

//        MouseArea {
//            id: dragger
//            anchors.fill: parent
//            drag.target: parent
//            drag.axis: Drag.XAndYAxis
//        }
    }
}
