./wmm_grid 2023.70685 > mag_var.xyz
gdal_translate -of GTiff -a_srs EPSG:4326 mag_var.xyz mag_var.tif
raster2pgsql -s 4326 -d -I -C -M -Y -t auto mag_var.tif -f declination import.mag_var | psql -dnavigraph