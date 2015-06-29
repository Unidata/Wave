from siphon.catalog import TDSCatalog
from siphon.ncss import NCSS
from datetime import datetime
import matplotlib.pyplot as plt
from metpy.calc.thermo import dewpoint_rh
from metpy.units import units


class DataAccessor(object):
    def __init__(self):
        pass

    @staticmethod
    def get_sounding(source,  lat, long):
        # source unused for now bc testing only on ncss
        source_place_holder = source
        #print(source_place_holder)
        best_gfs = TDSCatalog('http://thredds.ucar.edu/thredds/catalog/grib/NCEP/GFS/Global_0p5deg/' +
                              'catalog.xml?dataset=grib/NCEP/GFS/Global_0p5deg/Best')
        best_ds = list(best_gfs.datasets.values())[0]

        ncss = NCSS(best_ds.access_urls['NetcdfSubset'])
        query = ncss.query()
        query.lonlat_point(long, lat).time(datetime.utcnow())
        query.accept('netcdf4')
        query.variables('Temperature_isobaric', 'Relative_humidity_isobaric')

        data = ncss.get_data(query)

        temp = data.variables['Temperature_isobaric']
        temp_vals = temp[:].squeeze() * units.kelvin
        relh = data.variables['Relative_humidity_isobaric']
        relh_values = relh[:] / 100
        td = dewpoint_rh(temp_vals, relh_values)
        td_vals = td[:].squeeze()
        press = data.variables['isobaric3']
        press_vals = press[:].squeeze()

        return temp_vals, td_vals, press_vals

