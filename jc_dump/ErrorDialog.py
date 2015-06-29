from PyQt4 import QtGui


class ErrorDialog(QtGui.QMessageBox):
    def __init__(self, message_text):
        super(ErrorDialog, self).__init__()
        self.setIcon(QtGui.QMessageBox.Critical)
        self.ok = self.addButton(QtGui.QMessageBox.Ok)
        self.setText(message_text)
        # need to figure out why this QDialog code doesn't work...
        self.setGeometry(300, 300, 250, 150)
        self.move(QtGui.QApplication.desktop().screen().rect().center() - self.rect().center())
        self.setWindowTitle('Error')
