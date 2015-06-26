from PyQt4 import QtGui
#from siphon.catalog import TDSCatalog
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
import matplotlib.pyplot as plt
import numpy as np
from metpy.calc import get_wind_components, lcl, dry_lapse, parcel_profile
from metpy.plots import SkewT
from metpy.units import units, concatenate


class skewt_dialog(QtGui.QDialog):
    def __init__(self, parent=None):
        super(skewt_dialog, self).__init__()
        self.setWindowTitle("Skew-T Menu")
        self.setGeometry(0, 0, 450, 300)
        self.move(QtGui.QApplication.desktop().screen().rect().center()- self.rect().center())
        css = """
                QWidget{
                    Background: #4c4c4c;
                    color:white;
                    font:14px bold;
                    font-weight:bold;
                    border-radius: 1px;
                }

                QLineEdit{
                    Background: #666666;
                    padding: 4px 4px 4px 4px;
                }

                QComboBox{
                    Background: #666666;
                    padding: 4px 4px 4px 4px;
                }

                QPushButton{
                    Background: #999999;
                    padding: 4px 4px 4px 4px;
                    border-radius: 5px;

                }
                """

        self.setStyleSheet(css)
        self.setAutoFillBackground(True)
        self.setBackgroundRole(QtGui.QPalette.Highlight)

        grid = QtGui.QGridLayout()
        grid.setSpacing(10)
        self.setLayout(grid)


        self.lbl = QtGui.QLabel("Data Source:", self)
        grid.addWidget(self.lbl, 0, 0, 1, 1)
        combo = QtGui.QComboBox(self)
        combo.addItem("NCSS")
        grid.addWidget(combo, 0, 1, 1, 1)

        self.lbl = QtGui.QLabel("Latitude:", self)
        grid.addWidget(self.lbl, 1, 0, 1, 1)
        latbox = QtGui.QLineEdit(self)
        grid.addWidget(latbox, 1, 1, 1, 1)

        self.lbl = QtGui.QLabel("Longitude:", self)
        grid.addWidget(self.lbl, 2, 0, 1, 1)
        longbox = QtGui.QLineEdit(self)
        grid.addWidget(longbox, 2, 1, 1, 1)

        #combo.activated[str].connect(self.onActivated)
        submit_button = QtGui.QPushButton('Plot')
        submit_button.clicked.connect(self.plot)

        grid.addWidget(submit_button, 3, 0, 1, 3)

        self.show()

    def get_data(self):
        pass

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
