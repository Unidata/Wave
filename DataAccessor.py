from siphon.catalog import TDSCatalog
from siphon.ncss import NCSS
from siphon.radarserver import RadarServer
from siphon.cdmr import Dataset
from datetime import datetime
import matplotlib.pyplot as plt
from metpy.calc.thermo import dewpoint_rh
from metpy.units import units
from Dialogs import ErrorDialog
import numpy as np


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
        query.variables('Temperature_isobaric', 'Relative_humidity_isobaric', 'u-component_of_wind_isobaric',
                        'v-component_of_wind_isobaric')

        data = ncss.get_data(query)

        temp = data.variables['Temperature_isobaric']
        temp_vals = temp[:].squeeze() * units.kelvin
        relh = data.variables['Relative_humidity_isobaric']
        relh_values = relh[:] / 100
        td = dewpoint_rh(temp_vals, relh_values)
        td_vals = td[:].squeeze()
        press = data.variables['isobaric3']
        press_vals = press[:].squeeze()

        u_wind = data.variables['u-component_of_wind_isobaric']
        u_wind_vals = u_wind[:].squeeze()

        v_wind = data.variables['v-component_of_wind_isobaric']
        v_wind_vals = v_wind[:].squeeze()
        # Put temp, dewpoint, pressure, u/v winds into numpy arrays and reorder
        t = np.array(temp_vals)[::-1]
        td = np.array(td_vals)[::-1]
        p = np.array(press_vals)[::-1]
        u = np.array(u_wind_vals)[::-1]
        v = np.array(v_wind_vals)[::-1]

        # Change units for proper skew-T
        p = (p * units.pascals).to('mbar')
        t = (t * units.kelvin).to('degC')
        td = td * units.degC
        u = (u * units('m/s')).to('knot')
        v = (v * units('m/s')).to('knot')
        # spd = spd * units.knot
        # direc = direc * units.deg
        # u, v = get_wind_components(spd, direc)

        return t, td, p, u, v, lat, long, str(datetime.utcnow())[:-7]

    @staticmethod
    def get_radar(station, product):
        rs = RadarServer('http://thredds.ucar.edu/thredds/radarServer/nexrad/level3/IDD/')
        query = rs.query()
        query.stations(station).time(datetime.utcnow()).variables(product)
        validator = rs.validate_query(query)
        if validator is not True:
            ed = ErrorDialog('This query is not valid')
            ed.show()
        catalog = rs.get_catalog(query)
        ds = list(catalog.datasets.values())[0]
        data = Dataset(ds.access_urls['CdmRemote'])
        rng = data.variables['gate'][:] / 1000.
        az = data.variables['azimuth'][:]
        ref = data.variables['BaseReflectivityDR'][:]
        x = rng * np.sin(np.deg2rad(az))[:, None]
        y = rng * np.cos(np.deg2rad(az))[:, None]
        ref = np.ma.array(ref, mask=np.isnan(ref))

        return x, y, ref

