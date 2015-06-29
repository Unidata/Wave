# ==================================================================================================================== #
# SkewTDialog.py                                                                                                       #
#                                                                                                                      #
# About: This file contains the SkewTDialog class for the skew-t options dialog box.                                   #
#                                                                                                                      #
# Copyright (c) 2015 Joshua Clark <joclark@ucar.edu> and Ryan May rmay@ucar.edu                                        #
#                                                                                                                      #
# LICENSE:                                                                                                             #
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated         #
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the  #
# rights to use,copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to       #
# permit persons to whom the Software is furnished to do so, subject to the following conditions:                      #
#                                                                                                                      #
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the #
# Software.                                                                                                            #
#                                                                                                                      #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE #
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS   #
# OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT, TORT OR   #
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.     #
#                                                                                                                      #
# Changelog:                                                                                                           #
#     Date          Engineer        Commit         Changes                                                             #
#     ----          --------        ------         -------                                                             #
#    29jun15        Clark                           Class creation, css generation, plot and get_values funcs          #
# ==================================================================================================================== #

from PyQt4 import QtGui, QtCore
from jc_dump.ErrorDialog import ErrorDialog


class SkewTDialog(QtGui.QDialog):

    def __init__(self):
        super(SkewTDialog, self).__init__()

        # Set window title, size, and position on the screen (center)
        self.setWindowTitle("Skew-T Menu")
        self.setGeometry(0, 0, 450, 300)
        self.move(QtGui.QApplication.desktop().screen().rect().center() - self.rect().center())

        # Load the stylesheet for this window and set to fill the window
        stylesheet = "css/SkewT.css"
        with open(stylesheet, "r") as ssh:
            self.setStyleSheet(ssh.read())
        self.setAutoFillBackground(True)
        self.setBackgroundRole(QtGui.QPalette.Highlight)

        # Create a grid to position all widgets on (labels, lineedit, combobox, etc)
        grid = QtGui.QGridLayout()
        grid.setSpacing(10)
        self.setLayout(grid)

        # Label and combobox for data source
        self.lbl = QtGui.QLabel("Data Source:", self)
        grid.addWidget(self.lbl, 0, 0, 1, 1)
        self.combo = QtGui.QComboBox(self)
        self.combo.addItem("NCSS")
        grid.addWidget(self.combo, 0, 1, 1, 1)

        # Label and lineedit for latitude
        self.lbl = QtGui.QLabel("Latitude:", self)
        grid.addWidget(self.lbl, 1, 0, 1, 1)
        self.latbox = QtGui.QLineEdit(self)
        grid.addWidget(self.latbox, 1, 1, 1, 1)

        # Label and lineedit for longitude
        self.lbl = QtGui.QLabel("Longitude:", self)
        grid.addWidget(self.lbl, 2, 0, 1, 1)
        self.longbox = QtGui.QLineEdit(self)
        grid.addWidget(self.longbox, 2, 1, 1, 1)

        # Submit button
        submit_button = QtGui.QPushButton('Plot')
        submit_button.clicked.connect(self.plot)
        grid.addWidget(submit_button, 3, 0, 1, 1, QtCore.Qt.AlignRight)

        # Cancel button
        cancel_button = QtGui.QPushButton('Cancel')
        cancel_button.clicked.connect(self.cancel)
        grid.addWidget(cancel_button, 3, 1, 1, 1, QtCore.Qt.AlignLeft)

        # Show the window!
        self.show()

    def plot(self):
        self.source = self.combo.currentText()
        self.lat = self.latbox.text()
        self.long = self.longbox.text()
        if not (self.lat and self.long):
            error = ErrorDialog('Latitude and longitude must be specified!')
            error.exec()
        else:
            self.accept()

    def cancel(self):
        self.reject()

    def get_values(self):
        return self.source, self.lat, self.long