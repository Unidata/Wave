from collections import namedtuple
import numpy as np
from netCDF4 import Dataset

import rasterio
from rasterio import Affine, warp
from pyproj import Proj


print 'loaded wave'

# TODOs
# 1) Need to move away from GDAL-style transform to simple scale and translate,
#    since that's what rasterio will only use in the future and since the skew factors
#    break warping anyways
# 2) Clean up a lot of the hand-massaging here
# 3) Need to just be using some projection/transform throughout and properly
#    convert using it here instead of the implicit assumption of EQC/Plate Carree, etc.
# 4) Need to fix offset in images. Not sure where this comes from, but blue marble and
#    satellite images don't line up.
# 5) Need a better way to choose size of image returned. This likely needs to be folded in with the code
#    determining the projection parameters of the warped image, just as done in C++ GDAL API.

ImageInfo = namedtuple('ImageInfo', 'bbox data crs transform format')

def send_back(func):
    from IPython.core.display import JSON, display
    import json
    def dec(*args, **kw):
        res = func(*args, **kw)
        display(JSON(data=json.dumps(res)))
        return res
    return dec

def cornersToTriangleStripBBox(ll, ur):
    minX, minY = ll
    maxX, maxY = ur
    return np.array([[minX, minY], [minX,  maxY],
                     [maxX, minY], [maxX,  maxY]])

def xyToAffine(x, y, shape):
    src = np.vstack([[1, 1, 1], [0.5, shape[0] - 0.5, 0.5], [0.5, 0.5, shape[1] - 0.5]])
    dest = np.vstack([x, y])
    return np.dot(dest, np.linalg.inv(src)).flatten().tolist()

def readGDALRaster(src):
    with rasterio.open(src) as raster:
        bounds = raster.bounds
        bbox = cornersToTriangleStripBBox((bounds.left, bounds.bottom), (bounds.right, bounds.top))

        out = np.empty((raster.count,) + raster.shape, dtype=raster.dtypes[0])
        src_proj = Proj(**raster.crs)
        return ImageInfo(crs=raster.crs, transform=raster.affine,
            data=raster.read(out=out), bbox=bbox, format='RGB')

def readNetCDFRaster(src, var):
    with Dataset(src) as nc:
        if nc.ProjIndex == 3:
            lcc = nc.variables['LambertConformal']
            src_crs = dict(proj='lcc', lat_0=lcc.latitude_of_projection_origin,
                lon_0=lcc.longitude_of_central_meridian,
                lat_1=lcc.standard_parallel, lat_2=lcc.standard_parallel,
                radius=lcc.earth_radius)

        data = nc.variables[var][0, :-1, :-1]
        if data.dtype == np.int8:
            data.dtype = np.uint8

        x = nc.variables['x'][:-1] * 1000.
        lccX = [x[0], x[-1], x[0]]
        y = nc.variables['y'][:-1] * 1000.
        lccY = [y[0], y[0], y[-1]]
        src_trans = xyToAffine(lccX, lccY, (x.size, y.size))

        projBox = [(x[0], y[-1]), (x[0], y[0]), (x[-1], y[-1]), (x[-1], y[0])]

        return ImageInfo(crs=src_crs, transform=src_trans, data=data,
            bbox=projBox, format='LUMINANCE')

def warpRaster(imageinfo, dest_crs, dest_image):
    # Convert source bounding box to lon/lat
    src_proj = Proj(**imageinfo.crs)
    projX, projY = zip(*imageinfo.bbox)
    if src_proj.is_latlong():
        lon,lat = projX, projY
    else:
        lon,lat = src_proj(projX, projY, inverse=True)

    # Now put lon/lat into destiation CRS
    dest_proj = Proj(**dest_crs)
    dstX, dstY = np.array(dest_proj(lon, lat))

    # Figure out the enclosing bounding box of those coords
    dstMinX = dstX.min()
    dstMaxX = dstX.max()
    dstMinY = dstY.min()
    dstMaxY = dstY.max()

    dest_trans = xyToAffine([dstMinX, dstMaxX, dstMinX], [dstMaxY, dstMaxY, dstMinY], dest_image.shape[-2:][::-1])
    with rasterio.drivers():
            warp.reproject(imageinfo.data, dest_image, imageinfo.transform, imageinfo.crs, dest_trans, dest_crs)

    projBoxX = [dstMinX, dstMinX, dstMaxX, dstMaxX]
    projBoxY = [dstMinY, dstMaxY, dstMinY, dstMaxY]

    # If we have a multi-channel image, roll channels to last dimension
    if len(dest_image.shape) == 3:
        dest_image = np.rollaxis(dest_image, 0, 3)

    return ImageInfo(crs=dest_crs, transform=dest_trans, data=dest_image,
        bbox=zip(projBoxX, projBoxY), format=imageinfo.format)

typeMap = dict(uint8='UNSIGNED_BYTE')
def jsonRaster(imageinfo):
    arr = imageinfo.data
    print arr.shape
    imginfo = dict(data=arr.flatten().tolist(), type=typeMap[arr.dtype.name],
        shape=arr.shape, format=imageinfo.format)
    return dict(image=imginfo, bbox=imageinfo.bbox)

class DataManager(object):
    def __init__(self, crs=None, bounds=None):
        if crs is None:
            crs = dict(proj='eqc')

        if bounds is None:
            bounds = [(-180., -90.), (180., 90.)]

        self.crs = crs
        self.proj = Proj(**crs)

        # Turn lon/lat bounds into projected coordinates
        self._bounds = zip(*self.proj(*zip(*bounds)))

    @send_back
    def bounds(self):
        return dict(bounds=self._bounds)

    @send_back
    def blueMarble(self):
        image = readGDALRaster('static/world.topo.bathy.200406.3x5400x2700.png')

        warped_data = np.zeros_like(image.data)
        warped_image = warpRaster(image, self.crs, warped_data)

        return jsonRaster(warped_image)

    @send_back
    def satellite(self):
        # url = 'http://thredds-test.unidata.ucar.edu/thredds/dodsC/satellite/VIS/EAST-CONUS_1km/current/EAST-CONUS_1km_VIS_20141231_1915.gini'
        url = 'static/EAST-CONUS_1km_VIS_20150102_1700.gini.nc'
        image = readNetCDFRaster(url, 'VIS')

        warped_data = np.zeros((4096, 4096), dtype=np.uint8)
        warped_image = warpRaster(image, self.crs, warped_data)

        return jsonRaster(warped_image)

    @send_back
    def radar(self):
        # url = 'http://thredds-test.unidata.ucar.edu/thredds/dodsC/satellite/VIS/EAST-CONUS_1km/current/EAST-CONUS_1km_VIS_20141231_1915.gini'
        url = 'static/Level3_Composite_n0r_1km_20150102_1700.gini.nc'
        image = readNetCDFRaster(url, 'Reflectivity')

        warped_data = np.zeros_like(image.data)
        warped_image = warpRaster(image, self.crs, warped_data)

        return jsonRaster(warped_image)