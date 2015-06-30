import sys
from PyQt4 import QtGui, Qt, QtCore
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
import numpy as np
from metpy.calc import get_wind_components, lcl, dry_lapse, parcel_profile
from metpy.plots import SkewT
from metpy.units import units, concatenate
from jc_dump.SkewTDialog import SkewTDialog
from jc_dump.DataAccessor import DataAccessor


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
        clear_action = QtGui.QAction(QtGui.QIcon('./img/clear_64px.png'), 'Clear display', self)
        clear_action.setShortcut('Ctrl+C')
        clear_action.setStatusTip('Clear the display')
        clear_action.triggered.connect(self.clear_canvas)
        skewt_action = QtGui.QAction(QtGui.QIcon('./img/skewt_64px.png'), 'Skew-T', self)
        skewt_action.setShortcut('Ctrl+S')
        skewt_action.setStatusTip('Open Skew-T Dialog Box')
        skewt_action.triggered.connect(self.skewt_dialog)
        radar_action = QtGui.QAction(QtGui.QIcon('./img/radar_64px.png'), 'Radar', self)
        radar_action.setShortcut('Ctrl+R')
        radar_action.setStatusTip('Open Radar Dialog Box')
        radar_action.triggered.connect(self.skewt_dialog)

        menubar = self.menuBar()
        menubar.setNativeMenuBar(False)
        filemenu = menubar.addMenu('&File')
        editmenu = menubar.addMenu('&Edit')
        filemenu.addAction(exit_action)

        self.toolbar = QtGui.QToolBar()
        self.addToolBar(QtCore.Qt.LeftToolBarArea, self.toolbar)
        self.toolbar.addAction(clear_action)
        self.toolbar.addAction(skewt_action)
        self.toolbar.addAction(radar_action)

        self.statusBar().showMessage('Ready')
        self.setWindowTitle("WAVE (Skew-T Viewer)")
        self.setWindowIcon(QtGui.QIcon('./img/wave_64px.png'))

        # this is the Canvas Widget that displays the `figure`
        # it takes the `figure` instance as a parameter to __init__
        self.figure = plt.figure(facecolor='#2B2B2B')

        self.canvas = FigureCanvas(self.figure)

        self.setCentralWidget(self.canvas)


        self.show()

    def skewt_dialog(self):
        dialog = SkewTDialog()
        if dialog.exec_():
            source, lat, long = dialog.get_values()
            t, td, p, u, v, lat, long, time = DataAccessor.get_sounding(source, lat, long)
            self.plot(t, td, p, u, v, lat, long, time)

    def plot(self, t, td, p, u, v, lat, long, time):
        t = np.array(t)[::-1]
        td = np.array(td)[::-1]
        p = np.array(p)[::-1]
        u = np.array(u)[::-1]
        v = np.array(v)[::-1]
        p = (p * units.pascals).to('mbar')
        t = (t * units.kelvin).to('degC')
        td = td * units.degC
        u = (u * units('m/s')).to('knot')
        v = (v * units('m/s')).to('knot')
        #spd = spd * units.knot
        #direc = direc * units.deg
        #u, v = get_wind_components(spd, direc)

        # Create a new figure. The dimensions here give a good aspect ratio
        skew = SkewT(self.figure, rotation=45)

        # Plot the data using normal plotting functions, in this case using
        # log scaling in Y, as dictated by the typical meteorological plot
        skew.plot(p, t, 'r')
        skew.plot(p, td, 'g')
        skew.plot_barbs(p, u, v, barbcolor='#FF0000', flagcolor='#FF0000')
        skew.ax.set_ylim(1000, 100)
        skew.ax.set_xlim(-40, 60)

        #Axis colors
        skew.ax.tick_params(axis='x', colors='#A3A3A4')
        skew.ax.tick_params(axis='y', colors='#A3A3A4')


        # Calculate LCL height and plot as black dot
        l = lcl(p[0], t[0], td[0])
        lcl_temp = dry_lapse(concatenate((p[0], l)), t[0])[-1].to('degC')
        skew.plot(l, lcl_temp, 'ko', markerfacecolor='black')

        # Calculate full parcel profile and add to plot as black line
        prof = parcel_profile(p, t[0], td[0]).to('degC')
        skew.plot(p, prof, 'k', linewidth=2)

        # Example of coloring area between profiles
        skew.ax.fill_betweenx(p, t, prof, where=t >= prof, facecolor='#5D8C53', alpha=0.7)
        skew.ax.fill_betweenx(p, t, prof, where=t < prof, facecolor='#CD6659', alpha=0.7)

        # Add the relevant special lines
        skew.plot_dry_adiabats()
        skew.plot_moist_adiabats()
        skew.plot_mixing_lines()
        deg = u'\N{DEGREE SIGN}'

        skew.ax.set_title('Sounding for ' + lat + deg+'N, ' + long + deg + 'W at ' + time + 'z', y=1.02,
                          color='#A3A3A4')
        # Discards old graph, works poorly though
        #skew.ax.hold(False)

        # refresh canvas
        self.canvas.setMaximumSize(QtCore.QSize(700, 2000))

        self.canvas.draw()

    def clear_canvas(self):
        pass


def main():
    app = QtGui.QApplication(sys.argv)
    ex = Window()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()