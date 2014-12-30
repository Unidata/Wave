import json
import numpy as np
from netCDF4 import Dataset
from rasterio import warp
from pyproj import Proj
from IPython.core.display import JSON, Image, display

print 'loaded wave'

def cornersToTriangleStripBBox(ll, ur):
    minX, minY = ll
    maxX, maxY = ur
    return np.array([[minX, minY], [minX,  maxY],
                     [maxX, minY], [maxX,  maxY]])

def xyToAffine(x, y, shape):
    src = np.vstack([[1, 1, 1], [0, 0, shape[0] - 1], [0, shape[1] - 1, 0]])
    dest = np.vstack([x, y])
    return np.dot(dest, np.linalg.inv(src)).flatten()

def blueMarble():
    # arrayCoords = np.array([[-180.0, -90.0], [-180.0,  90.0],
    #                       [180.0, -90.0], [180.0,  90.0]])
    arrayCoords = cornersToTriangleStripBBox((-180., -90.), (180., 90.))
    display(Image('static/world.topo.bathy.200406.3x5400x2700.png'))
    info = dict(bbox=arrayCoords.tolist())
    display(JSON(data=json.dumps(info)))

def satellite():
    url = 'http://thredds-dev.unidata.ucar.edu/thredds/dodsC/satellite/VIS/EAST-CONUS_1km/current/EAST-CONUS_1km_VIS_20141211_1915.gini'
    nc = Dataset(url)
    # print nc
    if nc.ProjIndex == 3:
        lcc = nc.variables['LambertConformal']
        src_crs = dict(proj='lcc', lat_0=lcc.latitude_of_projection_origin,
            lon_0=lcc.longitude_of_central_meridian,
            lat_1=lcc.standard_parallel, radius=lcc.earth_radius)

    vis_data = nc.variables['VIS']
    numY, numX = vis_data.shape[1::]
    arr = vis_data[0, :-1, :-1]

    x = nc.variables['x'][:-1] * 1000.
    lccX = [x[0], x[-1], x[0]]
    y = nc.variables['y'][:-1] * 1000.
    lccY = [y[0], y[0], y[-1]]

    src_proj = Proj(**src_crs)
    src_trans = xyToAffine(lccX, lccY, (x.size, y.size))

    dest_crs = dict(proj='lonlat')
    dest_proj = Proj(**dest_crs)
    warped_data = np.zeros((4096, 4096), dtype=np.uint8)

    lon,lat = src_proj(lccX, lccY, inverse=True)
    # print lon, lat
    dstX, dstY = dest_proj(lon, lat)
    # print dstX, dstY
    dest_trans = xyToAffine(dstX, dstY, warped_data.shape)
    # dest_trans = xyToAffine(lon, lat, warped_data.shape)
    # print dest_trans
    ul, ur, ll = zip(lon, lat)
    lr = np.dot(dest_trans.reshape(2, 3),
        np.array([[1], [warped_data.shape[0]], [warped_data.shape[1]]])).squeeze()
    arrayCoords = np.array([ll, ul, lr, ur])
    # print arrayCoords

    arrayCoords = cornersToTriangleStripBBox((nc.geospatial_lon_min, nc.geospatial_lat_min),
        (nc.geospatial_lon_max, nc.geospatial_lat_max))
    imginfo = dict(data=arr.flatten().tolist(), type='UNSIGNED_BYTE', shape=arr.shape[::-1])
    # imginfo = dict(data=warped_data.flatten().tolist(), type='UNSIGNED_BYTE', shape=warped_data.shape[::-1])
    info = dict(image=imginfo, bbox=arrayCoords.tolist())
    display(JSON(data=json.dumps(info)))
    nc.close()
    return warped_data