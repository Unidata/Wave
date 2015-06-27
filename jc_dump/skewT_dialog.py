from PyQt4 import QtGui
# from siphon.catalog import TDSCatalog


class SkewTDialog(QtGui.QDialog):
    def __init__(self, parent=None):
        super(SkewTDialog, self).__init__()

        self.setWindowTitle("Skew-T Menu")
        self.setGeometry(0, 0, 450, 300)
        self.move(QtGui.QApplication.desktop().screen().rect().center() - self.rect().center())


        stylesheet = "css/SkewT.css"
        with open(stylesheet, "r") as ssh:
            self.setStyleSheet(ssh.read())
        self.setAutoFillBackground(True)
        self.setBackgroundRole(QtGui.QPalette.Highlight)

        grid = QtGui.QGridLayout()
        grid.setSpacing(10)
        self.setLayout(grid)

        self.lbl = QtGui.QLabel("Data Source:", self)
        grid.addWidget(self.lbl, 0, 0, 1, 1)
        combo = QtGui.QComboBox(self)
        combo.addItem("NCSS")
        combo.activated[str].connect(self.combo_data)
        grid.addWidget(combo, 0, 1, 1, 1)

        self.lbl = QtGui.QLabel("Latitude:", self)
        grid.addWidget(self.lbl, 1, 0, 1, 1)
        latbox = QtGui.QLineEdit(self)
        grid.addWidget(latbox, 1, 1, 1, 1)

        self.lbl = QtGui.QLabel("Longitude:", self)
        grid.addWidget(self.lbl, 2, 0, 1, 1)
        longbox = QtGui.QLineEdit(self)
        grid.addWidget(longbox, 2, 1, 1, 1)

        submit_button = QtGui.QPushButton('Plot')
        submit_button.clicked.connect(self.window_data)
        grid.addWidget(submit_button, 3, 0, 1, 3)

        self.show()

    def combo_data(self, text):
        return text

    def window_data(self):
        pass
