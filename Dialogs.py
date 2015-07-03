# ==================================================================================================================== #
# MODULE NAME:                                                                                                         #
#   Dialogs.py                                                                                                         #
#                                                                                                                      #
# ABOUT:                                                                                                               #
#   This file contains the dialog box classes.                                                                         #
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

from PyQt4 import QtGui
from siphon.radarserver import RadarServer
import string


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

        # Label and combobox for data source
        self.datalbl = QtGui.QLabel("Data Source:", self)
        self.combo = QtGui.QComboBox(self)
        self.combo.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        self.combo.addItem("NCSS (Best/Current)")

        # Label and lineedit for latitude
        self.latlbl = QtGui.QLabel("Latitude:", self)
        self.latbox = QtGui.QLineEdit(self)
        validator = QtGui.QDoubleValidator(-90, 90, 0, parent=self.latbox)
        self.latbox.setValidator(validator)

        # Label and lineedit for longitude
        self.longlbl = QtGui.QLabel("Longitude:", self)
        self.longbox = QtGui.QLineEdit(self)
        validator = QtGui.QDoubleValidator(-180, 180, 0, parent=self.longbox)
        self.longbox.setValidator(validator)

        # Submit button
        submit_button = QtGui.QPushButton('Plot')
        submit_button.setMinimumSize(100, 25)
        submit_button.clicked.connect(self.set_values)

        # Cancel button
        cancel_button = QtGui.QPushButton('Cancel')
        cancel_button.setMinimumSize(100, 25)
        cancel_button.clicked.connect(self.cancel)

        hbox1 = QtGui.QHBoxLayout()
        hbox2 = QtGui.QHBoxLayout()
        hbox3 = QtGui.QHBoxLayout()
        hbox4 = QtGui.QHBoxLayout()

        hbox1.addWidget(self.datalbl)
        hbox1.addWidget(self.combo)
        hbox2.addWidget(self.latlbl)
        hbox2.addSpacing(22)
        hbox2.addWidget(self.latbox)
        hbox3.addWidget(self.longlbl)
        hbox3.addSpacing(13)
        hbox3.addWidget(self.longbox)
        hbox4.addStretch(1)
        hbox4.addWidget(submit_button)
        hbox4.addSpacing(10)
        hbox4.addWidget(cancel_button)
        hbox4.addStretch(1)

        vbox = QtGui.QVBoxLayout()
        vbox.addLayout(hbox1)
        vbox.addLayout(hbox2)
        vbox.addLayout(hbox3)
        vbox.addLayout(hbox4)


        self.setLayout(vbox)
        # Show the window!
        self.show()

    def set_values(self):
        r""" Sets the values for data source, latitude, and longitude from user selections. If latitude or longitude is
        not provided or rather, the value is invalid, an ErrorDialog object is given.

        Args:
            None.
        Returns:
            None.
        Raises:
            ErrorDialog messagebox if latitude and/or longitude is not provided.

        """

        self.source = self.combo.currentText()

        if self.latbox.hasAcceptableInput() and self.longbox.hasAcceptableInput():
            self.lat = self.latbox.text()
            self.long = self.longbox.text()
            self.accept()

        else:
            error = ErrorDialog('Invalid lat/long pair. Valid lat values (-90 to 90). Valid long values ' +
                                '(-180 to 180).')
            error.exec()

    def cancel(self):
        """ If the user cancels the dialog box, the dialog is destroyed and the main window becomes active."""

        self.reject()

    def get_values(self):
        return self.source, self.lat, self.long


class RadarDialog(QtGui.QDialog):
    r"""Creates an instance of the Radar Dialog box. Inherits from QDialog"""

    def __init__(self):
        super(RadarDialog, self).__init__()
        self.interface()

    def interface(self):
        r""" Contains the radar window interface generation functionality. Commented where needed."""

        # Set window title, size, and position on the screen (center)
        self.setWindowTitle("Radar Menu")
        self.setGeometry(0, 0, 450, 300)
        self.move(QtGui.QApplication.desktop().screen().rect().center() - self.rect().center())

        # Load the stylesheet for this window and set to fill the window
        stylesheet = "css/SkewT.css"
        with open(stylesheet, "r") as ssh:
            self.setStyleSheet(ssh.read())
        self.setAutoFillBackground(True)
        self.setBackgroundRole(QtGui.QPalette.Highlight)

        # Label and combobox for station ID
        self.statlbl = QtGui.QLabel("Select a station:", self)
        self.statcombo = QtGui.QComboBox(self)
        self.statcombo.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        rs = RadarServer('http://thredds.ucar.edu/thredds/radarServer/nexrad/level3/IDD/')
        l = [station + ': ' + rs.stations[station][4].title().replace('_', ' ').split('/')[0].strip() for station
             in rs.stations]
        l = sorted(l)
        [self.statcombo.addItem(station) for station in l]

        # Label and combobox for product ID
        self.productlbl = QtGui.QLabel("Select a product:", self)
        self.prodcombo = QtGui.QComboBox(self)
        self.prodcombo.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        self.prodcombo.addItem('N0Q')
        print(rs.metadata)

        # Submit button
        submit_button1 = QtGui.QPushButton('Get')
        submit_button1.setMinimumSize(100, 25)
        submit_button1.clicked.connect(self.set_radarvals)

        # Cancel button
        cancel_button = QtGui.QPushButton('Cancel')
        cancel_button.setMinimumSize(100, 25)
        cancel_button.clicked.connect(self.cancel)

        hbox1 = QtGui.QHBoxLayout()
        hbox2 = QtGui.QHBoxLayout()
        hbox3 = QtGui.QHBoxLayout()

        hbox1.addWidget(self.statlbl)
        hbox1.addWidget(self.statcombo)
        hbox2.addWidget(self.productlbl)
        hbox2.addSpacing(22)
        hbox2.addWidget(self.prodcombo)
        hbox3.addStretch(1)
        hbox3.addWidget(submit_button1)
        hbox3.addSpacing(10)
        hbox3.addWidget(cancel_button)
        hbox3.addStretch(1)

        vbox = QtGui.QVBoxLayout()
        vbox.addLayout(hbox1)
        vbox.addLayout(hbox2)
        vbox.addLayout(hbox3)

        self.setLayout(vbox)
        # Show the window!
        self.show()

    def set_radarvals(self):
        r""" Sets the values for data source, latitude, and longitude from user selections. If latitude or longitude is
        not provided or rather, the value is invalid, an ErrorDialog object is given.

        Args:
            None.
        Returns:
            None.
        Raises:
            ErrorDialog messagebox if latitude and/or longitude is not provided.

        """
        st = self.statcombo.currentText()
        self.station = st[:3]
        self.product = self.prodcombo.currentText()
        self.accept()


    def cancel(self):
        """ If the user cancels the dialog box, the dialog is destroyed and the main window becomes active."""

        self.reject()

    def get_radarvals(self):
        return self.station, self.product


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


