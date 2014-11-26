import json
import numpy as np
from IPython.core.display import JSON, Image, display

print 'loaded wave'

def blueMarble():
	arrayCoords = np.array([[-180.0, -90.0], [-180.0,  90.0],
							[180.0, -90.0], [180.0,  90.0]])
	display(Image('static/world.topo.bathy.200406.3x5400x2700.png'))
	display(JSON(data=json.dumps(arrayCoords.tolist())))