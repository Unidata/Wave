# ==================================================================================================================== #
# MODULE NAME:                                                                                                         #
# MainWindow.py                                                                                                      #
#                                                                                                                      #
# ABOUT:                                                                                                               #
#   This file contains the MainWindow class. This script also executes the GUI                                         #
#                                                                                                                      #
# COPYRIGHT (c)                                                                                                        #
#   2015 Joshua Clark <joclark@ucar.edu> and Ryan May rmay@ucar.edu                                                    #
#                                                                                                                      #
# LICENSE (MIT):                                                                                                       #
#                                                                                                                      #
#   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated       #
#   documentation files (the "Software"), to deal in the Software without restriction, including without limitation    #
#   the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and   #
#   to permit persons to whom the Software is furnished to do so, subject to the following conditions:                 #
#                                                                                                                      #
#   The above copyright notice and this permission notice shall be included in all copies or substantial portions of   #
#   the Software.                                                                                                      #
#                                                                                                                      #
#   The software is provided 'as is'. Without warranty of any kid, express or implied, including but not limited to    #
#   the warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the     #
#   authors or copyright holders be liable for any claim, damages, or other liability, whether in an action of         #
#   contract, tort or otherwise, arising from, out of or in connection with the software or the use or other dealings  #
#   in the software.                                                                                                   #
#                                                                                                                      #
# ==================================================================================================================== #

import sys
from PyQt4 import QtGui, QtCore
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
import numpy as np
from metpy.calc import get_wind_components, lcl, dry_lapse, parcel_profile
from metpy.plots import SkewT
from metpy.units import units, concatenate
from Dialogs import SkewTDialog, RadarDialog
from DataAccessor import DataAccessor
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QT
import matplotlib.backends.backend_qt5 as override


class MplToolbar(NavigationToolbar2QT):
    r""" A subclassed toolbar of the matplotlib navigation bar to remove buttons not relevant to the GUI"""

    def __init__(self, canvas_, parent_):
        override.figureoptions = None  # Monkey patched to kill the figure options button on matplotlib toolbar

        self.toolitems = (
            ('Home', 'Reset original view', 'home', 'home'),
            ('Save', 'Save the current image', 'filesave', 'save_figure'),
            (None, None, None, None),
            ('Back', 'Back to previous view', 'back', 'back'),
            ('Forward', 'Forward to next view', 'forward', 'forward'),
            (None, None, None, None),
            ('Pan', 'Pan axes with left mouse, zoom with right', 'move', 'pan'),
            ('Zoom', 'Zoom to rectangle', 'zoom_to_rect', 'zoom'),
        )
        NavigationToolbar2QT.__init__(self, canvas_, parent_)


class Window(QtGui.QMainWindow):
    r""" A mainwindow object for the GUI display. Inherits from QMainWindow."""

    def __init__(self):
        super(Window, self).__init__()
        self.interface()

    def interface(self):
        r""" Contains the main window interface generation functionality. Commented where needed."""

        # Get the screen width and height and set the main window to that size
        screen = QtGui.QDesktopWidget().screenGeometry()
        self.setGeometry(0, 0, 800, screen.height())
        self.setMaximumSize(QtCore.QSize(800, 2000))

        # Set the window title and icon
        self.setWindowTitle("WAVE: Weather Analysis and Visualization Environment")
        self.setWindowIcon(QtGui.QIcon('./img/wave_64px.png'))

        # Import the stylesheet for this window and set it to the window
        stylesheet = "css/MainWindow.css"
        with open(stylesheet, "r") as ssh:
            self.setStyleSheet(ssh.read())
        self.setAutoFillBackground(True)
        self.setBackgroundRole(QtGui.QPalette.Highlight)

        # Create actions for menus and toolbar
        exit_action = QtGui.QAction(QtGui.QIcon('./img/exit_64px.png'), 'Exit', self)
        exit_action.setShortcut('Ctrl+Q')
        exit_action.setStatusTip('Exit application')
        exit_action.triggered.connect(self.close)
        clear_action = QtGui.QAction(QtGui.QIcon('./img/clear_64px.png'), 'Clear the display', self)
        clear_action.setShortcut('Ctrl+C')
        clear_action.setStatusTip('Clear the display')
        clear_action.triggered.connect(self.clear_canvas)
        skewt_action = QtGui.QAction(QtGui.QIcon('./img/skewt_64px.png'), 'Open the skew-T dialog', self)
        skewt_action.setShortcut('Ctrl+S')
        skewt_action.setStatusTip('Open the skew-T dialog')
        skewt_action.triggered.connect(self.skewt_dialog)
        radar_action = QtGui.QAction(QtGui.QIcon('./img/radar_64px.png'), 'Radar', self)
        radar_action.setShortcut('Ctrl+R')
        radar_action.setStatusTip('Open Radar Dialog Box')
        radar_action.triggered.connect(self.radar_dialog)

        # Create the top menubar, setting native to false (for OS) and add actions to the menus
        menubar = self.menuBar()
        menubar.setNativeMenuBar(False)
        filemenu = menubar.addMenu('&File')
        editmenu = menubar.addMenu('&Edit')
        helpmenu = menubar.addMenu('&Help')
        filemenu.addAction(exit_action)

        # Create the toolbar, place it on the left of the GUI and add actions to toolbar
        left_tb = QtGui.QToolBar()
        self.addToolBar(QtCore.Qt.LeftToolBarArea, left_tb)
        left_tb.setMovable(False)
        left_tb.addAction(clear_action)
        left_tb.addAction(skewt_action)
        left_tb.addAction(radar_action)
        self.setIconSize(QtCore.QSize(30, 30))

        # Create the toolbar, place it on the left of the GUI and add actions to toolbar
        right_tb = QtGui.QToolBar()
        self.addToolBar(QtCore.Qt.RightToolBarArea, right_tb)
        right_tb.setMovable(False)
        right_tb.addAction(clear_action)
        right_tb.addAction(skewt_action)
        right_tb.addAction(radar_action)

        # Create the status bar with a default display
        self.statusBar().showMessage('Ready')

        # Figure and canvas widgets that display the figure in the GUI
        self.figure = plt.figure(facecolor='#2B2B2B')
        self.canvas = FigureCanvas(self.figure)

        # Add subclassed matplotlib navbar to GUI
        # spacer widgets for left and right of buttons
        left_spacer = QtGui.QWidget()
        left_spacer.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        right_spacer = QtGui.QWidget()
        right_spacer.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.mpltb = QtGui.QToolBar()
        self.mpltb.addWidget(left_spacer)
        self.mpltb.addWidget(MplToolbar(self.canvas, self))
        self.mpltb.addWidget(right_spacer)
        self.mpltb.setMovable(False)
        self.addToolBar(QtCore.Qt.TopToolBarArea, self.mpltb)

        # Set the figure as the central widget and show the GUI
        self.setCentralWidget(self.canvas)
        self.show()

    def skewt_dialog(self):
        r""" When the toolbar icon for the Skew-T dialog is clicked, this function is executed. Creates an instance of
        the SkewTDialog object which is the dialog box. If the submit button on the dialog is clicked, get the user
        inputted values and pass them into the sounding retrieval call (DataAccessor.get_sounding) to fetch the data.
        Finally, plot the returned data via self.plot.

        Args:
            None.
        Returns:
            None.
        Raises:
            None.

        """

        dialog = SkewTDialog()
        if dialog.exec_():
            source, lat, long = dialog.get_values()
            t, td, p, u, v, lat, long, time = DataAccessor.get_sounding(source, lat, long)
            self.plot(t, td, p, u, v, lat, long, time)

    def plot(self, t, td, p, u, v, lat, long, time):
        r"""Displays the Skew-T data on a matplotlib figure.

        Args:
            t (array-like): A list of temperature values.
            td (array-like): A list of dewpoint values.
            p (array-like): A list of pressure values.
            u (array-like): A list of u-wind component values.
            v (array-like): A list of v-wind component values.
            lat (string): A string containing the requested latitude value.
            long (string): A string containing the requested longitude value.
            time (string): A string containing the UTC time requested with seconds truncated.
        Returns:
            None.
        Raises:
            None.

        """

        # Create a new figure. The dimensions here give a good aspect ratio
        self.skew = SkewT(self.figure, rotation=40)

        # Plot the data using normal plotting functions, in this case using
        # log scaling in Y, as dictated by the typical meteorological plot
        self.skew.plot(p, t, 'r')
        self.skew.plot(p, td, 'g')
        self.skew.plot_barbs(p, u, v, barbcolor='#FF0000', flagcolor='#FF0000')
        self.skew.ax.set_ylim(1000, 100)
        self.skew.ax.set_xlim(-40, 60)

        # Axis colors
        self.skew.ax.tick_params(axis='x', colors='#A3A3A4')
        self.skew.ax.tick_params(axis='y', colors='#A3A3A4')

        # Calculate LCL height and plot as black dot
        l = lcl(p[0], t[0], td[0])
        lcl_temp = dry_lapse(concatenate((p[0], l)), t[0])[-1].to('degC')
        self.skew.plot(l, lcl_temp, 'ko', markerfacecolor='black')

        # Calculate full parcel profile and add to plot as black line
        prof = parcel_profile(p, t[0], td[0]).to('degC')
        self.skew.plot(p, prof, 'k', linewidth=2)

        # Color shade areas between profiles
        self.skew.ax.fill_betweenx(p, t, prof, where=t >= prof, facecolor='#5D8C53', alpha=0.7)
        self.skew.ax.fill_betweenx(p, t, prof, where=t < prof, facecolor='#CD6659', alpha=0.7)

        # Add the relevant special lines
        self.skew.plot_dry_adiabats()
        self.skew.plot_moist_adiabats()
        self.skew.plot_mixing_lines()

        # Set title
        deg = u'\N{DEGREE SIGN}'
        self.skew.ax.set_title('Sounding for ' + lat + deg + ', ' + long + deg + ' at ' + time + 'z', y=1.02,
                               color='#A3A3A4')

        # Discards old graph, works poorly though
        # skew.ax.hold(False)
        # Figure and canvas widgets that display the figure in the GUI

        # set canvas size to display Skew-T appropriately
        self.canvas.setMaximumSize(QtCore.QSize(800, 2000))
        # refresh canvas
        self.canvas.draw()

    def radar_dialog(self):
        r""" When the toolbar icon for the Skew-T dialog is clicked, this function is executed. Creates an instance of
        the SkewTDialog object which is the dialog box. If the submit button on the dialog is clicked, get the user
        inputted values and pass them into the sounding retrieval call (DataAccessor.get_sounding) to fetch the data.
        Finally, plot the returned data via self.plot.

        Args:
            None.
        Returns:
            None.
        Raises:
            None.

        """

        radar_dialog = RadarDialog()

        if radar_dialog.exec_():
            station, product = radar_dialog.get_radarvals()
            x, y, ref = DataAccessor.get_radar(station, product)
            self.plot_radar(x, y, ref)

    def plot_radar(self, x, y, ref):
        r"""Displays the Skew-T data on a matplotlib figure.

        Args:
            t (array-like): A list of temperature values.
            td (array-like): A list of dewpoint values.
            p (array-like): A list of pressure values.
            u (array-like): A list of u-wind component values.
            v (array-like): A list of v-wind component values.
            lat (string): A string containing the requested latitude value.
            long (string): A string containing the requested longitude value.
            time (string): A string containing the UTC time requested with seconds truncated.
        Returns:
            None.
        Raises:
            None.

        """

        self.ax = self.figure.add_subplot(111)
        self.ax.pcolormesh(x, y, ref)
        self.ax.set_aspect('equal', 'datalim')
        self.ax.set_xlim(-460, 460)
        self.ax.set_ylim(-460, 460)
        self.ax.tick_params(axis='x', colors='#A3A3A4')
        self.ax.tick_params(axis='y', colors='#A3A3A4')

        # set canvas size to display Skew-T appropriately
        self.canvas.setMaximumSize(QtCore.QSize(800, 2000))
        # refresh canvas
        self.canvas.draw()

    def clear_canvas(self):
        self.canvas.close()
        self.figure = plt.figure(facecolor='#2B2B2B')
        self.canvas = FigureCanvas(self.figure)
        self.setCentralWidget(self.canvas)


def main():
    app = QtGui.QApplication(sys.argv)
    ex = Window()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()