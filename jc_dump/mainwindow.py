import sys
from PyQt4 import QtGui, Qt, QtCore
from jc_dump.skewT_dialog import SkewTDialog
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
import numpy as np
from metpy.calc import get_wind_components, lcl, dry_lapse, parcel_profile
from metpy.plots import SkewT
from metpy.units import units, concatenate


class Window(QtGui.QMainWindow):

    def __init__(self):
        super(Window, self).__init__()
        self.interface()

    def interface(self):
        screen = QtGui.QDesktopWidget().screenGeometry()
        self.setGeometry(0, 0, screen.width(), screen.height())

        stylesheet = "css/MainWindow.css"
        with open(stylesheet, "r") as ssh:
            self.setStyleSheet(ssh.read())
        self.setAutoFillBackground(True)
        self.setBackgroundRole(QtGui.QPalette.Highlight)

        exit_action = QtGui.QAction(QtGui.QIcon('./img/exit_64px.png'), 'Exit', self)
        exit_action.setShortcut('Ctrl+Q')
        exit_action.setStatusTip('Exit application')
        exit_action.triggered.connect(self.close)
        skewt_action = QtGui.QAction(QtGui.QIcon('./img/skewt_64px.png'), 'Skew-T', self)
        skewt_action.setShortcut('Ctrl+S')
        skewt_action.setStatusTip('Open Skew-T Dialog Box')
        skewt_action.triggered.connect(self.skewt_dialog)

        menubar = self.menuBar()
        menubar.setNativeMenuBar(False)
        filemenu = menubar.addMenu('&File')
        editmenu = menubar.addMenu('&Edit')
        filemenu.addAction(exit_action)

        toolbar = self.addToolBar('Exit')
        toolbar.addAction(exit_action)
        toolbar.addAction(skewt_action)

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
        self.dialog = SkewTDialog()
        self.dialog.exec_()

    def plot(self):

        p, T, Td, direc, spd = np.loadtxt(open('may3_sounding.txt'),
                usecols=(0, 2, 3, 6, 7), unpack=True)
        p = p * units.mbar
        T = T * units.degC
        Td = Td * units.degC
        spd = spd * units.knot
        direc = direc * units.deg

        u, v = get_wind_components(spd, direc)
        # Create a new figure. The dimensions here give a good aspect ratio
        skew = SkewT(self.figure, rotation=45)

        # Plot the data using normal plotting functions, in this case using
        # log scaling in Y, as dictated by the typical meteorological plot
        skew.plot(p, T, 'r')
        skew.plot(p, Td, 'g')
        skew.plot_barbs(p, u, v)
        skew.ax.set_ylim(1000, 100)
        skew.ax.set_xlim(-40, 60)

        # Calculate LCL height and plot as black dot
        l = lcl(p[0], T[0], Td[0])
        lcl_temp = dry_lapse(concatenate((p[0], l)), T[0])[-1].to('degC')
        skew.plot(l, lcl_temp, 'ko', markerfacecolor='black')

        # Calculate full parcel profile and add to plot as black line
        prof = parcel_profile(p, T[0], Td[0]).to('degC')
        skew.plot(p, prof, 'k', linewidth=2)

        # Example of coloring area between profiles
        skew.ax.fill_betweenx(p, T, prof, where=T>=prof, facecolor='blue', alpha=0.4)
        skew.ax.fill_betweenx(p, T, prof, where=T<prof, facecolor='red', alpha=0.4)

        # An example of a slanted line at constant T -- in this case the 0
        # isotherm
        l = skew.ax.axvline(0, color='c', linestyle='--', linewidth=2)

        # Add the relevant special lines
        skew.plot_dry_adiabats()
        skew.plot_moist_adiabats()
        skew.plot_mixing_lines()

        # Discards old graph, works poorly though
        #skew.ax.hold(False)

        # refresh canvas
        self.canvas.draw()


def main():

    app = QtGui.QApplication(sys.argv)
    ex = Window()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()