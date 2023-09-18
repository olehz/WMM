#!/usr/bin/env python3
from pygeomag import GeoMag
geo_mag = GeoMag()

lon = 30
lat = 30
r = geo_mag.calculate(
    glat=lat, glon=lon, alt=1, time=2023.7)
print(lon, lat, ' ', round(r.d, 4))
exit()
step_size = 1
lat = -90
while lat <= 90:
    lon = -180
    while lon < 180:
        r = geo_mag.calculate(
            glat=lat, glon=lon, alt=1, time=2023.7)
        print(lon, lat, ' ', round(r.d, 4))
        lon += step_size
    lat += step_size
    
