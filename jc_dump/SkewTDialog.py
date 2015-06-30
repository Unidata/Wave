# ==================================================================================================================== #
# MODULE NAME:                                                                                                         #
#   SkewTDialog.py                                                                                                     #
#                                                                                                                      #
# ABOUT:                                                                                                               #
#   This file contains the SkewTDialog class for the skew-t options dialog box.                                        #
#                                                                                                                      #
# COPYRIGHT (c)                                                                                                        #
#   2015 Joshua Clark <joclark@ucar.edu> and Ryan May rmay@ucar.edu                                                    #
#                                                                                                                      #
# LICENSE (MIT):                                                                                                       #
#   See MainWindow.py for full license information.                                                                    #
#                                                                                                                      #
# CHANGELOG:                                                                                                           #
#                                                                                                                      #
#     Date          Engineer        Commit         Changes                                                             #
#     ----          --------        ------         -------                                                             #
#    29jun15        Clark           c916584        Class creation, css generation, plot and get_values funcs           #
#                                                                                                                      #
# ==================================================================================================================== #

from PyQt4 import QtGui, QtCore
from jc_dump.ErrorDialog import ErrorDialog


class SkewTDialog(QtGui.QDialog):
    r"""Creates an instance of the Skew-T Dialog box. Inherits from QDialog"""

    def __init__(self):
        super(SkewTDialog, self).__init__()
        self.interface()

    def interface(self):
        r""" Contains the Skew-t window interface generation functionality. Commented where needed."""

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

    def set_values(self):
        r""" Sets the values for data source, latitude, and longitude from user selections. If latitude or longitude is
        not provided, an ErrorDialog object is given.

        Args:
            None.
        Returns:
            None.
        Raises:
            ErrorDialog messagebox if latitude and/or longitude is not provided.

        """

        self.source = self.combo.currentText()
        self.lat = self.latbox.text()
        self.long = self.longbox.text()

        if not (self.lat and self.long):
            error = ErrorDialog('Latitude and longitude must be specified!')
            error.exec()
        else:
            self.accept()

    def cancel(self):
        """ If the user cancels the dialog box, the dialog is destroyed and the main window becomes active."""

        self.reject()

    def get_values(self):
        return self.source, self.lat, self.long