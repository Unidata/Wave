import QtQuick 2.3

Item {
    id: container
    transform: null

    Connections {
        target: viewProjection
        onScreenMatrixChanged: {
            var obj = mapToItem(null, 0, 0);
            console.log("New projection " + obj.x + " " + obj.y);
        }
    }

    Text {
        transform: Matrix4x4 {
            matrix: viewProjection.screenMatrix
        }
        id:label
        text: "Placeholder"
        color: "white"
    }

    Rectangle {
        width: 10
        height: 10
        color: "blue"
    }
}
