import sys
from PyQt4 import QtGui, Qt, QtCore
from jc_dump.skewT_dialog import skewt_dialog
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
import matplotlib.pyplot as plt


class Window(QtGui.QMainWindow):

    def __init__(self):
        super(Window, self).__init__()
        self.interface()

    def interface(self):
        screen = QtGui.QDesktopWidget().screenGeometry()
        self.setGeometry(0, 0, screen.width(), screen.height())

        css = """
                QWidget{
                    Background: #4c4c4c;
                    color:white;
                    font:14px bold;
                    font-weight:bold;
                    border-radius: 1px;
                }

                QMenuBar::item {
                    spacing: 3px; /* spacing between menu bar items */
                    padding: 10px 4px 4px 4px;
                    background: transparent;
                    font:16px bold;
                }

                QMenuBar::item:selected { /* when selected using mouse or keyboard */
                    background: #666666;
                }

                QMenu::item:selected {
                    background: #666666;
                }

                QToolButton{
                    Background: transparent;
                }
                QToolButton:hover{
                    Background: #666666;
                }
                """

        self.setStyleSheet(css)
        self.setAutoFillBackground(True)
        self.setBackgroundRole(QtGui.QPalette.Highlight)

        exitaction = QtGui.QAction(QtGui.QIcon('./img/exit_64px.png'), 'Exit', self)
        exitaction.setShortcut('Ctrl+Q')
        exitaction.setStatusTip('Exit application')
        exitaction.triggered.connect(self.close)
        skewtaction = QtGui.QAction(QtGui.QIcon('./img/skewt_64px.png'), 'Skew-T', self)
        skewtaction.setShortcut('Ctrl+S')
        skewtaction.setStatusTip('Open Skew-T Dialog Box')
        skewtaction.triggered.connect(self.skewt_dialog)

        menubar = self.menuBar()
        menubar.setNativeMenuBar(False)
        filemenu = menubar.addMenu('&File')
        editmenu = menubar.addMenu('&Edit')
        filemenu.addAction(exitaction)

        toolbar = self.addToolBar('Exit')
        toolbar.addAction(exitaction)
        toolbar.addAction(skewtaction)

        self.statusBar().showMessage('Ready')
        self.setWindowTitle("WAVE (Skew-T Viewer)")
        self.setWindowIcon(QtGui.QIcon('./img/wave_64px.png'))

        # this is the Canvas Widget that displays the `figure`
        # it takes the `figure` instance as a parameter to __init__
        self.figure = plt.figure()
        self.canvas = FigureCanvas(self.figure)
        self.setCentralWidget(self.canvas)
        self.show()

    def skewt_dialog(self):
        self.dialog = skewt_dialog()
        self.dialog.exec_()


def main():

    app = QtGui.QApplication(sys.argv)
    ex = Window()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()