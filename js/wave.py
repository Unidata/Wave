import json
import numpy as np
from netCDF4 import Dataset
from rasterio import Affine, warp
import rasterio
from pyproj import Proj
from IPython.core.display import JSON, Image, display

print 'loaded wave'

# TODOs
# 1) Need to move away from GDAL-style transform to simple scale and translate,
#    since that's what rasterio will only use in the future and since the skew factors
#    break warping anyways
# 2) Clean up a lot of the hand-massaging here
# 3) Need to just be using some projection/transform throughout and properly
#    convert using it here instead of the implicit assumption of EQC/Plate Carree, etc.
# 4) Need to fix half-pixel offset in images. This is caused by the use of the x,y values from
#    TDS that correspond to grid box centers (?) while GDAL wants the offset to the upper left corner
#    of the pixel
# 5) Need a better way to choose size of image returned. This likely needs to be folded in with the code
#    determining the projection parameters of the warped image, just as done in C++ GDAL API.

def cornersToTriangleStripBBox(ll, ur):
    minX, minY = ll
    maxX, maxY = ur
    return np.array([[minX, minY], [minX,  maxY],
                     [maxX, minY], [maxX,  maxY]])

def xyToAffine(x, y, shape):
    src = np.vstack([[1, 1, 1], [0, shape[0] - 1, 0], [0, 0, shape[1] - 1]])
    dest = np.vstack([x, y])
    return np.dot(dest, np.linalg.inv(src)).flatten().tolist()

def blueMarble():
    # arrayCoords = np.array([[-180.0, -90.0], [-180.0,  90.0],
    #                       [180.0, -90.0], [180.0,  90.0]])
    arrayCoords = cornersToTriangleStripBBox((-180., -90.), (180., 90.))
    display(Image('static/world.topo.bathy.200406.3x5400x2700.png'))
    info = dict(bbox=arrayCoords.tolist())
    display(JSON(data=json.dumps(info)))

def satellite():
    # url = 'http://thredds-test.unidata.ucar.edu/thredds/dodsC/satellite/VIS/EAST-CONUS_1km/current/EAST-CONUS_1km_VIS_20141231_1915.gini'
    url = 'static/EAST-CONUS_1km_VIS_20141230_1915.gini.nc'
    nc = Dataset(url)
    # print nc
    if nc.ProjIndex == 3:
        lcc = nc.variables['LambertConformal']
        src_crs = dict(proj='lcc', lat_0=lcc.latitude_of_projection_origin,
            lon_0=lcc.longitude_of_central_meridian,
            lat_1=lcc.standard_parallel, lat_2=lcc.standard_parallel,
            radius=lcc.earth_radius)

    vis_data = nc.variables['VIS']
    numY, numX = vis_data.shape[1::]
    arr = vis_data[0, :-1, :-1].astype(np.uint8)

    x = nc.variables['x'][:-1] * 1000.
    lccX = [x[0], x[-1], x[0]]
    y = nc.variables['y'][:-1] * 1000.
    lccY = [y[0], y[0], y[-1]]

    src_proj = Proj(**src_crs)
    src_trans = xyToAffine(lccX, lccY, (x.size, y.size))
    print src_trans

    dest_crs = dict(proj='eqc', lon_0=lcc.longitude_of_central_meridian, lat_0=lcc.latitude_of_projection_origin)
    dest_proj = Proj(**dest_crs)
    projBoxX = [x[0], x[0], x[-1], x[-1]]
    projBoxY = [y[-1], y[0], y[-1], y[0]]

    lon,lat = np.array(src_proj(projBoxX, projBoxY, inverse=True))
    dstX, dstY = dest_proj(lon, lat)

    dstMinX = dstX.min()
    dstMaxX = dstX.max()
    dstMinY = dstY.min()
    dstMaxY = dstY.max()

    warped_data = np.zeros((4096, 4096), dtype=np.uint8)

    dest_trans = xyToAffine([dstMinX, dstMaxX, dstMinX], [dstMaxY, dstMaxY, dstMinY], warped_data.shape[::-1])
    with rasterio.drivers():
        warp.reproject(arr, warped_data, src_trans, src_crs, dest_trans, dest_crs)

    projBoxX = [dstMinX, dstMinX, dstMaxX, dstMaxX]
    projBoxY = [dstMinY, dstMaxY, dstMinY, dstMaxY]

    arrayCoords = np.array(dest_proj(projBoxX, projBoxY, inverse=True)).T

    print arrayCoords
    imginfo = dict(data=warped_data.flatten().tolist(), type='UNSIGNED_BYTE', shape=warped_data.shape[::-1])
    info = dict(image=imginfo, bbox=arrayCoords.tolist())
    display(JSON(data=json.dumps(info)))
    nc.close()
    return warped_data
