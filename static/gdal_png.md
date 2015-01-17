Notes for geo-referencing a PNG
=========

1) Create the georeferencing information in .aux.xml file:
`gdal_translate -of PNG -a_ullr ul_lon ul_lat lr_lon lr_lat -a_srs SRS file.png output.png`

For example:
`gdal_translate -of PNG -a_ullr -180. 90. 180. -90. -a_srs EPSG:4326 world.topo.bathy.200406.3x5400x2700.png output.png`

2) Remove output.png and move output.png.aux.xml to file.png.aux.xml

3) You may need to adjust the rounding of parameters in file, such as 6.66666666e-02 to 6.66666667e-02
