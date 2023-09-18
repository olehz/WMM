DROP TABLE IF EXISTS apt_deltas;CREATE TABLE apt_deltas AS
SELECT
    icao_identifier AS icao,
    magnetic_variation AS jeppesen_mag_var,
    ROUND(noaa * 10) / 10 AS wmm_mag_var,
    delta
    --,ST_X(geom) AS lon, ST_Y(geom) AS lat
    --,geom
FROM
    arinc424_gis.tbl_airport, import.mag_var,
    ST_Value(declination, geom) noaa,
    ROUND(ABS(magnetic_variation - noaa)) delta
WHERE
    icao_identifier ~ '[A-Z]{4}' AND
    ABS(ST_Y(geom)) < 85 AND delta > 1 AND
    --magnetic_true_indicator != 'T' AND
    ST_Intersects(declination, geom) AND delta > 2
ORDER BY delta DESC;