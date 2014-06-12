import QtQuick 2.3

Text {
    text: "Placeholder"
    color: "white"
//    transform: viewProjection.screenMatrix
//    transform: Translate {
//        x: 50
//        y: 50
//    }
    transform: Matrix4x4 {
        matrix: Qt.matrix4x4(0.9, 0, 0, 180,
                             0, 0.9, 0, 126,
                             0, 0, -10, 0,
                             0, 0, 0, 1)
    }

    Connections {
        target: viewProjection
        onScreenMatrixChanged: console.log("New projection" + transform)
    }
}
